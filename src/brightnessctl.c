#include "brightnessctl.h"
#include "awdctl-dbus.h"
#include <assert.h>
#include <gio/gio.h>
#include <glib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static AwdctlBrightness *brightness_skeleton;
static GFile *brightness_file;
static guint max_brightness;
static char brightness_path[256], max_brightness_path[256];

static void get_backlight_sysfs_path(char *brightness_path, char *max_brightness_path) {
    GFile *file = g_file_new_for_path("/sys/class/backlight");
    GFileEnumerator *enumerator = g_file_enumerate_children(file, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NONE, NULL, NULL);
    GFileInfo *info = g_file_enumerator_next_file(enumerator, NULL, NULL);

    const char *name = g_file_info_get_name(info);
    sprintf(brightness_path, "/sys/class/backlight/%s/brightness", name);
    sprintf(max_brightness_path, "/sys/class/backlight/%s/max_brightness", name);

    g_file_enumerator_close(enumerator, NULL, NULL);
    g_object_unref(enumerator);
    g_object_unref(file);
    g_object_unref(info);
}

static guint read_brightness_path(char *path) {
    GError *err = NULL;

    char *contents;
    gsize length;
    if (!g_file_get_contents(path, &contents, &length, &err)) {
        fprintf(stderr, "unable to read %s: %s\n", path, err->message);
        g_error_free(err);
        exit(EXIT_FAILURE);
    }

    guint temp = atoi(contents);
    g_free(contents);
    return temp;
}

static guint read_max_brightness() {
    return read_brightness_path(max_brightness_path);
}

static guint read_brightness() {
    return read_brightness_path(brightness_path);
}

static void write_brightness(guint value) {
    GError *err = NULL;
    GFileOutputStream *stream = g_file_replace(brightness_file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, &err);
    if (err) {
        char *fpath = g_file_get_path(brightness_file);
        fprintf(stderr, "unable to write %s: %s\n", fpath, err->message);
        g_free(fpath);
        g_error_free(err);
        g_object_unref(stream);
        exit(EXIT_FAILURE);
    }

    char buf[12];
    int size = sprintf(buf, "%d", value);

    gssize wrote = g_output_stream_write((GOutputStream *) stream, (void *) buf, size, NULL, &err);
    if (err) {
        char *fpath = g_file_get_path(brightness_file);
        fprintf(stderr, "unable to write data to %s: %s\n", fpath, err->message);
        g_free(fpath);
        g_error_free(err);
        g_object_unref(stream);
        exit(EXIT_FAILURE);
    }

    g_output_stream_close((GOutputStream *) stream, NULL, NULL);
    g_object_unref(stream);
}

static void file_changed_cb(GFileMonitor *monitor, GFile *file, GFile *other, GFileMonitorEvent evtype, gpointer user_data) {
    char *fpath = g_file_get_path(file);
    char *opath = NULL;
    if (other) {
        opath = g_file_get_path(other);
    }
    switch (evtype) {
    case G_FILE_MONITOR_EVENT_CHANGED:
        // g_print("%s contents changed\n", fpath);
        break;
    case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
        g_print("%s set of changes done\n", fpath);
        guint brightness = read_brightness();
        awdctl_brightness_set_percentage(brightness_skeleton, (gfloat) brightness / max_brightness * 100);
        g_print("brightness = %d\n", brightness);
        break;
    case G_FILE_MONITOR_EVENT_DELETED:
        // g_print("%s deleted\n", fpath);
        break;
    case G_FILE_MONITOR_EVENT_CREATED:
        // g_print("%s created\n", fpath);
        break;
    case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
        // g_print("%s attributes changed\n", fpath);
        break;
    case G_FILE_MONITOR_EVENT_RENAMED:
        // g_print("%s renamed to %s\n", fpath, opath);
        break;
    case G_FILE_MONITOR_EVENT_MOVED_IN:
        // g_print("%s moved to directory\n", fpath);
        break;
    case G_FILE_MONITOR_EVENT_MOVED_OUT:
        // g_print("%s moved from directory\n", fpath);
        break;
    default:
        g_print("%s event %x\n", fpath, evtype);
        break;
    }
    if (opath) {
        g_free(opath);
    }
    g_free(fpath);
}

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

    guint true_value = ceil(value / 100.0 * max_brightness);
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

    guint true_value = ceil(target_value / 100.0 * max_brightness);
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

    guint true_value = ceil(target_value / 100.0 * max_brightness);
    write_brightness(true_value);
    g_print("brightness set to %d\n", true_value);

    awdctl_brightness_complete_dec_brightness(skeleton, invocation);
}

void brightnessctl_close() {
    // free everything that needs to be freed in brightnessctl here
    g_object_unref(brightness_skeleton);
}

void brightnessctl_export(GDBusConnection *connection) {
    // setup brightness interface
    brightness_skeleton = awdctl_brightness_skeleton_new();

    g_signal_connect(brightness_skeleton, "handle-set-brightness", G_CALLBACK(on_handle_set_brightness), NULL);
    g_signal_connect(brightness_skeleton, "handle-inc-brightness", G_CALLBACK(on_handle_inc_brightness), NULL);
    g_signal_connect(brightness_skeleton, "handle-dec-brightness", G_CALLBACK(on_handle_dec_brightness), NULL);

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(brightness_skeleton), connection,
                                     "/fr/mpostaire/awdctl/Brightness", NULL);
}

void brightnessctl_start() {
    // we can begin to monitor brightness
    get_backlight_sysfs_path(brightness_path, max_brightness_path);
    brightness_file = g_file_new_for_path(brightness_path);
    assert(brightness_file);

    GError *err = NULL;
    GFileMonitor *fm = g_file_monitor(brightness_file, G_FILE_MONITOR_SEND_MOVED, NULL, &err);
    if (err) {
        char *fpath = g_file_get_path(brightness_file);
        fprintf(stderr, "unable to monitor %s: %s\n", fpath, err->message);
        g_free(fpath);
        g_error_free(err);
        exit(EXIT_FAILURE);
    }

    g_signal_connect(G_OBJECT(fm), "changed", G_CALLBACK(file_changed_cb), NULL);

    char *fpath = g_file_get_path(brightness_file);
    g_print("monitoring %s\n", fpath);
    g_free(fpath);

    // update initial brightness properties
    guint brightness = read_brightness();
    max_brightness = read_max_brightness();
    guint min_brightness = MIN_BRIGHTNESS_PERCENT / 100.0 * max_brightness;
    awdctl_brightness_set_percentage(brightness_skeleton, (gfloat) brightness / max_brightness * 100);
    awdctl_brightness_set_min_percentage(brightness_skeleton, MIN_BRIGHTNESS_PERCENT);
}
