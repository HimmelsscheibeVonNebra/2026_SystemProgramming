CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
PYTHON ?= python3

DIRECTORY_PROGRAMS = dir_create dir_list dir_chdir dir_remove
PROGRAMS = mystat mini_ls $(DIRECTORY_PROGRAMS)

all: $(PROGRAMS)

mystat: mystat.c

mini_ls: mini_ls.c

dir_create: dir_create.c

dir_list: dir_list.c

dir_chdir: dir_chdir.c

dir_remove: dir_remove.c

test-directory: $(DIRECTORY_PROGRAMS)
	$(PYTHON) test_directory.py

clean:
	rm -f $(PROGRAMS)

.PHONY: all clean test-directory
