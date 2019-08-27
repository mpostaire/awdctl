#ifndef AUDIOCTL_H
#define AUDIOCTL_H

#include <gio/gio.h>

void audioctl_export(GDBusConnection *connection);

void audioctl_start();

void audioctl_close();

#endif
