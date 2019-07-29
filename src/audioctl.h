#ifndef AUDIOCTL_H
#define AUDIOCTL_H

#include <gio/gio.h>

void audioctl_start(GDBusConnection *connection);

void audioctl_close();

#endif
