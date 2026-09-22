#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s DIRECTORY\n", argv[0]);
        return 2;
    }

    /* 빈 디렉토리만 삭제한다. 내부 파일을 재귀적으로 지우지 않는다. */
    if (rmdir(argv[1]) == -1) {
        perror(argv[1]);
        return 1;
    }

    if (printf("removed: %s\n", argv[1]) < 0 || fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
