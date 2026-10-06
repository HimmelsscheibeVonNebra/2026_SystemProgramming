CC = cc
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -D_DARWIN_C_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
PROGRAMS = id_probe arg_show env_dump cwd_probe procinfo

.PHONY: all check clean
all: $(PROGRAMS)
%: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@
check: all
	python3 test_examples.py
clean:
	rm -f $(PROGRAMS)
