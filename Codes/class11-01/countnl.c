/* countnl.c — 같은 파일의 개행 수를 read 방식과 mmap 방식으로 세고
 *             경과 시간과 page fault 수를 함께 보여 준다.
 * 사용법:
 *   ./countnl read FILE
 *   ./countnl mmap FILE [seq]    seq: madvise(MADV_SEQUENTIAL)
 *   ./countnl drop FILE          이 파일의 페이지 캐시를 비운다 (cold 상태)
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/resource.h>

#define BUFSZ (64 * 1024)

static void die(const char *msg) { perror(msg); exit(1); }

static long count_buf(const char *p, size_t n)
{
    long c = 0;
    const char *end = p + n;
    while ((p = memchr(p, '\n', (size_t)(end - p))) != NULL) {
        c++;
        p++;
    }
    return c;
}

static long count_read(int fd)
{
    static char buf[BUFSZ];
    long c = 0;
    ssize_t n;
    while ((n = read(fd, buf, sizeof buf)) > 0)
        c += count_buf(buf, (size_t)n);
    if (n < 0) die("read");
    return c;
}

static long count_mmap(int fd, size_t len, int advice)
{
    if (len == 0) return 0;
    char *p = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED) die("mmap");
    if (advice != MADV_NORMAL && madvise(p, len, advice) < 0) die("madvise");
    long c = count_buf(p, len);
    munmap(p, len);
    return c;
}

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s read|mmap|drop FILE [seq]\n", argv[0]);
        return 2;
    }
    const char *mode = argv[1];
    int fd = open(argv[2], O_RDONLY);
    if (fd < 0) die(argv[2]);
    struct stat st;
    if (fstat(fd, &st) < 0) die("fstat");

    if (strcmp(mode, "drop") == 0) {
#ifdef POSIX_FADV_DONTNEED
        int e = posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
        if (e != 0) { fprintf(stderr, "posix_fadvise: %s\n", strerror(e)); return 1; }
        printf("dropped page cache of %s\n", argv[2]);
        return 0;
#else
        fprintf(stderr, "drop: posix_fadvise is not available on this OS\n");
        return 1;
#endif
    }

    int seq = (argc == 4 && strcmp(argv[3], "seq") == 0);
    int advice = seq ? MADV_SEQUENTIAL : MADV_NORMAL;
    const char *tag = seq ? "+seq" : "";
    struct rusage r0, r1;
    getrusage(RUSAGE_SELF, &r0);
    double t0 = now_ms();

    long lines;
    if (strcmp(mode, "read") == 0)
        lines = count_read(fd);
    else if (strcmp(mode, "mmap") == 0)
        lines = count_mmap(fd, (size_t)st.st_size, advice);
    else { fprintf(stderr, "unknown mode %s\n", mode); return 2; }

    double t1 = now_ms();
    getrusage(RUSAGE_SELF, &r1);
    printf("%-4s%-4s lines=%ld  %8.1f ms  minflt=%ld  majflt=%ld\n",
           mode, tag, lines, t1 - t0,
           r1.ru_minflt - r0.ru_minflt, r1.ru_majflt - r0.ru_majflt);
    close(fd);
    return 0;
}
