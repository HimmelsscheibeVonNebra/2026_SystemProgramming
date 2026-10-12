#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

/* 같은 1초라도 CPU를 어떻게 쓰는지에 따라 사용량 숫자가 달라진다.
   busy는 사용자 모드 계산, sys는 짧은 시스템 호출 반복, sleep은
   잠자기를 반복한다. 모드마다 getrusage 차이로 user·sys CPU 시간과
   자발적(nvcsw)·비자발적(nivcsw) 문맥 교환 횟수를 보고한다. */

static double now_sec(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        perror("clock_gettime");
        exit(1);
    }
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static double tv(struct timeval t)
{
    return (double)t.tv_sec + (double)t.tv_usec / 1e6;
}

static void run(const char *mode, double seconds, int devnull)
{
    struct rusage a, b;
    if (getrusage(RUSAGE_SELF, &a) == -1) {
        perror("getrusage");
        exit(1);
    }
    double start = now_sec(), stop = start + seconds;
    volatile unsigned long spin = 0;
    char byte = 'x';
    while (now_sec() < stop) {
        if (strcmp(mode, "busy") == 0) {
            for (int i = 0; i < 100000; i++)
                spin++;
        } else if (strcmp(mode, "sys") == 0) {
            if (write(devnull, &byte, 1) == -1) {
                perror("write");
                exit(1);
            }
        } else {
            usleep(10000);   /* 10ms 잠들기 */
        }
    }
    double wall = now_sec() - start;
    if (getrusage(RUSAGE_SELF, &b) == -1) {
        perror("getrusage");
        exit(1);
    }
    printf("mode=%-5s wall=%.2f user=%.2f sys=%.2f nvcsw=%ld nivcsw=%ld\n",
           mode, wall, tv(b.ru_utime) - tv(a.ru_utime),
           tv(b.ru_stime) - tv(a.ru_stime),
           b.ru_nvcsw - a.ru_nvcsw, b.ru_nivcsw - a.ru_nivcsw);
    if (fflush(stdout) == EOF) {
        perror("stdout");
        exit(1);
    }
}

int main(int argc, char **argv)
{
    const char *modes[] = {"busy", "sys", "sleep"};
    const char *only = argc == 2 ? argv[1] : NULL;
    if (argc > 2 || (only != NULL && strcmp(only, "busy") != 0 &&
                     strcmp(only, "sys") != 0 && strcmp(only, "sleep") != 0)) {
        fprintf(stderr, "usage: %s [busy|sys|sleep]\n", argv[0]);
        return 2;
    }
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull == -1) {
        perror("/dev/null");
        return 1;
    }
    printf("pid=%ld seconds_per_mode=1\n", (long)getpid());
    for (int i = 0; i < 3; i++)
        if (only == NULL || strcmp(only, modes[i]) == 0)
            run(modes[i], 1.0, devnull);
    if (close(devnull) == -1) {
        perror("close");
        return 1;
    }
    return 0;
}
