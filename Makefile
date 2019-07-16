CFLAGS=`pkg-config --cflags gio-2.0 gio-unix-2.0 glib-2.0`
LDLIBS=`pkg-config --libs gio-2.0 gio-unix-2.0 glib-2.0` -lm
CC=gcc
EXEC=watcher

all: $(EXEC)

$(EXEC): $(EXEC)-dbus.o watcher.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

$(EXEC)-dbus.o: $(EXEC)-dbus.c $(EXEC)-dbus.h
	$(CC) -o $@ -c $< $(CFLAGS) $(LDLIBS)

run: all
	./$(EXEC)

gen: fr.mpostaire.watcher.xml
	gdbus-codegen --interface-prefix com.watcher --interface-prefix fr.mpostaire. --generate-c-code $(EXEC)-dbus fr.mpostaire.watcher.xml

clean:
	rm -rf *.o

cleaner: clean
	rm -rf $(EXEC) $(EXEC)-dbus.[ch]

.PHONY: all clean cleaner
