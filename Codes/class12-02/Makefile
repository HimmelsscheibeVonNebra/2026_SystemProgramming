CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g
PROGS   = pipe_two pipetalk pipeline fifosrv

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

test: all
	python3 test_examples.py

clean:
	rm -f $(PROGS)
	rm -rf *.dSYM
