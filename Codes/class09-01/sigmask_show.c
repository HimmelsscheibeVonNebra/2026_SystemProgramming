#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* /proc/PID/status의 Sig* 줄을 읽어 비트를 시그널 이름으로 푼다.
   비트 n-1(오른쪽 끝이 비트 0)이 시그널 n이다. 인자가 없으면 self.
   이름 표는 x86-64 리눅스 번호 기준이고, 32 이상은 RT(실시간)로 적는다. */

static const char *names[32] = {
    "?", "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "BUS", "FPE",
    "KILL", "USR1", "SEGV", "USR2", "PIPE", "ALRM", "TERM", "STKFLT",
    "CHLD", "CONT", "STOP", "TSTP", "TTIN", "TTOU", "URG", "XCPU",
    "XFSZ", "VTALRM", "PROF", "WINCH", "IO", "PWR", "SYS"};

static void decode(const char *line)
{
    char label[16];
    unsigned long long mask;
    if (sscanf(line, "%15[^:]: %llx", label, &mask) != 2)
        return;
    printf("%-6s %016llx ", label, mask);
    if (mask == 0)
        printf(" -");
    for (int sig = 1; sig <= 64; sig++) {
        if (!(mask & (1ULL << (sig - 1))))
            continue;
        if (sig < 32)
            printf(" %s", names[sig]);
        else
            printf(" RT%d", sig);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    if (argc > 2) {
        fprintf(stderr, "usage: %s [pid]\n", argv[0]);
        return 2;
    }
    char path[64];
    snprintf(path, sizeof path, "/proc/%s/status", argc == 2 ? argv[1] : "self");
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror(path);
        return 1;
    }
    char line[256];
    while (fgets(line, sizeof line, fp) != NULL) {
        if (strncmp(line, "Name:", 5) == 0 || strncmp(line, "Pid:", 4) == 0)
            fputs(line, stdout);
        else if (strncmp(line, "SigPnd:", 7) == 0 || strncmp(line, "ShdPnd:", 7) == 0
                 || strncmp(line, "SigBlk:", 7) == 0 || strncmp(line, "SigIgn:", 7) == 0
                 || strncmp(line, "SigCgt:", 7) == 0)
            decode(line);
    }
    fclose(fp);
    return 0;
}
