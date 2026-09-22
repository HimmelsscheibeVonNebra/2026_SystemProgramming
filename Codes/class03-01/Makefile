CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
PROGRAMS = file_create file_write file_read file_seek

all: $(PROGRAMS)

file_create: file_create.c
	$(CC) $(CFLAGS) $< -o $@

file_write file_read file_seek: %: %.c io_helpers.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(PROGRAMS)

.PHONY: all clean
