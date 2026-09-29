#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static struct timespec start;

static long long ts_ns(const struct timespec *ts)
{
    return (long long)ts->tv_sec * 1000000000LL + ts->tv_nsec;
}

static void spin_ns(long long ns)
{
    struct timespec begin;
    if (clock_gettime(CLOCK_MONOTONIC, &begin) == -1) {
        perror("clock_gettime");
        exit(1);
    }
    struct timespec now;
    do {
        if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
            perror("clock_gettime");
            exit(1);
        }
    } while (ts_ns(&now) - ts_ns(&begin) < ns);
}

/* 로그 한 줄 = 달력 시각 + 경과 시간 + 메시지.
   쓴 뒤 곧바로 fflush해야 비정상 종료에도 줄이 남는다. */
static int log_line(FILE *out, const char *message)
{
    time_t now = time(NULL);
    if (now == (time_t)-1) {
        perror("time");
        return -1;
    }
    struct tm *tm = localtime(&now);
    if (tm == NULL) {
        perror("localtime");
        return -1;
    }
    char stamp[32];
    if (strftime(stamp, sizeof stamp, "%Y-%m-%d %H:%M:%S %Z", tm) == 0) {
        fprintf(stderr, "strftime: buffer too small\n");
        return -1;
    }
    struct timespec mono;
    if (clock_gettime(CLOCK_MONOTONIC, &mono) == -1) {
        perror("clock_gettime");
        return -1;
    }
    double elapsed = (double)(ts_ns(&mono) - ts_ns(&start)) / 1e9;
    if (fprintf(out, "%s +%07.4fs %s\n", stamp, elapsed, message) < 0)
        return -1;
    if (fflush(out) == EOF)
        return -1;
    return 0;
}

int main(int argc, char **argv)
{
    FILE *out = stderr;
    int demo = 0;

    int i = 1;
    while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0') {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc || out != stderr) {
                fprintf(stderr, "usage: %s [-o FILE] [--demo | MESSAGE...]\n",
                        argv[0]);
                return 2;
            }
            out = fopen(argv[i + 1], "a");
            if (out == NULL) {
                fprintf(stderr, "%s: %s\n", argv[i + 1], strerror(errno));
                return 1;
            }
            i += 2;
        } else if (strcmp(argv[i], "--demo") == 0) {
            demo = 1;
            ++i;
        } else {
            break;
        }
    }
    static char *demo_messages[] = {"start", "read config", "process data",
                                    "write result", "done"};
    char **messages;
    int nmsg = argc - i;
    if (demo) {
        if (nmsg != 0) {
            fprintf(stderr, "usage: %s [-o FILE] [--demo | MESSAGE...]\n", argv[0]);
            if (out != stderr && fclose(out) == EOF)
                perror("fclose");
            return 2;
        }
        messages = demo_messages;
        nmsg = 5;
    } else {
        messages = argv + i;
        if (nmsg == 0) {
            fprintf(stderr, "usage: %s [-o FILE] [--demo | MESSAGE...]\n", argv[0]);
            if (out != stderr && fclose(out) == EOF)
                perror("fclose");
            return 2;
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        perror("clock_gettime");
        return 1;
    }
    int status = 0;
    for (int m = 0; m < nmsg; ++m) {
        if (log_line(out, messages[m]) == -1) {
            fprintf(stderr, "log write failed\n");
            status = 1;
            break;
        }
        if (m + 1 < nmsg)   /* demo의 줄 사이에 약 12ms 간격을 만든다 */
            spin_ns(12000000);
    }
    if (out != stderr) {
        if (fclose(out) == EOF) {
            perror("fclose");
            status = 1;
        }
    } else if (fflush(stderr) == EOF) {
        perror("stderr");
        status = 1;
    }
    return status;
}
