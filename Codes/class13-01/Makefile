CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g
LDLIBS  = -lrt
PROGS   = isolated pmq mqserver mqclient

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

mqserver mqclient: proto.h

test: all
	python3 test_examples.py

clean:
	rm -f $(PROGS)
