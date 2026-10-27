#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* async-signal-safe 핸들러 규범입니다. 신호는 어느 코드 사이에든
   끼어들 수 있으므로 핸들러는 아주 조심해야 합니다: errno를
   저장·복원하고, write()만 부르고, volatile sig_atomic_t 플래그만
   고칩니다. printf는 내부 잠금과 stdio 버퍼 상태를 만지므로
   핸들러 안에서 부르면 잠금 교착이나 뒤섞인 출력을 낳을 수
   있습니다. 보고는 전부 main이 플래그를 보고 합니다.
   self N : 자기에게 SIGINT를 N번 raise하고 플래그 전이를 봅니다.
   wait N : N초 동안 Ctrl-C를 기다렸다가 받은 횟수를 보고합니다. */

static volatile sig_atomic_t ints = 0;

static void on_int(int sig)
{
    int saved = errno;          /* 끼어들기 전의 errno를 지킨다 */
    static const char msg[] = "  [handler] SIGINT\n";
    (void)sig;
    ssize_t r = write(STDOUT_FILENO, msg, sizeof msg - 1);
    (void)r;
    ints++;                     /* 플래그에 한 번의 덧셈만 한다 */
    errno = saved;
}

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char **argv)
{
    long n = 0;
    if (argc == 3) {
        char *end = NULL;
        n = strtol(argv[2], &end, 10);
        if (end == argv[2] || *end != '\0' || n < 1 || n > 99)
            n = -1;
    }
    if (n == -1 || argc != 3
        || (strcmp(argv[1], "self") != 0 && strcmp(argv[1], "wait") != 0)) {
        fprintf(stderr, "usage: %s self N(1-99) | wait N(1-99) seconds\n",
                argv[0]);
        return 2;
    }
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_int;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    if (strcmp(argv[1], "self") == 0) {
        errno = 77;             /* 관찰용 임의 값 */
        for (long i = 1; i <= n; i++) {
            raise(SIGINT);      /* 블록되지 않았으므로 raise 안에서 전달 */
            if (ints == i)
                printf("main: saw flag=%d after raise #%ld\n", (int)ints, i);
            else
                printf("main: flag=%d expected %ld?!\n", (int)ints, i);
        }
        int preserved = (errno == 77);
        printf("errno preserved across handler: %s (expected %d)\n",
               preserved ? "yes" : "NO", 77);
        printf("total ints=%d\n", (int)ints);
    } else {
        printf("pid=%ld waiting %lds for Ctrl-C\n", (long)getpid(), n);
        fflush(stdout);
        double t0 = now();
        long seen = 0;
        while (now() - t0 < (double)n) {
            sleep(1);           /* Ctrl-C가 오면 일찍 돌아온다 */
            if (ints > seen) {
                seen = ints;
                printf("tick %.0fs: received=%ld\n", now() - t0, seen);
            }
        }
        printf("received %d SIGINT in %lds\n", (int)ints, n);    }
    return 0;
}
