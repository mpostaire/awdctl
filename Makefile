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

$(ODIR)/$(EXEC)-dbus.o: fr.mpostaire.$(EXEC).xml
	rm -rf $(EXEC)
	gdbus-codegen --interface-prefix fr.mpostaire. --output-directory $(SDIR) --generate-c-code $(EXEC)-dbus fr.mpostaire.$(EXEC).xml
	$(CC) -o $(ODIR)/$(EXEC)-dbus.o -c $(SDIR)/$(EXEC)-dbus.c $(CFLAGS) $(LDLIBS)

out:
	mkdir $@

run: all
	./$(EXEC)

install: all
	install -m 755 $(EXEC) /usr/bin/

uninstall:
	rm -f /usr/bin/$(EXEC)

clean:
	rm -f $(OBJ)

cleaner: clean
	rm -f $(EXEC) $(SDIR)/$(EXEC)-dbus.[ch]

.PHONY: all clean cleaner
