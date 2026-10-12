#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* 파일을 열고 지정한 초 동안 유지하며 FD 표를 관찰하게 한다.
   open은 남은 가장 작은 번호를 돌려주므로 표준 스트림(0·1·2)
   만 열려 있으면 3이 돌아온다. 관찰 후 close로 반환한다.
   실패 경로에서도 연 FD는 닫는 것이 3주차 이후의 반복 원칙이다. */

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: %s FILE [seconds 1-600]\n", argv[0]);
        return 2;
    }
    long seconds = 10;
    if (argc == 3) {
        char *end = NULL;
        seconds = strtol(argv[2], &end, 10);
        if (end == argv[2] || *end != '\0' ||
            seconds < 1 || seconds > 600) {
            fprintf(stderr, "usage: %s FILE [seconds 1-600]\n", argv[0]);
            return 2;
        }
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        return 1;
    }
    printf("pid=%ld fd=%d path=%s\n", (long)getpid(), fd, argv[1]);
    printf("hold_seconds=%ld\n", seconds);
    printf("observe: ls -l /proc/%ld/fd\n", (long)getpid());
    printf("observe: ls /proc/%ld/fd | wc -l\n", (long)getpid());
    if (fflush(stdout) == EOF) {
        perror("stdout");
        close(fd);
        return 1;
    }
    for (long done = 1; done <= seconds; done++) {
        sleep(1);
        printf("hold=%ld of %ld\n", done, seconds);
        if (fflush(stdout) == EOF) {
            perror("stdout");
            close(fd);
            return 1;
        }
    }
    if (close(fd) == -1) {
        perror("close");
        return 1;
    }
    printf("closed fd=%d\n", fd);
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
