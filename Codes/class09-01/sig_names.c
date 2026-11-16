#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 시그널 번호와 이름의 대응을 보여 주는 참조 도구입니다.
   인자가 없으면 1-31을 표로, 인자 N이면 그 번호의 이름·설명·
   기본 동작을 출력합니다. 이름과 기본 동작 표는 x86-64 리눅스
   번호 기준이고 환경마다 다를 수 있으므로 kill -l과 strsignal 열,
   signal(7) 매뉴얼과 서로 대조해 보세요(같은 번호가 환경마다
   다른 시그널을 가리킬 수 있습니다). */

static const char *names[32] = {
    "?", "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "BUS", "FPE",
    "KILL", "USR1", "SEGV", "USR2", "PIPE", "ALRM", "TERM", "STKFLT",
    "CHLD", "CONT", "STOP", "TSTP", "TTIN", "TTOU", "URG", "XCPU",
    "XFSZ", "VTALRM", "PROF", "WINCH", "IO", "PWR", "SYS"};

/* 기본 동작: T=Term(종료), C=Core(종료+코어 덤프), S=Stop(정지),
   c=Cont(재개), I=Ignore(무시). 리눅스 signal(7)의 표를 옮겼습니다. */
static const char acts[32] = {
    '?', 'T', 'T', 'C', 'C', 'C', 'C', 'C',
    'C', 'T', 'T', 'C', 'T', 'T', 'T', 'T',
    'T', 'I', 'c', 'S', 'S', 'S', 'S', 'I',
    'C', 'C', 'T', 'T', 'I', 'T', 'T', 'C'};

static const char *act_name(char c)
{
    switch (c) {
    case 'T': return "Term";
    case 'C': return "Core";
    case 'S': return "Stop";
    case 'c': return "Cont";
    case 'I': return "Ignore";
    }
    return "?";
}

static void print_row(int sig)
{
    printf("%2d) %-6s %-30s default=%s\n", sig, names[sig],
           strsignal(sig), act_name(acts[sig]));
}

int main(int argc, char **argv)
{
    if (argc > 2) {
        fprintf(stderr, "usage: %s [signal 1-31]\n", argv[0]);
        return 2;
    }
    if (argc == 2) {
        char *end = NULL;
        long sig = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || sig < 1 || sig > 31) {
            fprintf(stderr, "usage: %s [signal 1-31]\n", argv[0]);
            return 2;
        }
        print_row((int)sig);
        return 0;
    }
    for (int sig = 1; sig <= 31; sig++)
        print_row(sig);
    printf("32-64: RT(real-time) 범위입니다. 이 과목 범위에서는 다루지 않습니다.\n");
    return 0;
}
