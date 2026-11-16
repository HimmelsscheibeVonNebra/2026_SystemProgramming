# Week10-class01 · 프로세스 생성과 실행 (fork · exec · wait)
# 예제 출처: https://github.com/bluekds/SystemProgramming/tree/main/Lecture7 (SPIN Lab, 김덕수)
CC = cc
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -D_DARWIN_C_SOURCE
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
PROGRAMS = fork execlp execv forkNexec wait waitpid atexit

.PHONY: all clean check
all: $(PROGRAMS)
%: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@
check: all
	python3 test_examples.py
clean:
	rm -f $(PROGRAMS)
