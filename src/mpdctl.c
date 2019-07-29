#include "awdctl-dbus.h"
#include <gio/gio.h>
#include <glib.h>
#include <mpd/async.h>
#include <mpd/client.h>
#include <stdio.h>
#include <stdlib.h>

static AwdctlMpd *mpd_skeleton;
static struct mpd_connection *mpd;

static void mpdctl_toggle_pause() {
    // we leave idle mode
    mpd_run_noidle(mpd);

    bool ret = mpd_run_toggle_pause(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not toggle pause\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }

    // we reenter idle mode
    ret = mpd_send_idle(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not set idle state\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }
}

static void mpdctl_previous() {
    // we leave idle mode
    mpd_run_noidle(mpd);

    bool ret = mpd_run_previous(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not change to previous track\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }

    // we reenter idle mode
    ret = mpd_send_idle(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not set idle state\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }
}

static void mpdctl_stop() {
    // we leave idle mode
    mpd_run_noidle(mpd);

    bool ret = mpd_run_stop(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not stop\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }

    // we reenter idle mode
    ret = mpd_send_idle(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not set idle state\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }
}

static void mpdctl_next() {
    // we leave idle mode
    mpd_run_noidle(mpd);

    bool ret = mpd_run_next(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not change to next track\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }

    // we reenter idle mode
    ret = mpd_send_idle(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not set idle state\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }
}

static void mpdctl_play() {
    // we leave idle mode
    mpd_run_noidle(mpd);

    bool ret = mpd_run_play(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not start playing\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }

    // we reenter idle mode
    ret = mpd_send_idle(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not set idle state\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }
}

static void mpdctl_toggle_repeat() {
    // we leave idle mode
    mpd_run_noidle(mpd);

    gboolean repeat = awdctl_mpd_get_repeat(mpd_skeleton);
    bool ret = mpd_run_repeat(mpd, !repeat);
    if (!ret) {
        fprintf(stderr, "mpd: could not set repeat state\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }

    // we reenter idle mode
    ret = mpd_send_idle(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not set idle state\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }
}

static void mpdctl_toggle_random() {
    // we leave idle mode
    mpd_run_noidle(mpd);

    gboolean random = awdctl_mpd_get_random(mpd_skeleton);
    bool ret = mpd_run_random(mpd, !random);
    if (!ret) {
        fprintf(stderr, "mpd: could not set random state\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }

    // we reenter idle mode
    ret = mpd_send_idle(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not set idle state\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }
}

static void awdctl_update_status() {
    struct mpd_status *status = mpd_run_status(mpd);
    if (status == NULL) {
        fprintf(stderr, "mpd: could not get status\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }

    awdctl_mpd_set_length(mpd_skeleton, mpd_status_get_total_time(status));
    awdctl_mpd_set_volume(mpd_skeleton, mpd_status_get_volume(status));
    awdctl_mpd_set_play_state(mpd_skeleton, mpd_status_get_state(status));
    awdctl_mpd_set_repeat(mpd_skeleton, mpd_status_get_repeat(status));
    awdctl_mpd_set_random(mpd_skeleton, mpd_status_get_random(status));

    mpd_status_free(status);
}

static void awdtcl_update_song() {
    struct mpd_song *song = mpd_run_current_song(mpd);
    if (song == NULL) {
        fprintf(stderr, "mpd: could not get song\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }

    awdctl_mpd_set_title(mpd_skeleton, mpd_song_get_tag(song, MPD_TAG_TITLE, 0));
    awdctl_mpd_set_artist(mpd_skeleton, mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));
    awdctl_mpd_set_album(mpd_skeleton, mpd_song_get_tag(song, MPD_TAG_ALBUM, 0));
    awdctl_mpd_set_path(mpd_skeleton, mpd_song_get_uri(song));

    mpd_song_free(song);
}

static gboolean check_mpd_event(GIOChannel *source, GIOCondition condition, gpointer data) {
    // we receive what type of event it was
    enum mpd_idle flags = mpd_recv_idle(mpd, true);
    const char *buf = mpd_idle_name(flags);
    g_print("mpd: %s event received\n", buf);

    switch (flags) {
    case MPD_IDLE_PLAYER:
        // the player has been started, stopped or seeked
        awdctl_update_status();
        awdtcl_update_song();
        break;
    case MPD_IDLE_OPTIONS:
        // options like repeat, random, crossfade, replay gain
        awdctl_update_status();
        break;
    case MPD_IDLE_MIXER:
        // the volume has been changed
        awdctl_update_status();
        break;
    default:
        fprintf(stderr, "mpd: no support for %s event\n", buf);
        break;
    }

    // we reenter idle mode after parsing
    bool ret = mpd_send_idle(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not set idle state\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }
    return TRUE;
}

static void on_handle_toggle_pause(AwdctlMpd *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    mpdctl_toggle_pause();
    awdctl_mpd_complete_toggle_pause(skeleton, invocation);
}

static void on_handle_previous(AwdctlMpd *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    mpdctl_previous();
    awdctl_mpd_complete_previous(skeleton, invocation);
}

static void on_handle_next(AwdctlMpd *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    mpdctl_next();
    awdctl_mpd_complete_next(skeleton, invocation);
}

static void on_handle_stop(AwdctlMpd *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    mpdctl_stop();
    awdctl_mpd_complete_next(skeleton, invocation);
}

static void on_handle_play(AwdctlMpd *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    mpdctl_play();
    awdctl_mpd_complete_next(skeleton, invocation);
}

static void on_handle_toggle_repeat(AwdctlMpd *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    mpdctl_toggle_repeat();
    awdctl_mpd_complete_next(skeleton, invocation);
}

static void on_handle_toggle_random(AwdctlMpd *skeleton, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    mpdctl_toggle_repeat();
    awdctl_mpd_complete_next(skeleton, invocation);
}

void mpdctl_close() {
    mpd_connection_free(mpd);
}

void mpdctl_start(GDBusConnection *connection) {
    // setup mpd interface
    mpd_skeleton = awdctl_mpd_skeleton_new();

    g_signal_connect(mpd_skeleton, "handle-toggle-pause", G_CALLBACK(on_handle_toggle_pause), NULL);
    g_signal_connect(mpd_skeleton, "handle-previous", G_CALLBACK(on_handle_previous), NULL);
    g_signal_connect(mpd_skeleton, "handle-next", G_CALLBACK(on_handle_next), NULL);
    g_signal_connect(mpd_skeleton, "handle-stop", G_CALLBACK(on_handle_stop), NULL);
    g_signal_connect(mpd_skeleton, "handle-play", G_CALLBACK(on_handle_play), NULL);
    g_signal_connect(mpd_skeleton, "handle-toggle-repeat", G_CALLBACK(on_handle_toggle_repeat), NULL);
    g_signal_connect(mpd_skeleton, "handle-toggle-random", G_CALLBACK(on_handle_toggle_random), NULL);

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(mpd_skeleton), connection,
                                     "/fr/mpostaire/awdctl/Mpd", NULL);

    // we can begin to monitor mpd
    // TODO: args for custom host and port
    // TODO: add password support
    const char *host = "localhost";
    int port = 6600;
    mpd = mpd_connection_new(host, port, 0);

    if (mpd_connection_get_error(mpd) != MPD_ERROR_SUCCESS) {
        const char *err = mpd_connection_get_error_message(mpd);
        fprintf(stderr, "mpd connection failed with error: %s\n", err);
        free((char *) err);
        exit(EXIT_FAILURE);
    }

    mpd_connection_set_keepalive(mpd, true);
    int fd = mpd_connection_get_fd(mpd);

    GIOChannel *channel = g_io_channel_unix_new(fd);
    g_io_add_watch(channel, G_IO_IN, check_mpd_event, NULL);

    // update initial mpd properties
    awdctl_update_status();
    awdtcl_update_song();

    bool ret = mpd_send_idle(mpd);
    if (!ret) {
        fprintf(stderr, "mpd: could not set idle state\n");
        // maybe exiting here is an overkill (we should at least try again).
        exit(EXIT_FAILURE);
    }

    g_print("monitoring mpd at %s:%d\n", host, port);
}
