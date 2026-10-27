#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 블록과 보류(block & pending)입니다. 마스크에 넣어 둔 시그널은
   도착해도 전달되지 않고 SigPnd(보류)에 쌓입니다.
   hold [초]: SIGINT를 블록한 임계 영역을 유지합니다. 그 사이 눌린
              Ctrl-C는 핸들러를 실행하지 못하고 보류되며, 풀리는
              순간 즉시 전달됩니다. sigpending으로 자기 보고합니다.
   show PID : /proc/PID/status의 SigPnd·ShdPnd·SigBlk를 시그널 이름으로
              풉니다. 보류 줄은 둘이다: SigPnd는 스레드별, ShdPnd는
              프로세스 공용이다. kill PID가 채우는 것은 ShdPnd다.
              Linux 전용입니다(macOS에는 /proc가 없습니다). */

static const char *names[32] = {
    "?", "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "BUS", "FPE",
    "KILL", "USR1", "SEGV", "USR2", "PIPE", "ALRM", "TERM", "STKFLT",
    "CHLD", "CONT", "STOP", "TSTP", "TTIN", "TTOU", "URG", "XCPU",
    "XFSZ", "VTALRM", "PROF", "WINCH", "IO", "PWR", "SYS"};

static volatile sig_atomic_t ints = 0;

static void on_int(int sig)
{
    static const char msg[] = "  [handler] SIGINT delivered\n";
    (void)sig;
    ssize_t r = write(STDOUT_FILENO, msg, sizeof msg - 1);
    (void)r;                    /* 핸들러 안에서는 printf 대신 write */
    ints++;
}

static void decode(const char *line)
{
    char label[16];
    unsigned long long mask;
    if (sscanf(line, "%15[^:]: %llx", label, &mask) != 2)
        return;
    printf("%-6s %016llx ", label, mask);
    if (mask == 0)
        printf(" -");
    for (int sig = 1; sig <= 31; sig++)
        if (mask & (1ULL << (sig - 1)))
            printf(" %s", names[sig]);
    printf("\n");
}

static int do_hold(long sec)
{
    struct sigaction sa;
    sigset_t block, old, pend;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_int;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    sigemptyset(&block);
    sigaddset(&block, SIGINT);
    if (sigprocmask(SIG_BLOCK, &block, &old) == -1) {
        perror("sigprocmask");
        return 1;
    }
    printf("pid=%ld SIGINT blocked for %lds: press Ctrl-C now\n",
           (long)getpid(), sec);
    printf("observe: ./block_pending show %ld  (another terminal, Linux)\n",
           (long)getpid());
    printf("observe: ./sigmask_show %ld           (first-session tool)\n",
           (long)getpid());
    fflush(stdout);
    int reported = 0;
    for (long i = 0; i < sec; i++) {
        sleep(1);
        if (sigpending(&pend) == -1) {
            perror("sigpending");
            return 1;
        }
        if (sigismember(&pend, SIGINT) && !reported) {
            reported = 1;
            printf("  pending: Ctrl-C arrived, handler is NOT running\n");
            fflush(stdout);
        }
    }
    printf("hold over: unblocking (a pending SIGINT is delivered now)\n");
    fflush(stdout);
    if (sigprocmask(SIG_SETMASK, &old, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }
    printf("after unblock: ints=%d\n", (int)ints);
    return 0;
}

static int do_show(const char *pid)
{
    if (access("/proc/self/status", F_OK) == -1) {
        printf("show=unsupported: no /proc on this OS (Linux only)\n");
        return 0;
    }
    char path[64];
    snprintf(path, sizeof path, "/proc/%s/status", pid);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror(path);
        return 1;
    }
    char line[256];
    while (fgets(line, sizeof line, fp) != NULL)
        if (strncmp(line, "SigPnd:", 7) == 0
            || strncmp(line, "ShdPnd:", 7) == 0
            || strncmp(line, "SigBlk:", 7) == 0)
            decode(line);
    fclose(fp);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc >= 2 && strcmp(argv[1], "hold") == 0) {
        long sec = argc == 3 ? strtol(argv[2], NULL, 10) : 10;
        if (argc > 3 || sec < 1 || sec > 60) {
            fprintf(stderr, "usage: %s hold [seconds 1-60]\n", argv[0]);
            return 2;
        }
        return do_hold(sec);
    }
    if (argc == 3 && strcmp(argv[1], "show") == 0)
        return do_show(argv[2]);
    fprintf(stderr, "usage: %s hold [seconds 1-60] | show PID\n", argv[0]);
    return 2;
}
