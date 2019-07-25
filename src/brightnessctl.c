#include "brightnessctl.h"
#include "watcher-dbus.h"
#include <assert.h>
#include <gio/gio.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

static GFile *file;
static guint max_brightness;

guint get_max_brightness() {
    return max_brightness;
}

guint read_brightness(char *path) {
    GError *err = NULL;

    char *contents;
    gsize length;
    if (!g_file_get_contents(path, &contents, &length, &err)) {
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
    int size = sprintf(buf, "%d", value);

    gssize wrote = g_output_stream_write((GOutputStream *) stream, (void *) buf, size, NULL, &err);
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
    WatcherBrightness *skel = (WatcherBrightness *) user_data;
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
        guint brightness = read_brightness(BRIGHTNESS_PATH);
        watcher_brightness_set_percentage(skel, (gfloat) brightness / max_brightness * 100);
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

void start_brightness_monitoring(WatcherBrightness *skeleton) {
    // TODO: make a smart detection of the path
    file = g_file_new_for_path(BRIGHTNESS_PATH);
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

    g_signal_connect(G_OBJECT(fm), "changed", G_CALLBACK(file_changed_cb), skeleton);

    char *fpath = g_file_get_path(file);
    g_print("monitoring %s\n", fpath);
    g_free(fpath);

    // update initial brightness properties
    guint brightness = read_brightness(BRIGHTNESS_PATH);
    max_brightness = read_brightness(MAX_BRIGHTNESS_PATH);
    guint min_brightness = MIN_BRIGHTNESS_PERCENT / 100.0 * max_brightness;
    watcher_brightness_set_percentage(skeleton, (gfloat) brightness / max_brightness * 100);
    watcher_brightness_set_min_percentage(skeleton, MIN_BRIGHTNESS_PERCENT);
}
