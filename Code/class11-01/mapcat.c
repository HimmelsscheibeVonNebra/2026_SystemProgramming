/* mapcat.c — 파일을 메모리에 매핑해서 표준 출력으로 내보낸다.
 * 사용법: ./mapcat [-p] FILE
 *   -p  출력 뒤 PID를 알리고 Enter를 기다린다 (/proc/PID/maps 관찰용)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

static int write_all(int fd, const char *p, size_t n)
{
    while (n > 0) {
        ssize_t k = write(fd, p, n);
        if (k < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += k;
        n -= (size_t)k;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int pause_flag = (argc == 3 && strcmp(argv[1], "-p") == 0);
    if (argc != 2 + pause_flag) {
        fprintf(stderr, "usage: %s [-p] FILE\n", argv[0]);
        return 2;
    }
    const char *path = argv[1 + pause_flag];

    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror(path); return 1; }

    struct stat st;
    if (fstat(fd, &st) < 0) { perror("fstat"); return 1; }
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "%s: not a regular file\n", path);
        return 1;
    }
    if (st.st_size == 0) {          /* length 0 은 mmap 이 EINVAL */
        fprintf(stderr, "%s: empty file, nothing to map\n", path);
        close(fd);
        return 0;
    }

    size_t len = (size_t)st.st_size;
    char *p = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED) { perror("mmap"); return 1; }
    close(fd);                      /* 매핑은 fd 와 별개로 남는다 */

    if (write_all(STDOUT_FILENO, p, len) < 0) { perror("write"); return 1; }

    if (pause_flag) {
        fprintf(stderr, "[mapcat] pid=%d mapped %zu bytes at %p — Enter to unmap\n",
                (int)getpid(), len, (void *)p);
        getchar();
    }
    if (munmap(p, len) < 0) { perror("munmap"); return 1; }
    return 0;
}
