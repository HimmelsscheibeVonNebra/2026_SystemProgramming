#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* 시계마다 값의 기준과 해상도가 다르다.
   clock_gettime이 실패하는 시계는 지원하지 않음으로 표시한다. */
static int show(const char *name, clockid_t clock)
{
    struct timespec now, res;
    if (clock_gettime(clock, &now) == -1) {
        printf("%-24s unsupported (%s)\n", name, strerror(errno));
        return 0;
    }
    if (clock_getres(clock, &res) == -1) {
        perror("clock_getres");
        return 1;
    }
    printf("%-24s now=%lld.%09ld res=%ld.%09ld\n", name,
           (long long)now.tv_sec, now.tv_nsec,
           (long)res.tv_sec, res.tv_nsec);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 2;
    }
    int status = 0;
    status |= show("CLOCK_REALTIME", CLOCK_REALTIME);
    status |= show("CLOCK_MONOTONIC", CLOCK_MONOTONIC);
#ifdef CLOCK_MONOTONIC_RAW
    status |= show("CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW);
#endif
#ifdef CLOCK_BOOTTIME
    status |= show("CLOCK_BOOTTIME", CLOCK_BOOTTIME);
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
    status |= show("CLOCK_PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID);
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
    status |= show("CLOCK_THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID);
#endif
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return status;
}
