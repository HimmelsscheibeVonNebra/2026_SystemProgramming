/* pipe_basic.c — 한 프로세스 안에서 파이프에 쓰고 다시 읽는다. */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    int fd[2];                  /* fd[0]: 읽기 끝, fd[1]: 쓰기 끝 */

    if (pipe(fd) == -1) {       /* 성공 0, 실패 -1 */
        perror("pipe");
        return 1;
    }
    printf("fd[0] = %d, fd[1] = %d\n", fd[0], fd[1]);

    const char *msg = "hello pipe\n";
    if (write(fd[1], msg, strlen(msg)) == -1) {   /* 커널 버퍼에 들어간다 */
        perror("write");
        return 1;
    }
    close(fd[1]);               /* 쓰기 끝이 모두 닫혀야 EOF가 생긴다 */

    char buf[64];
    ssize_t n;
    while ((n = read(fd[0], buf, sizeof buf)) > 0)    /* 0이면 EOF */
        fwrite(buf, 1, (size_t)n, stdout);
    close(fd[0]);
    return 0;
}
