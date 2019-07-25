#ifndef BRIGHTNESSCTL_H
#define BRIGHTNESSCTL_H

#include "watcher-dbus.h"
#include <glib.h>

#define MIN_BRIGHTNESS_PERCENT 10
#define BRIGHTNESS_PATH "/sys/class/backlight/radeon_bl0/brightness"
#define MAX_BRIGHTNESS_PATH "/sys/class/backlight/radeon_bl0/max_brightness"

guint get_max_brightness();

void write_brightness(guint value);

void start_brightness_monitoring(WatcherBrightness *skel);

#endif
