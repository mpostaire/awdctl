# awdctl
Monitoring program I use to control my laptop's screen brightness and alsa volume via dbus.

## Installation
Clone this repository, cd in the cloned directory then use `make install`. To uninstall use `make uninstall`.

## Usage
Open a terminal and launch the program using the `awdctl` command. You can use `awdctl -d` to run it as a daemon.

You can now monitor/control the screen brightness and alsa volume using dbus. Use a tool like `d-feet` to inspect the dbus interfaces under the name `fr.mpostaire.awdctl` in the session bus.

## TODO
- Figure out how to make it work as a systemd service.
