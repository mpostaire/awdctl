# awdctl
Monitoring program I use to control my laptop's screen brightness, mpd and alsa volume via dbus.
This is a "swiss army knife" tool I made for my awesomewm widgets.

## Installation
Clone this repository, cd in the cloned directory then use `make install`. To uninstall use `make uninstall`.
This program depends on `glib2` and `alsa-lib` so you may need to install these before.

## Usage
Open a terminal and launch the program using the `awdctl` command.

```
Usage: awdctl [OPTIONS]
  -d, --daemon		Launch as a daemon.
  -h, --help		Show this message.
  --no-alsa		Disable alsa monitoring and dbus interface.
  --no-brightness	Disable brightness monitoring and dbus interface.
```

You can now monitor/control the screen brightness and alsa volume using dbus. Use a tool like `d-feet` to inspect the dbus interfaces under the name `fr.mpostaire.awdctl` in the session bus.

## TODO
- Figure out how to make it work as a systemd service.
- Use `g_source_unref()` to free sources such as gio_add_watch etc...
