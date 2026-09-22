#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s DIRECTORY\n", argv[0]);
        return 2;
    }

    /* 부모 디렉토리는 이미 있어야 한다. 실제 권한에는 umask가 적용된다. */
    if (mkdir(argv[1], 0755) == -1) {
        perror(argv[1]);
        return 1;
    }

    if (printf("created: %s\n", argv[1]) < 0 || fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
