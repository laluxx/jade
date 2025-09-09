CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = jade
SRC = main.c
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean
