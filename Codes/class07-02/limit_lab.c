#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

/* 자원 한계를 낮춘 뒤 일부러 넘겨서 "넘으면 무엇이 일어나는가"를 본다.
   nofile N : soft NOFILE을 N으로 낮추고 /dev/null을 실패할 때까지 연다.
              EMFILE로 실패하며 열린 개수는 N - 3(표준 스트림)이다.
   as MiB   : soft AS를 MiB로 낮추고 그 두 배를 malloc한다. NULL과
              ENOMEM이 돌아온다(Linux. 다른 시스템은 결과가 다를 수 있다).
   cpu S    : soft CPU를 S초로 낮추고 계속 계산한다. 한계를 넘으면
              커널이 SIGXCPU를 보내 기본 동작으로 종료된다.
   한계는 이 프로세스(와 이후의 자식)에만 적용되고 셸에는 영향이 없다. */

/* soft 한계를 value로 낮추고 실제로 적용된 값을 돌려준다(실패 시 -1) */
static long long lower_soft(int resource, rlim_t value)
{
    struct rlimit lim;
    if (getrlimit(resource, &lim) == -1) {
        perror("getrlimit");
        return -1;
    }
    if (lim.rlim_max != RLIM_INFINITY && value > lim.rlim_max)
        value = lim.rlim_max;   /* soft는 hard를 넘을 수 없다 */
    lim.rlim_cur = value;
    if (setrlimit(resource, &lim) == -1) {
        perror("setrlimit");
        return -1;
    }
    return (long long)value;
}

static int nofile(long n)
{
    long long soft = lower_soft(RLIMIT_NOFILE, (rlim_t)n);
    if (soft == -1)
        return 1;
    int fds[1024];
    int count = 0;
    while (count < 1024) {
        int fd = open("/dev/null", O_RDONLY);
        if (fd == -1) {
            printf("open failed after %d opens: errno=%d (%s)\n",
                   count, errno, strerror(errno));
            break;
        }
        fds[count++] = fd;
    }
    printf("last_fd=%d soft_limit=%lld\n", count ? fds[count - 1] : -1, soft);
    for (int i = 0; i < count; i++)
        close(fds[i]);
    printf("closed=%d\n", count);
    return 0;
}

static int as_limit(long mib)
{
    if (lower_soft(RLIMIT_AS, (rlim_t)mib * 1024 * 1024) == -1)
        return 1;
    size_t want = (size_t)mib * 2 * 1024 * 1024;
    errno = 0;
    void *p = malloc(want);
    if (p == NULL) {
        printf("malloc(%ld MiB) = NULL errno=%d (%s)\n",
               mib * 2, errno, strerror(errno));
    } else {
        printf("malloc(%ld MiB) succeeded (limit not enforced here)\n",
               mib * 2);
        free(p);
    }
    return 0;
}

static int cpu_limit(long seconds)
{
    if (lower_soft(RLIMIT_CPU, (rlim_t)seconds) == -1)
        return 1;
    printf("pid=%ld spinning past %ld s of CPU...\n",
           (long)getpid(), seconds);
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    volatile unsigned long spin = 0;
    for (;;)
        spin++;
}

int main(int argc, char **argv)
{
    if (argc == 3) {
        char *end = NULL;
        long v = strtol(argv[2], &end, 10);
        int valid = end != argv[2] && *end == '\0';
        if (valid && strcmp(argv[1], "nofile") == 0 && v >= 4 && v <= 1024)
            return nofile(v);
        if (valid && strcmp(argv[1], "as") == 0 && v >= 16 && v <= 4096)
            return as_limit(v);
        if (valid && strcmp(argv[1], "cpu") == 0 && v >= 1 && v <= 10)
            return cpu_limit(v);
    }
    fprintf(stderr, "usage: %s nofile N(4-1024) | as MiB(16-4096)"
            " | cpu S(1-10)\n", argv[0]);
    return 2;
}
