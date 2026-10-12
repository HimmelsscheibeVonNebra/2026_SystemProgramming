#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* 이 프로그램은 스스로를 관찰 대상으로 만든다. 기본 모드는 1초씩
   잠들어 ps의 STAT에서 S(sleeping)가, --busy 모드는 CPU를 놓지
   않아 R(running/runnable)이 주로 보인다. busy 모드의 경과는
   CLOCK_MONOTONIC으로 잰다. sleep은 중단되면 남은 시간을 돌려
   줄 수 있지만 오늘은 1초 단위 관찰이 목적이므로 그대로 쓴다. */

static double now_sec(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        perror("clock_gettime");
        exit(1);
    }
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main(int argc, char **argv)
{
    int busy = 0;
    int first = 1;
    long seconds = 10;
    if (argc > 1 && strcmp(argv[1], "--busy") == 0) {
        busy = 1;
        first = 2;
    }
    if (argc - first > 1) {
        fprintf(stderr, "usage: %s [--busy] [seconds 1-600]\n", argv[0]);
        return 2;
    }
    if (argc - first == 1) {
        char *end = NULL;
        seconds = strtol(argv[first], &end, 10);
        if (end == argv[first] || *end != '\0' ||
            seconds < 1 || seconds > 600) {
            fprintf(stderr, "usage: %s [--busy] [seconds 1-600]\n",
                    argv[0]);
            return 2;
        }
    }
    printf("pid=%ld\n", (long)getpid());
    printf("mode=%s seconds=%ld\n", busy ? "busy" : "sleep", seconds);
    printf("observe: ps -o pid,ppid,stat,time,%%cpu,command -p %ld\n",
           (long)getpid());
    printf("observe: grep State /proc/%ld/status\n", (long)getpid());
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    double start = now_sec();
    for (long done = 1; done <= seconds; done++) {
        double next = start + (double)done;
        if (busy) {
            while (now_sec() < next) {
                /* R 관찰용: CPU를 놓지 않고 기다린다 */
            }
        } else {
            sleep(1);
        }
        printf("tick=%ld of %ld\n", done, seconds);
        if (fflush(stdout) == EOF) {
            perror("stdout");
            return 1;
        }
    }
    return 0;
}
