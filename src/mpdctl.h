#ifndef MPDCTL_H
#define MPDCTL_H

#include <gio/gio.h>

void mpdctl_start(GDBusConnection *connection);

void mpdctl_close();

#endif
