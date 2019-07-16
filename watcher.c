/*
 * TODO: daemonize this
 * TODO: refactor if needed
 * TODO: check corner cases
 * TODO: auto (backlight sysfs) path detection
 * TODO: power saving features (timers to dim screen, etc...). Only do this if no alternatives.
 * 
 * 
 * can add more interfaces to monitor more things in the future
 */
#include <assert.h>
#include <gio/gio.h>
#include <glib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "watcher-dbus.h"

#define MIN_BRIGHTNESS_PERCENT 10

Watcher *skeleton;
GFile *file;
guint max_brightness;

guint get_max_brightness() {
    GError *err = NULL;

    gchar *contents;
    gsize length;
    if (!g_file_get_contents("/sys/class/backlight/radeon_bl0/max_brightness", &contents, &length, &err)) {
        char *fpath = g_file_get_path(file);
        fprintf(stderr, "unable to read %s: %s\n", fpath, err->message);
        g_free(fpath);
        g_error_free(err);
        exit(EXIT_FAILURE);
    }

    guint temp = atoi(contents);
    g_free(contents);
    return temp;
}

guint read_brightness() {
    GError *err = NULL;

    gchar *contents;
    gsize length;
    if (!g_file_get_contents("/sys/class/backlight/radeon_bl0/brightness", &contents, &length, &err)) {
        char *fpath = g_file_get_path(file);
        fprintf(stderr, "unable to read %s: %s\n", fpath, err->message);
        g_free(fpath);
        g_error_free(err);
        exit(EXIT_FAILURE);
    }

    guint temp = atoi(contents);
    g_free(contents);
    return temp;
}

void write_brightness(guint value) {
    GError *err = NULL;
    GFileOutputStream *stream = g_file_replace(file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, &err);
    if (err) {
        char *fpath = g_file_get_path(file);
        fprintf(stderr, "unable to write %s: %s\n", fpath, err->message);
        g_free(fpath);
        g_error_free(err);
        g_object_unref(stream);
        exit(EXIT_FAILURE);
    }

    char buf[12];
    sprintf(buf, "%d", value);

    gssize read = g_output_stream_write((GOutputStream *) stream, (void *) buf, 12, NULL, &err);
    if (err) {
        char *fpath = g_file_get_path(file);
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
        guint brightness = read_brightness(file);
        watcher_set_percentage(skeleton, (gfloat) brightness / max_brightness * 100);
        printf("brightness = %d\n", brightness);
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

void start_monitoring() {
    // TODO: make a smart detection of the path
    file = g_file_new_for_path("/sys/class/backlight/radeon_bl0/brightness");
    assert(file);

    GError *err = NULL;
    GFileMonitor *fm = g_file_monitor(file, G_FILE_MONITOR_SEND_MOVED, NULL, &err);
    if (err) {
        char *fpath = g_file_get_path(file);
        fprintf(stderr, "unable to monitor %s: %s\n", fpath, err->message);
        g_free(fpath);
        g_error_free(err);
        exit(EXIT_FAILURE);
    }

    g_signal_connect(G_OBJECT(fm), "changed", G_CALLBACK(file_changed_cb), NULL);

    char *fpath = g_file_get_path(file);
    g_print("monitoring %s\n", fpath);
    g_free(fpath);

    // update initial brightness properties
    guint brightness = read_brightness();
    max_brightness = get_max_brightness();
    guint min_brightness = MIN_BRIGHTNESS_PERCENT / 100.0 * max_brightness;
    watcher_set_percentage(skeleton, (gfloat) brightness / max_brightness * 100);
    watcher_set_min_percentage(skeleton, MIN_BRIGHTNESS_PERCENT);
    printf("brightness = %d\n", brightness);
    printf("max brightness = %d\n", max_brightness);
    printf("min brightness = %d\n", min_brightness);
}

static void on_handle_set_brightness(Watcher *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value < MIN_BRIGHTNESS_PERCENT || value > 100) {
        g_dbus_method_invocation_return_error_literal(invocation, G_IO_ERROR, G_IO_ERROR_EXISTS, "input must be in interval: [MinPercentage, 100]");
        return;
    }

    guint current_value = watcher_get_percentage(skeleton);
    if (value == current_value) {
        watcher_complete_set_brightness(skeleton, invocation);
        return;
    }

    watcher_set_percentage(skeleton, value);

    guint true_value = ceil(value / 100.0 * max_brightness);
    write_brightness(true_value);
    printf("brightness set to %d\n", true_value);

    watcher_complete_set_brightness(skeleton, invocation);
}

static void on_handle_inc_brightness(Watcher *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value <= 0) {
        watcher_complete_inc_brightness(skeleton, invocation);
        return;
    }

    guint current_value = watcher_get_percentage(skeleton);
    guint target_value = current_value + value;
    if (target_value > 100)
        target_value = 100;

    if (target_value == current_value) {
        watcher_complete_inc_brightness(skeleton, invocation);
        return;
    }

    watcher_set_percentage(skeleton, target_value);
    guint true_value = ceil(target_value / 100.0 * max_brightness);
    write_brightness(true_value);
    printf("brightness set to %d\n", true_value);

    watcher_complete_inc_brightness(skeleton, invocation);
}

static void on_handle_dec_brightness(Watcher *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value <= 0) {
        watcher_complete_dec_brightness(skeleton, invocation);
        return;
    }

    guint current_value = watcher_get_percentage(skeleton);
    gint target_value = current_value - value;
    if (target_value < MIN_BRIGHTNESS_PERCENT)
        target_value = MIN_BRIGHTNESS_PERCENT;
    
    if (target_value == current_value) {
        watcher_complete_dec_brightness(skeleton, invocation);
        return;
    }

    watcher_set_percentage(skeleton, target_value);
    guint true_value = ceil(target_value / 100.0 * max_brightness);
    write_brightness(true_value);
    printf("brightness set to %d\n", true_value);

    watcher_complete_dec_brightness(skeleton, invocation);
}

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data) {
    skeleton = watcher_skeleton_new();

    g_signal_connect(skeleton, "handle-set-brightness", G_CALLBACK(on_handle_set_brightness), NULL);
    g_signal_connect(skeleton, "handle-inc-brightness", G_CALLBACK(on_handle_inc_brightness), NULL);
    g_signal_connect(skeleton, "handle-dec-brightness", G_CALLBACK(on_handle_dec_brightness), NULL);

    // add error handling for this line
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton), connection,
                                     "/fr/mpostaire/Watcher", NULL);

    // if name acquired we can begin to monitor brightness
    start_monitoring();
}

int main(int argc, char **argv) {
    GMainLoop *ws = g_main_loop_new(NULL, FALSE);
    assert(ws);

    // change to SESSION bus
    g_bus_own_name(G_BUS_TYPE_SESSION, "fr.mpostaire.Watcher", G_BUS_NAME_OWNER_FLAGS_NONE, NULL,
                   on_name_acquired, NULL, NULL, NULL);

    g_main_loop_run(ws);

    g_object_unref(file);

    return EXIT_SUCCESS;
}
