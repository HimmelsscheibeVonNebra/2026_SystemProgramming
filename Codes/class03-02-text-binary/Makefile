CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -Werror -g
PROGRAMS = file_write file_read file_seek binary_write binary_read binary_seek test_write_all
.PHONY: all test clean
all: $(PROGRAMS)
%: %.c
	$(CC) $(CFLAGS) $< -o $@
file_write file_read file_seek test_write_all: io_helpers.h
test: all
	./test_write_all
	python3 test_io.py
clean:
	rm -f $(PROGRAMS)
