SDIR=src
ODIR=out
CFLAGS=`pkg-config --cflags gio-2.0 gio-unix-2.0 glib-2.0 alsa`
LDLIBS=`pkg-config --libs gio-2.0 gio-unix-2.0 glib-2.0 alsa` -lm
CC=gcc
EXEC=awdctl
SRC= $(filter-out $(SDIR)/$(EXEC)-dbus.c, $(wildcard $(SDIR)/*.c))
OBJ= $(SRC:$(SDIR)/%.c=$(ODIR)/%.o)
OBJ+=$(ODIR)/$(EXEC)-dbus.o

all: out $(ODIR)/$(EXEC)-dbus.o $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS) $(LDLIBS)

$(ODIR)/$(EXEC)-dbus.o: $(SDIR)/$(EXEC)-dbus.c $(SDIR)/$(EXEC)-dbus.h
	rm -rf $(EXEC)
	$(CC) -o $@ -c $< $(CFLAGS) $(LDLIBS)

$(SDIR)/$(EXEC)-dbus.c: fr.mpostaire.$(EXEC).xml
	gdbus-codegen --interface-prefix fr.mpostaire. --body --output $@ $^

$(SDIR)/$(EXEC)-dbus.h: fr.mpostaire.$(EXEC).xml
	gdbus-codegen --interface-prefix fr.mpostaire. --header --output $@ $^

out:
	mkdir $@

run: all
	./$(EXEC)

install: all backlight.rules
	install -m 0755 $(EXEC) /usr/bin/
	install -m 0644 backlight.rules /lib/udev/rules.d/

uninstall:
	rm -f /usr/bin/$(EXEC)
	rm -f /usr/bin/$(EXEC) /lib/udev/rules.d/backlight.rules

clean:
	rm -f $(OBJ)

cleaner: clean
	rm -f $(EXEC) $(SDIR)/$(EXEC)-dbus.[ch]

.PHONY: all clean cleaner install uninstall run
