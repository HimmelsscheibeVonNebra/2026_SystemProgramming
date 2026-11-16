CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g
PROGS   = pipe_basic pipemsg pipe2 pipeio

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

test: all
	python3 test_examples.py

clean:
	rm -f $(PROGS) out.txt
