#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* alarm으로 read에 시간 제한을 건다.
   default  : 핸들러가 없다. SIGALRM의 기본 동작(종료)이 일어난다.
   norestart: SIGALRM을 잡는다. 핸들러가 끝나면 read가 -1, EINTR로 돌아온다.
   restart  : SA_RESTART를 켠다. 핸들러가 끝나면 read가 다시 기다린다. */

static volatile sig_atomic_t alarms = 0;

static void on_alarm(int sig)
{
    static const char msg[] = "  [handler] SIGALRM\n";
    (void)sig;
    ssize_t r = write(STDOUT_FILENO, msg, sizeof msg - 1);
    (void)r;                    /* 핸들러 안에서는 printf 대신 write */
    alarms++;
}

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char **argv)
{
    long sec = argc == 3 ? strtol(argv[2], NULL, 10) : 0;
    if (argc != 3 || sec < 1 || sec > 60 || (strcmp(argv[1], "default") != 0
        && strcmp(argv[1], "norestart") != 0 && strcmp(argv[1], "restart") != 0)) {
        fprintf(stderr, "usage: %s default|norestart|restart seconds(1-60)\n",
                argv[0]);
        return 2;
    }
    if (strcmp(argv[1], "default") != 0) {
        struct sigaction sa;
        memset(&sa, 0, sizeof sa);
        sa.sa_handler = on_alarm;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = strcmp(argv[1], "restart") == 0 ? SA_RESTART : 0;
        if (sigaction(SIGALRM, &sa, NULL) == -1) {
            perror("sigaction");
            return 1;
        }
    }
    printf("mode=%s alarm=%lds: waiting for input\n", argv[1], sec);
    fflush(stdout);

    char buf[256];
    double t0 = now();
    alarm((unsigned)sec);
    ssize_t n = read(STDIN_FILENO, buf, sizeof buf);
    int saved = errno;
    alarm(0);                   /* 남은 알람 취소 */
    double t = now() - t0;
    if (n == -1 && saved == EINTR)
        printf("read: -1 EINTR after %.2fs -> timeout\n", t);
    else if (n == -1)
        printf("read: -1 %s\n", strerror(saved));
    else
        printf("read: %zd bytes after %.2fs (alarms=%d)\n", n, t, (int)alarms);
    return n == -1;
}
