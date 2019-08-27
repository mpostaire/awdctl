#include "awdctl-dbus.h"
#include "volume_mapping.h"
#include <alsa/asoundlib.h>
#include <glib.h>
#include <stdlib.h>

static AwdctlVolume *volume_skeleton;
static snd_ctl_t *ctl;

static guint alsa_get_volume() {
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

    double vol = get_normalized_playback_volume(elem, SND_MIXER_SCHN_MONO);

    snd_mixer_close(handle);

    return vol * 100;
}

static void alsa_set_volume(guint volume) {
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

    set_normalized_playback_volume(elem, SND_MIXER_SCHN_MONO, (double) ((double) volume / 100.0), 1);

    snd_mixer_close(handle);
}

static gboolean alsa_get_muted() {
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

    if (snd_mixer_selem_has_playback_switch(elem)) {
        int val;
        snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_MONO, &val);
        if (val) {
            snd_mixer_close(handle);
            return FALSE;
        } else {
            snd_mixer_close(handle);
            return TRUE;
        }
    }

    snd_mixer_close(handle);
    return FALSE;
}

static void alsa_toggle_volume() {
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

static gboolean check_audio_event(GIOChannel *source, GIOCondition condition, gpointer data) {
    snd_ctl_event_t *event;
    unsigned int mask;
    int err;

    snd_ctl_event_alloca(&event);
    err = snd_ctl_read(ctl, event);
    if (err < 0) {
        fprintf(stderr, "check_audio_event() failed\n");
        exit(EXIT_FAILURE); // maybe better if we just ignore with return TRUE
    }

    if (snd_ctl_event_get_type(event) != SND_CTL_EVENT_ELEM)
        return TRUE;

    mask = snd_ctl_event_elem_get_mask(event);
    if (!(mask & SND_CTL_EVENT_MASK_VALUE))
        return TRUE;

    awdctl_volume_set_percentage(volume_skeleton, alsa_get_volume());
    awdctl_volume_set_muted(volume_skeleton, alsa_get_muted());
    g_print("alsa event received\n");

    return TRUE;
}

static int open_ctl(const char *name, snd_ctl_t **ctlp) {
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

void audioctl_close() {
    // free everything that needs to be freed in audioctl here
    snd_ctl_close(ctl);
    g_object_unref(volume_skeleton);
}

void audioctl_export(GDBusConnection *connection) {
    // setup volume interface
    volume_skeleton = awdctl_volume_skeleton_new();

    g_signal_connect(volume_skeleton, "handle-set-volume", G_CALLBACK(on_handle_set_volume), NULL);
    g_signal_connect(volume_skeleton, "handle-inc-volume", G_CALLBACK(on_handle_inc_volume), NULL);
    g_signal_connect(volume_skeleton, "handle-dec-volume", G_CALLBACK(on_handle_dec_volume), NULL);
    g_signal_connect(volume_skeleton, "handle-toggle-volume", G_CALLBACK(on_handle_toggle_volume), NULL);

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(volume_skeleton), connection,
                                     "/fr/mpostaire/awdctl/Volume", NULL);
}

void audioctl_start() {
    // we can begin to monitor volume
    int err = open_ctl("default", &ctl);
    if (err < 0) {
        snd_ctl_close(ctl);
        fprintf(stderr, "start_volume_monitoring() failed\n");
        exit(EXIT_FAILURE);
    }

    struct pollfd pfd;
    snd_ctl_poll_descriptors(ctl, &pfd, 1);

    GIOChannel *channel = g_io_channel_unix_new(pfd.fd);
    g_io_add_watch(channel, G_IO_IN, check_audio_event, NULL);

    g_print("monitoring alsa\n");

    // update initial audio volume properties
    awdctl_volume_set_percentage(volume_skeleton, alsa_get_volume());
    awdctl_volume_set_muted(volume_skeleton, alsa_get_muted());
}
