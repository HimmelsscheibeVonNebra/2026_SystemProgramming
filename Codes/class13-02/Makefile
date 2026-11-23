CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g
LDLIBS  = -lrt -pthread
PROGS   = shmhello shmrace semcount ringbuf thcount thqueue thlimit

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

shmrace semcount: counter.h
ringbuf: ring.h

test: all
	python3 test_examples.py

clean:
	rm -f $(PROGS)
