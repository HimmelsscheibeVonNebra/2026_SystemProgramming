#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* timespec을 나노초 정수로 바꾸고 빼기를 정수로 처리한다. */
static long long ts_ns(const struct timespec *ts)
{
    return (long long)ts->tv_sec * 1000000000LL + ts->tv_nsec;
}

static long long now_ns(clockid_t clock)
{
    struct timespec ts;
    if (clock_gettime(clock, &ts) == -1) {
        perror("clock_gettime");
        exit(1);
    }
    return ts_ns(&ts);
}

static int cmp_ll(const void *a, const void *b)
{
    long long x = *(const long long *)a, y = *(const long long *)b;
    return x < y ? -1 : x > y;
}

/* 재지정 불가 작업량: volatile 배열을 여러 번 훑는다. */
static long long workload(void)
{
    static volatile int a[1000000];
    long long sum = 0;
    for (int pass = 0; pass < 4; ++pass)
        for (int i = 0; i < 1000000; ++i)
            sum += a[i] + i;
    return sum;
}

static void spin_ns(long long ns)
{
    long long start = now_ns(CLOCK_MONOTONIC);
    while (now_ns(CLOCK_MONOTONIC) - start < ns)
        ;
}

static void show_clock(const char *name, clockid_t clock)
{
    struct timespec now, res;
    if (clock_gettime(clock, &now) == -1) {
        printf("%-24s unsupported (%s)\n", name, strerror(errno));
        return;
    }
    if (clock_getres(clock, &res) == -1) {
        perror("clock_getres");
        exit(1);
    }
    printf("%-24s now=%lld.%09ld res=%ld.%09ld\n", name,
           (long long)now.tv_sec, now.tv_nsec,
           (long)res.tv_sec, res.tv_nsec);
}

int main(int argc, char **argv)
{
    int clocks_only = argc == 2 && strcmp(argv[1], "--clocks") == 0;
    if (!(argc == 1 || clocks_only)) {
        fprintf(stderr, "usage: %s [--clocks]\n", argv[0]);
        return 2;
    }
    if (clocks_only) {
        show_clock("CLOCK_REALTIME", CLOCK_REALTIME);
        show_clock("CLOCK_MONOTONIC", CLOCK_MONOTONIC);
#ifdef CLOCK_MONOTONIC_RAW
        show_clock("CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW);
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
        show_clock("CLOCK_PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID);
#endif
        if (fflush(stdout) == EOF) {
            perror("stdout");
            return 1;
        }
        return 0;
    }

    enum { RUNS = 5 };
    long long ns[RUNS];
    long long sink = 0;
    spin_ns(2000000);   /* 워밍업: 첫 측정의 캐시 효과를 줄인다 */
    for (int i = 0; i < RUNS; ++i) {
        long long start = now_ns(CLOCK_MONOTONIC);
        sink += workload();
        ns[i] = now_ns(CLOCK_MONOTONIC) - start;
        printf("run%d=%lldns\n", i + 1, ns[i]);
    }
    qsort(ns, RUNS, sizeof ns[0], cmp_ll);
    printf("min=%lldns median=%lldns max=%lldns sink=%lld\n",
           ns[0], ns[RUNS / 2], ns[RUNS - 1], sink);
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
