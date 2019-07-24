CFLAGS=`pkg-config --cflags gio-2.0 gio-unix-2.0 glib-2.0 alsa`
LDLIBS=`pkg-config --libs gio-2.0 gio-unix-2.0 glib-2.0 alsa` -lm
CC=gcc
EXEC=watcher

all: $(EXEC)-dbus.[ch] $(EXEC)

$(EXEC): $(EXEC)-dbus.o watcher.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

$(EXEC)-dbus.o: $(EXEC)-dbus.c $(EXEC)-dbus.h
	$(CC) -o $@ -c $< $(CFLAGS) $(LDLIBS)

$(EXEC)-dbus.[ch]: fr.mpostaire.Watcher.xml
	rm -rf $(EXEC)
	gdbus-codegen --interface-prefix fr.mpostaire. --generate-c-code $(EXEC)-dbus fr.mpostaire.Watcher.xml

run: all
	./$(EXEC)

clean:
	rm -rf *.o

cleaner: clean
	rm -rf $(EXEC) $(EXEC)-dbus.[ch]

.PHONY: all clean cleaner
