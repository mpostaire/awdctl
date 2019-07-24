/*
 * TODO: daemonize this
 * TODO: refactor if needed
 * TODO: check corner cases
 * TODO: auto (backlight sysfs) path detection
 * TODO: power saving features (timers to dim screen, etc...). Only do this if no alternatives. (check acpi)
 * TODO: use user_data args to get rid of global variables
 * TODO: break in smaller files
 * TODO: memory leak check
 * 
 * can add more interfaces to monitor more things in the future
 */
#include <assert.h>
#include <gio/gio.h>
#include <glib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include "watcher-dbus.h"

#define MIN_BRIGHTNESS_PERCENT 10
#define BRIGHTNESS_PATH "/sys/class/backlight/radeon_bl0/brightness"
#define MAX_BRIGHTNESS_PATH "/sys/class/backlight/radeon_bl0/max_brightness"

WatcherVolume *volume_skeleton;
struct pollfd pfd;

WatcherBrightness *brightness_skeleton;
GFile *file;
guint max_brightness;

static guint read_brightness(char *path) {
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

static void write_brightness(guint value) {
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
        guint brightness = read_brightness(BRIGHTNESS_PATH);
        watcher_brightness_set_percentage(brightness_skeleton, (gfloat) brightness / max_brightness * 100);
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

static void start_brightness_monitoring() {
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

    g_signal_connect(G_OBJECT(fm), "changed", G_CALLBACK(file_changed_cb), NULL);

    char *fpath = g_file_get_path(file);
    g_print("monitoring %s\n", fpath);
    g_free(fpath);

    // update initial brightness properties
    guint brightness = read_brightness(BRIGHTNESS_PATH);
    max_brightness = read_brightness(MAX_BRIGHTNESS_PATH);
    guint min_brightness = MIN_BRIGHTNESS_PERCENT / 100.0 * max_brightness;
    watcher_brightness_set_percentage(brightness_skeleton, (gfloat) brightness / max_brightness * 100);
    watcher_brightness_set_min_percentage(brightness_skeleton, MIN_BRIGHTNESS_PERCENT);
    printf("brightness = %d\n", brightness);
    printf("max brightness = %d\n", max_brightness);
    printf("min brightness = %d\n", min_brightness);
}

static void on_handle_set_brightness(WatcherBrightness *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value < MIN_BRIGHTNESS_PERCENT || value > 100) {
        g_dbus_method_invocation_return_error_literal(invocation, G_IO_ERROR, G_IO_ERROR_EXISTS, "input must be in interval: [MinPercentage, 100]");
        watcher_brightness_complete_set_brightness(skeleton, invocation);
        return;
    }

    guint current_value = watcher_brightness_get_percentage(skeleton);
    if (value == current_value) {
        watcher_brightness_complete_set_brightness(skeleton, invocation);
        return;
    }

    guint true_value = ceil(value / 100.0 * max_brightness);
    write_brightness(true_value);
    printf("brightness set to %d\n", true_value);

    watcher_brightness_complete_set_brightness(skeleton, invocation);
}

static void on_handle_inc_brightness(WatcherBrightness *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value <= 0) {
        watcher_brightness_complete_inc_brightness(skeleton, invocation);
        return;
    }

    guint current_value = watcher_brightness_get_percentage(skeleton);
    guint target_value = current_value + value;
    if (target_value > 100)
        target_value = 100;

    if (target_value == current_value) {
        watcher_brightness_complete_inc_brightness(skeleton, invocation);
        return;
    }

    guint true_value = ceil(target_value / 100.0 * max_brightness);
    write_brightness(true_value);
    printf("brightness set to %d\n", true_value);

    watcher_brightness_complete_inc_brightness(skeleton, invocation);
}

static void on_handle_dec_brightness(WatcherBrightness *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value <= 0) {
        watcher_brightness_complete_dec_brightness(skeleton, invocation);
        return;
    }

    guint current_value = watcher_brightness_get_percentage(skeleton);
    gint target_value = current_value - value;
    if (target_value < MIN_BRIGHTNESS_PERCENT)
        target_value = MIN_BRIGHTNESS_PERCENT;
    
    if (target_value == current_value) {
        watcher_brightness_complete_dec_brightness(skeleton, invocation);
        return;
    }

    guint true_value = ceil(target_value / 100.0 * max_brightness);
    write_brightness(true_value);
    printf("brightness set to %d\n", true_value);

    watcher_brightness_complete_dec_brightness(skeleton, invocation);
}

long alsa_get_volume() {
    long min, max, vol;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Master";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &vol);

    return floor((gdouble) vol / max * 100);
}

void alsa_set_volume(guint volume) {
    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Master";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t *elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(handle);
}

gboolean alsa_get_muted() {
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Master";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    if (snd_mixer_selem_has_playback_switch(elem)) {
        int val;
        snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_MONO, &val);
        if (val)
            return FALSE;
        else
            return TRUE;
    }
}

void alsa_toggle_volume() {
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Master";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    if (snd_mixer_selem_has_playback_switch(elem)) {
        int val;
        snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_MONO, &val);
        if (val)
            snd_mixer_selem_set_playback_switch_all(elem, 0);
        else
            snd_mixer_selem_set_playback_switch_all(elem, 1);
    }

    snd_mixer_close(handle);
}

gboolean check_audio_event(GIOChannel *source, GIOCondition condition, gpointer data) {
    snd_ctl_t *ctl = (snd_ctl_t *) data;
    snd_ctl_event_t *event;
    unsigned int mask;
    int err;

    snd_ctl_event_alloca(&event);
    err = snd_ctl_read(ctl, event);
    if (err < 0)
        exit(EXIT_FAILURE); // maybe better if we just ignore with return TRUE

    if (snd_ctl_event_get_type(event) != SND_CTL_EVENT_ELEM)
        return TRUE;
    
    mask = snd_ctl_event_elem_get_mask(event);
    if (!(mask & SND_CTL_EVENT_MASK_VALUE))
        return TRUE;

    watcher_volume_set_percentage(volume_skeleton, alsa_get_volume());
    watcher_volume_set_muted(volume_skeleton, alsa_get_muted());
    printf("alsa event received\n");

    return TRUE;
}

int open_ctl(const char *name, snd_ctl_t **ctlp) {
    snd_ctl_t *ctl;
    int err;

    err = snd_ctl_open(&ctl, name, SND_CTL_READONLY);
    if (err < 0) {
        fprintf(stderr, "Cannot open ctl %s\n", name);
        return err;
    }
    err = snd_ctl_subscribe_events(ctl, 1);
    if (err < 0) {
        fprintf(stderr, "Cannot open subscribe events to ctl %s\n", name);
        snd_ctl_close(ctl);
        return err;
    }
    *ctlp = ctl;
    return 0;
}

static void start_volume_monitoring() {
    snd_ctl_t *ctl;
    int err = 0;

    err = open_ctl("default", &ctl);
    if (err < 0) {
        snd_ctl_close(ctl);
        exit(EXIT_FAILURE);
    }
    
    snd_ctl_poll_descriptors(ctl, &pfd, 1);

    GIOChannel *channel = g_io_channel_unix_new((&pfd)->fd);
    g_io_add_watch(channel, G_IO_IN, check_audio_event, ctl);

    g_print("monitoring alsa\n");

    watcher_volume_set_percentage(volume_skeleton, alsa_get_volume());
}

static void on_handle_set_volume(WatcherVolume *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value < 0 || value > 100) {
        g_dbus_method_invocation_return_error_literal(invocation, G_IO_ERROR, G_IO_ERROR_EXISTS, "input must be in interval: [MinPercentage, 100]");
        watcher_volume_complete_set_volume(skeleton, invocation);
        return;
    }

    alsa_set_volume(value);
    printf("audio volume set to %d\n", value);

    watcher_volume_complete_set_volume(skeleton, invocation);
}

static void on_handle_inc_volume(WatcherVolume *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value <= 0) {
        watcher_volume_complete_inc_volume(skeleton, invocation);
        return;
    }

    guint current_value = watcher_volume_get_percentage(skeleton);
    guint target_value = current_value + value;
    if (target_value > 100)
        target_value = 100;

    if (target_value == current_value) {
        watcher_volume_complete_inc_volume(skeleton, invocation);
        return;
    }

    alsa_set_volume(target_value);
    printf("audio volume set to %d\n", target_value);
    
    watcher_volume_complete_inc_volume(skeleton, invocation);
}

static void on_handle_dec_volume(WatcherVolume *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    if (value <= 0) {
        watcher_volume_complete_dec_volume(skeleton, invocation);
        return;
    }

    guint current_value = watcher_volume_get_percentage(skeleton);
    gint target_value = current_value - value;
    if (target_value < 0)
        target_value = 0;
    
    if (target_value == current_value) {
        watcher_volume_complete_dec_volume(skeleton, invocation);
        return;
    }

    alsa_set_volume(target_value);
    printf("audio volume set to %d\n", target_value);

    watcher_volume_complete_dec_volume(skeleton, invocation);
}

static void on_handle_toggle_volume(WatcherVolume *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    alsa_toggle_volume();
    watcher_volume_complete_toggle_volume(skeleton, invocation);
}

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data) {
    // TODO: add eror handling for all lines where it is possible
    
    // setup brightness interface
    brightness_skeleton = watcher_brightness_skeleton_new();

    g_signal_connect(brightness_skeleton, "handle-set-brightness", G_CALLBACK(on_handle_set_brightness), NULL);
    g_signal_connect(brightness_skeleton, "handle-inc-brightness", G_CALLBACK(on_handle_inc_brightness), NULL);
    g_signal_connect(brightness_skeleton, "handle-dec-brightness", G_CALLBACK(on_handle_dec_brightness), NULL);

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(brightness_skeleton), connection,
                                     "/fr/mpostaire/Watcher/Brightness", NULL);

    // we can begin to monitor brightness
    start_brightness_monitoring();

    // setup volume interface
    volume_skeleton = watcher_volume_skeleton_new();

    g_signal_connect(volume_skeleton, "handle-set-volume", G_CALLBACK(on_handle_set_volume), NULL);
    g_signal_connect(volume_skeleton, "handle-inc-volume", G_CALLBACK(on_handle_inc_volume), NULL);
    g_signal_connect(volume_skeleton, "handle-dec-volume", G_CALLBACK(on_handle_dec_volume), NULL);
    g_signal_connect(volume_skeleton, "handle-toggle-volume", G_CALLBACK(on_handle_toggle_volume), NULL);

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(volume_skeleton), connection,
                                     "/fr/mpostaire/Watcher/Volume", NULL);
    // we can begin to monitor volume
    start_volume_monitoring();
}

static void on_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data) {
    fprintf(stderr, "Connection lost or already owned.\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    assert(loop);

    // change to SESSION bus
    g_bus_own_name(G_BUS_TYPE_SESSION, "fr.mpostaire.Watcher", G_BUS_NAME_OWNER_FLAGS_NONE, NULL,
                   on_name_acquired, on_name_lost, NULL, NULL);

    g_main_loop_run(loop);

    g_object_unref(file);
    g_main_loop_unref(loop);

    return EXIT_SUCCESS;
}
