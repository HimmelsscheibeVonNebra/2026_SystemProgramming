#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

/* 요구 페이징(demand paging)을 숫자로 확인한다. 큰 블록을 malloc한
   직후에는 VmRSS가 거의 늘지 않고, 페이지마다 한 바이트씩 써서
   처음 건드릴 때마다 minor page fault가 일어나며 RSS가 늘어난다.
   minflt는 getrusage(표준 API), VmRSS는 /proc/self/status(Linux)
   에서 읽는다. /proc가 없으면 rss_kib=unavailable로 표시한다. */

static long rss_kib(void)
{
    FILE *fp = fopen("/proc/self/status", "r");
    if (fp == NULL)
        return -1;
    char line[256];
    long kib = -1;
    while (fgets(line, sizeof line, fp) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            kib = strtol(line + 6, NULL, 10);
            break;
        }
    }
    fclose(fp);
    return kib;
}

static void report(const char *stage)
{
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) == -1) {
        perror("getrusage");
        exit(1);
    }
    long kib = rss_kib();
    if (kib < 0)
        printf("stage=%-12s rss_kib=unavailable minflt=%ld majflt=%ld\n",
               stage, ru.ru_minflt, ru.ru_majflt);
    else
        printf("stage=%-12s rss_kib=%-7ld minflt=%ld majflt=%ld\n",
               stage, kib, ru.ru_minflt, ru.ru_majflt);
}

int main(int argc, char **argv)
{
    long mib = 64;
    if (argc > 2) {
        fprintf(stderr, "usage: %s [MiB 1-1024]\n", argv[0]);
        return 2;
    }
    if (argc == 2) {
        char *end = NULL;
        mib = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || mib < 1 || mib > 1024) {
            fprintf(stderr, "usage: %s [MiB 1-1024]\n", argv[0]);
            return 2;
        }
    }
    long page = sysconf(_SC_PAGESIZE);
    if (page <= 0) {
        perror("sysconf");
        return 1;
    }
    size_t size = (size_t)mib * 1024 * 1024;
    size_t pages = size / (size_t)page;
    printf("pid=%ld size_mib=%ld page=%ld pages=%zu\n",
           (long)getpid(), mib, page, pages);
    report("start");
    /* volatile: 해제 직전의 쓰기를 컴파일러가 지우지 못하게 한다 */
    volatile char *buf = malloc(size);
    if (buf == NULL) {
        perror("malloc");
        return 1;
    }
    report("after_malloc");
    for (size_t i = 0; i < pages / 2; i++)
        buf[i * (size_t)page] = 1;
    report("touch_half");
    for (size_t i = pages / 2; i < pages; i++)
        buf[i * (size_t)page] = 1;
    report("touch_all");
    for (size_t i = 0; i < pages; i++)
        buf[i * (size_t)page] = 2;
    report("touch_again");
    free((void *)buf);
    report("after_free");
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
