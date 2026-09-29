#include <errno.h>
#include <stdio.h>
#include <time.h>

/* 달력 시간의 기본 흐름: time(2)으로 초를 얻고
   localtime·gmtime으로 struct tm을 만들어 strftime으로 문자열로 바꾼다. */
int main(int argc, char **argv)
{
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 2;
    }
    time_t now = time(NULL);
    if (now == (time_t)-1) {
        perror("time");
        return 1;
    }
    printf("epoch=%lld\n", (long long)now);

    char buf[64];
    struct tm *tm_local = localtime(&now);
    if (tm_local == NULL) {
        perror("localtime");
        return 1;
    }
    if (strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S %Z", tm_local) == 0) {
        fprintf(stderr, "strftime: buffer too small\n");
        return 1;
    }
    printf("local=%s\n", buf);

    struct tm *tm_utc = gmtime(&now);
    if (tm_utc == NULL) {
        perror("gmtime");
        return 1;
    }
    if (strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S UTC", tm_utc) == 0) {
        fprintf(stderr, "strftime: buffer too small\n");
        return 1;
    }
    printf("utc=%s\n", buf);

    /* difftime은 time_t끼리의 차를 초 단위 실수로 돌려준다.
       time(2)는 초 단위 해상도이므로 1.2초를 기다려 차이를 만든다. */
    struct timespec start, tick;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        perror("clock_gettime");
        return 1;
    }
    do {
        if (clock_gettime(CLOCK_MONOTONIC, &tick) == -1) {
            perror("clock_gettime");
            return 1;
        }
    } while ((tick.tv_sec - start.tv_sec) * 1000000000L
             + (tick.tv_nsec - start.tv_nsec) < 1200000000L);
    time_t later = time(NULL);
    if (later == (time_t)-1) {
        perror("time");
        return 1;
    }
    printf("difftime=%.1fs\n", difftime(later, now));
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
