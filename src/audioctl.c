#include "watcher-dbus.h"
#include <alsa/asoundlib.h>
#include <glib.h>
#include <stdlib.h>

struct data {
    snd_ctl_t *ctl;
    WatcherVolume *skeleton;
};

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
    snd_mixer_elem_t *elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &vol);

    snd_mixer_close(handle);

    return (gfloat) vol / max * 100;
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

gboolean check_audio_event(GIOChannel *source, GIOCondition condition, gpointer data) {
    struct data *args = (struct data *) data;

    snd_ctl_event_t *event;
    unsigned int mask;
    int err;

    snd_ctl_event_alloca(&event);
    err = snd_ctl_read(args->ctl, event);
    if (err < 0)
        exit(EXIT_FAILURE); // maybe better if we just ignore with return TRUE

    if (snd_ctl_event_get_type(event) != SND_CTL_EVENT_ELEM)
        return TRUE;

    mask = snd_ctl_event_elem_get_mask(event);
    if (!(mask & SND_CTL_EVENT_MASK_VALUE))
        return TRUE;

    watcher_volume_set_percentage(args->skeleton, alsa_get_volume());
    watcher_volume_set_muted(args->skeleton, alsa_get_muted());
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

void start_volume_monitoring(WatcherVolume *skeleton) {
    snd_ctl_t *ctl;
    int err = 0;

    err = open_ctl("default", &ctl);
    if (err < 0) {
        snd_ctl_close(ctl);
        exit(EXIT_FAILURE);
    }

    struct pollfd pfd;
    snd_ctl_poll_descriptors(ctl, &pfd, 1);

    struct data *args = malloc(sizeof(struct data));
    if (args == NULL) {
        fprintf(stderr, "malloc in start_volume_monitoring() failed\n");
        exit(EXIT_FAILURE);
    }
    args->ctl = ctl;
    args->skeleton = skeleton;

    GIOChannel *channel = g_io_channel_unix_new(pfd.fd);
    g_io_add_watch(channel, G_IO_IN, check_audio_event, args);

    g_print("monitoring alsa\n");

    // update initial audio volume properties
    watcher_volume_set_percentage(skeleton, alsa_get_volume());
    watcher_volume_set_muted(skeleton, alsa_get_muted());
}
