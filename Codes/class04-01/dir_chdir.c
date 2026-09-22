#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s DIRECTORY\n", argv[0]);
        return 2;
    }

    /* 수업용 버퍼다. 경로가 더 길면 getcwd의 실패를 처리한다. */
    char cwd[4096];
    if (getcwd(cwd, sizeof cwd) == NULL) {
        perror("getcwd before");
        return 1;
    }
    if (printf("before: %s\n", cwd) < 0 || fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }

    /* 이 프로세스의 위치만 바뀐다. 실행한 부모 셸의 위치는 그대로다. */
    if (chdir(argv[1]) == -1) {
        perror(argv[1]);
        return 1;
    }
    if (getcwd(cwd, sizeof cwd) == NULL) {
        perror("getcwd after");
        return 1;
    }
    if (printf("after: %s\n", cwd) < 0 || fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
