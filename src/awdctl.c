/*
 * TODO: daemonize this
 * TODO: check for errors return values where its possible
 * TODO: power saving features (timers to dim screen, etc...). Only do this if no alternatives. (check acpi)
 * 
 * can add more interfaces to monitor more things in the future
 */
#include "audioctl.h"
#include "brightnessctl.h"
#include "awdctl-dbus.h"
#include <alsa/asoundlib.h>
#include <assert.h>
#include <gio/gio.h>
#include <glib-unix.h>
#include <glib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

static void on_handle_set_brightness(AwdctlBrightness *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value < MIN_BRIGHTNESS_PERCENT || value > 100) {
        g_dbus_method_invocation_return_error_literal(invocation, G_IO_ERROR, G_IO_ERROR_EXISTS, "input must be in interval: [MinPercentage, 100]");
        awdctl_brightness_complete_set_brightness(skeleton, invocation);
        return;
    }

    guint current_value = awdctl_brightness_get_percentage(skeleton);
    if (value == current_value) {
        awdctl_brightness_complete_set_brightness(skeleton, invocation);
        return;
    }

    guint true_value = ceil(value / 100.0 * get_max_brightness());
    write_brightness(true_value);
    // awdctl_brightness_set_percentage(skeleton, value); // ici marche
    g_print("brightness set to %d\n", true_value);

    awdctl_brightness_complete_set_brightness(skeleton, invocation);
}

static void on_handle_inc_brightness(AwdctlBrightness *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value <= 0) {
        awdctl_brightness_complete_inc_brightness(skeleton, invocation);
        return;
    }

    guint current_value = awdctl_brightness_get_percentage(skeleton);
    guint target_value = current_value + value;
    if (target_value > 100)
        target_value = 100;

    if (target_value == current_value) {
        awdctl_brightness_complete_inc_brightness(skeleton, invocation);
        return;
    }

    guint true_value = ceil(target_value / 100.0 * get_max_brightness());
    write_brightness(true_value);
    g_print("brightness set to %d\n", true_value);

    awdctl_brightness_complete_inc_brightness(skeleton, invocation);
}

static void on_handle_dec_brightness(AwdctlBrightness *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value <= 0) {
        awdctl_brightness_complete_dec_brightness(skeleton, invocation);
        return;
    }

    guint current_value = awdctl_brightness_get_percentage(skeleton);
    gint target_value = current_value - value;
    if (target_value < MIN_BRIGHTNESS_PERCENT)
        target_value = MIN_BRIGHTNESS_PERCENT;

    if (target_value == current_value) {
        awdctl_brightness_complete_dec_brightness(skeleton, invocation);
        return;
    }

    guint true_value = ceil(target_value / 100.0 * get_max_brightness());
    write_brightness(true_value);
    g_print("brightness set to %d\n", true_value);

    awdctl_brightness_complete_dec_brightness(skeleton, invocation);
}

static void on_handle_set_volume(AwdctlVolume *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value < 0 || value > 100) {
        g_dbus_method_invocation_return_error_literal(invocation, G_IO_ERROR, G_IO_ERROR_EXISTS, "input must be in interval: [MinPercentage, 100]");
        awdctl_volume_complete_set_volume(skeleton, invocation);
        return;
    }

    alsa_set_volume(value);
    g_print("audio volume set to %d\n", value);

    awdctl_volume_complete_set_volume(skeleton, invocation);
}

static void on_handle_inc_volume(AwdctlVolume *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value <= 0) {
        awdctl_volume_complete_inc_volume(skeleton, invocation);
        return;
    }

    guint current_value = awdctl_volume_get_percentage(skeleton);
    guint target_value = current_value + value;
    if (target_value > 100)
        target_value = 100;

    if (target_value == current_value) {
        awdctl_volume_complete_inc_volume(skeleton, invocation);
        return;
    }

    alsa_set_volume(target_value);
    g_print("audio volume set to %d\n", target_value);

    awdctl_volume_complete_inc_volume(skeleton, invocation);
}

static void on_handle_dec_volume(AwdctlVolume *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value <= 0) {
        awdctl_volume_complete_dec_volume(skeleton, invocation);
        return;
    }

    guint current_value = awdctl_volume_get_percentage(skeleton);
    gint target_value = current_value - value;
    if (target_value < 0)
        target_value = 0;

    if (target_value == current_value) {
        awdctl_volume_complete_dec_volume(skeleton, invocation);
        return;
    }

    alsa_set_volume(target_value);
    g_print("audio volume set to %d\n", target_value);

    awdctl_volume_complete_dec_volume(skeleton, invocation);
}

static void on_handle_toggle_volume(AwdctlVolume *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    alsa_toggle_volume();
    awdctl_volume_complete_toggle_volume(skeleton, invocation);
}

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data) {
    // setup brightness interface
    AwdctlBrightness *brightness_skeleton = awdctl_brightness_skeleton_new();

    g_signal_connect(brightness_skeleton, "handle-set-brightness", G_CALLBACK(on_handle_set_brightness), NULL);
    g_signal_connect(brightness_skeleton, "handle-inc-brightness", G_CALLBACK(on_handle_inc_brightness), NULL);
    g_signal_connect(brightness_skeleton, "handle-dec-brightness", G_CALLBACK(on_handle_dec_brightness), NULL);

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(brightness_skeleton), connection,
                                     "/fr/mpostaire/awdctl/Brightness", NULL);

    // we can begin to monitor brightness
    start_brightness_monitoring(brightness_skeleton);

    // setup volume interface
    AwdctlVolume *volume_skeleton = awdctl_volume_skeleton_new();

    g_signal_connect(volume_skeleton, "handle-set-volume", G_CALLBACK(on_handle_set_volume), NULL);
    g_signal_connect(volume_skeleton, "handle-inc-volume", G_CALLBACK(on_handle_inc_volume), NULL);
    g_signal_connect(volume_skeleton, "handle-dec-volume", G_CALLBACK(on_handle_dec_volume), NULL);
    g_signal_connect(volume_skeleton, "handle-toggle-volume", G_CALLBACK(on_handle_toggle_volume), NULL);

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(volume_skeleton), connection,
                                     "/fr/mpostaire/awdctl/Volume", NULL);

    // we can begin to monitor volume
    start_volume_monitoring(volume_skeleton);
}

static void on_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data) {
    fprintf(stderr, "Connection lost or already owned.\n");
    exit(EXIT_FAILURE);
}

static gboolean quit(gpointer user_data) {
    GMainLoop *loop = (GMainLoop *) user_data;
    g_main_loop_quit(loop);
    return FALSE;
}

int main(int argc, char **argv) {
    int err = daemon(0, 0);
    if (err == -1) {
        fprintf(stderr, "failed to daemonize\n");
        return EXIT_FAILURE;
    }

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    assert(loop);

    g_unix_signal_add(SIGINT, quit, loop);
    // g_unix_signal_add(SIGHUP, quit, loop); // should reload config file but there is none yet
    g_unix_signal_add(SIGTERM, quit, loop);

    guint id = g_bus_own_name(G_BUS_TYPE_SESSION, "fr.mpostaire.awdctl", G_BUS_NAME_OWNER_FLAGS_NONE, NULL,
                              on_name_acquired, on_name_lost, NULL, NULL);

    g_main_loop_run(loop);

    // free and close everything here
    g_print("exiting...\n");
    g_bus_unown_name(id); // maybe useless
    g_main_loop_unref(loop);
    audioctl_close();
    brightnessctl_close();

    return EXIT_SUCCESS;
}