#ifndef AUDIOCTL_H
#define AUDIOCTL_H

#include "watcher-dbus.h"
#include <gio/gio.h>
#include <glib.h>

gboolean check_audio_event(GIOChannel *source, GIOCondition condition, gpointer data);

void start_volume_monitoring(WatcherVolume *skeleton);

void alsa_set_volume(guint volume);

void alsa_toggle_volume();

#endif
