#ifndef BRIGHTNESSCTL_H
#define BRIGHTNESSCTL_H

#include "watcher-dbus.h"
#include <glib.h>

#define MIN_BRIGHTNESS_PERCENT 10

guint get_max_brightness();

void write_brightness(guint value);

void start_brightness_monitoring(WatcherBrightness *skel);

#endif
