/* mapmode.c — 매핑 플래그에 따라 수정이 어디까지 보이는지 확인한다.
 * 사용법:
 *   ./mapmode private FILE        MAP_PRIVATE 로 첫 줄을 대문자로 바꾼다
 *   ./mapmode shared  FILE        MAP_SHARED  로 첫 줄을 대문자로 바꾼다
 *   ./mapmode anon                익명 공유 매핑 vs 전역 변수 (fork 뒤)
 *   ./mapmode race NPROC LOOPS    여러 자식이 공유 카운터를 동시에 증가
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

static void die(const char *msg) { perror(msg); exit(1); }

/* 파일 매핑(map_file)과 익명 공유 매핑(shared_int) — 두 가지 매핑 함수 */
static char *map_file(const char *path, int flags, size_t *len, int *fdp)
{
    int fd = open(path, O_RDWR);
    if (fd < 0) die(path);
    struct stat st;
    if (fstat(fd, &st) < 0) die("fstat");
    if (st.st_size == 0) { fprintf(stderr, "%s: empty\n", path); exit(1); }
    *len = (size_t)st.st_size;
    char *p = mmap(NULL, *len, PROT_READ | PROT_WRITE, flags, fd, 0);
    if (p == MAP_FAILED) die("mmap");
    *fdp = fd;
    return p;
}

static int *shared_int(void)
{
    int *p = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) die("mmap");
    return p;                                          /* 0 으로 초기화됨 */
}

static void print_line(const char *tag, const char *p, size_t len)
{
    size_t n = 0;
    while (n < len && p[n] != '\n') n++;
    printf("%-14s %.*s\n", tag, (int)n, p);
}

static int file_mode(const char *path, int shared)
{
    size_t len; int fd;
    char *p = map_file(path, shared ? MAP_SHARED : MAP_PRIVATE, &len, &fd);

    for (size_t i = 0; i < len && p[i] != '\n'; i++)   /* 첫 줄 대문자 */
        p[i] = (char)toupper((unsigned char)p[i]);
    print_line("mapping:", p, len);

    if (shared && msync(p, len, MS_SYNC) < 0) die("msync");
    if (munmap(p, len) < 0) die("munmap");

    char buf[256];                                     /* 파일을 다시 읽는다 */
    ssize_t n = pread(fd, buf, sizeof buf, 0);
    if (n < 0) die("pread");
    print_line("file (pread):", buf, (size_t)n);
    close(fd);
    return 0;
}

static int global_value = 0;                           /* 일반 전역 변수 */

static int anon_mode(void)
{
    int *shared = shared_int();
    pid_t pid = fork();
    if (pid < 0) die("fork");
    if (pid == 0) {                                    /* 자식 */
        global_value = 42;
        *shared = 42;
        printf("child : global=%d shared=%d\n", global_value, *shared);
        fflush(stdout);
        _exit(0);
    }
    if (waitpid(pid, NULL, 0) < 0) die("waitpid");
    printf("parent: global=%d shared=%d\n", global_value, *shared);
    munmap(shared, sizeof(int));
    return 0;
}

static int race_mode(int nproc, long loops)
{
    volatile int *counter = shared_int();
    for (int i = 0; i < nproc; i++) {
        pid_t pid = fork();
        if (pid < 0) die("fork");
        if (pid == 0) {
            for (long j = 0; j < loops; j++)
                (*counter)++;                          /* 읽기-더하기-쓰기 */
            _exit(0);
        }
    }
    while (wait(NULL) > 0)
        ;
    long expected = (long)nproc * loops;
    printf("expected %ld  actual %d  lost %ld\n",
           expected, *counter, expected - *counter);
    munmap((void *)counter, sizeof(int));
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc == 3 && strcmp(argv[1], "private") == 0) return file_mode(argv[2], 0);
    if (argc == 3 && strcmp(argv[1], "shared") == 0)  return file_mode(argv[2], 1);
    if (argc == 2 && strcmp(argv[1], "anon") == 0)    return anon_mode();
    if (argc == 4 && strcmp(argv[1], "race") == 0)
        return race_mode(atoi(argv[2]), atol(argv[3]));
    fprintf(stderr, "usage: %s private|shared FILE | anon | race NPROC LOOPS\n", argv[0]);
    return 2;
}
