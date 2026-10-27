#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* pause와 sigsuspend의 차이입니다. 두 모드 다 SIGUSR1을 블록한 채
   raise로 보류를 만들어 놓고 기다립니다. 보류 중인 시그널은
   pause를 깨우지 못해 3초 뒤 alarm에 구제되지만, sigsuspend는
   마스크를 원자적으로 바꿔 보류 시그널을 즉시 받고 돌아올 때
   원래 마스크로 복구합니다. 검사와 대기 사이의 경쟁 창이
   없다는 것이 sigsuspend의 핵심입니다. */

static volatile sig_atomic_t usr1 = 0;
static volatile sig_atomic_t alarms = 0;

static void on_usr1(int sig)
{
    static const char msg[] = "  [handler] SIGUSR1 delivered\n";
    (void)sig;
    ssize_t r = write(STDOUT_FILENO, msg, sizeof msg - 1);
    (void)r;
    usr1++;
}

static void on_alarm(int sig)
{
    static const char msg[] = "  [handler] SIGALRM\n";
    (void)sig;
    ssize_t r = write(STDOUT_FILENO, msg, sizeof msg - 1);
    (void)r;
    alarms++;
}

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void setup_handlers(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_usr1;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_alarm;   /* 구제용 안전망(alarm이 pause를 깨운다) */
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

static sigset_t block_usr1(sigset_t *old)
{
    sigset_t block;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &block, old) == -1) {
        perror("sigprocmask");
        exit(1);
    }
    return block;
}

static void make_pending(void)
{
    sigset_t pend;
    raise(SIGUSR1);             /* 블록되어 있으므로 보류만 된다 */
    sigpending(&pend);
    printf("  SIGUSR1 raised and pending (blocked): usr1=%d\n", (int)usr1);
}

int main(int argc, char **argv)
{
    sigset_t old;
    if (argc != 2
        || (strcmp(argv[1], "pause") != 0 && strcmp(argv[1], "suspend") != 0)) {
        fprintf(stderr, "usage: %s pause|suspend\n", argv[0]);
        return 2;
    }
    setup_handlers();
    block_usr1(&old);
    make_pending();
    double t0 = now();
    if (strcmp(argv[1], "pause") == 0) {
        alarm(3);               /* pause가 영원히 돌아오지 않는 안전망 */
        printf("pause(): waiting — a pending BLOCKED signal cannot wake it\n");
        fflush(stdout);
        int rc = pause();
        double t = now() - t0;
        alarm(0);
        printf("pause: %d (%s) after %.2fs -> woke by ALRM, usr1=%d\n",
               rc, strerror(errno), t, (int)usr1);
        printf("restore mask: the pending SIGUSR1 is delivered now\n");
        fflush(stdout);
        sigprocmask(SIG_SETMASK, &old, NULL);
        printf("after restore: usr1=%d\n", (int)usr1);
    } else {
        sigset_t empty;
        alarm(10);              /* 이론상 오지 않는 안전망 */
        printf("sigsuspend(&empty): waiting — mask swaps atomically\n");
        fflush(stdout);
        sigemptyset(&empty);
        int rc = sigsuspend(&empty);
        double t = now() - t0;
        printf("sigsuspend: %d (%s) after %.2fs -> woke at once, usr1=%d\n",
               rc, strerror(errno), t, (int)usr1);
        sigset_t cur;
        sigprocmask(SIG_BLOCK, NULL, &cur);
        printf("mask restored: %s\n",
               sigismember(&cur, SIGUSR1) ? "SIGUSR1 blocked again" : "NOT blocked?");
        alarm(0);
    }
    return 0;
}
