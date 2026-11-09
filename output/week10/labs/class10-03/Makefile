CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -O2 -g
PROGS   = forkpool sigctl

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

test: all
	python3 test_examples.py

clean:
	rm -f $(PROGS)
