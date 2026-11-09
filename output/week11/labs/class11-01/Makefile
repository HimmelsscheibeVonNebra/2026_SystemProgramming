CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g
PROGS   = mapcat mapmode countnl

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

test: all
	python3 test_examples.py

clean:
	rm -f $(PROGS) hello.txt empty.txt big.txt
