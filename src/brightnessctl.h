#ifndef BRIGHTNESSCTL_H
#define BRIGHTNESSCTL_H

#include <gio/gio.h>

#define MIN_BRIGHTNESS_PERCENT 10

void brightnessctl_export(GDBusConnection *connection);

void brightnessctl_start();

void brightnessctl_close();

#endif
