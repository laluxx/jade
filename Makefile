CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
TARGET = jade
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
