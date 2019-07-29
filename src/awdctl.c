/*
 * TODO: check for errors return values where its possible
 * TODO: power saving features (timers to dim screen, etc...). Only do this if no alternatives. (check acpi)
 * 
 * can add more interfaces to monitor more things in the future
 */
#include "audioctl.h"
#include "brightnessctl.h"
#include "mpdctl.h"
#include <alsa/asoundlib.h>
#include <assert.h>
#include <getopt.h>
#include <gio/gio.h>
#include <glib-unix.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int no_mpd = 0, no_alsa = 0, no_brightness = 0, daemonize = 0;

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data) {
    if (!no_brightness)
        brightnessctl_start(connection);

    if (!no_alsa)
        audioctl_start(connection);

    if (!no_mpd)
        mpdctl_start(connection);
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

static void start_monitoring() {
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

    if (!no_alsa)
        audioctl_close();
    if (!no_brightness)
        brightnessctl_close();
    if (!no_mpd)
        mpdctl_close();
}

static void usage() {
    g_print("Usage: awdctl [OPTIONS]\n\
  -d, --daemon\t\tLaunch as a daemon.\n\
  -h, --help\t\tShow this message.\n\
  --no-mpd\t\tDisable mpd monitoring and dbus interface.\n\
  --no-alsa\t\tDisable alsa monitoring and dbus interface.\n\
  --no-brightness\tDisable brightness monitoring and dbus interface.\n");
}

int main(int argc, char **argv) {
    int opt, opt_index;
    struct option long_options[] = {
        {"daemon", no_argument, NULL, 'd'},
        {"help", no_argument, NULL, 'h'},
        {"no-mpd", no_argument, &no_mpd, 1},
        {"no-alsa", no_argument, &no_alsa, 1},
        {"no-brightness", no_argument, &no_brightness, 1},
        {NULL, 0, NULL, 0}};

    while ((opt = getopt_long(argc, argv, "hd", long_options, &opt_index)) != -1) {
        switch (opt) {
        case 0:
            break;
        case 'd':
            daemonize = 1;
            break;
        case 'h':
            usage();
            return EXIT_SUCCESS;
            break;
        default:
            usage();
            return EXIT_FAILURE;
        }
    }

    if (no_alsa && no_mpd && no_brightness) {
        g_print("There is nothing to monitor, exiting now.\n");
        return EXIT_SUCCESS;
    }

    if (daemonize) {
        if (daemon(0, 0) == -1) {
            fprintf(stderr, "failed to daemonize\n");
            return EXIT_FAILURE;
        }
    }

    start_monitoring();

    return EXIT_SUCCESS;
}
