/* pipe_basic.c — 한 프로세스 안에서 파이프에 쓰고 다시 읽는다. */
#include <stdio.h>                  /* printf, perror, fwrite */
#include <string.h>                 /* strlen */
#include <unistd.h>                 /* pipe, read, write, close */

int main(void)
{
    int fd[2];                  /* fd[0]: 읽기 끝, fd[1]: 쓰기 끝 */

    if (pipe(fd) == -1) {       /* 성공 0, 실패 -1 */
        perror("pipe");             /* 예: fd가 모자라면 EMFILE */
        return 1;
    }
    printf("fd[0] = %d, fd[1] = %d\n", fd[0], fd[1]);  /* 보통 3과 4: 0·1·2 다음의 빈 번호 */

    const char *msg = "hello pipe\n";  /* 줄바꿈까지 11바이트 */
    if (write(fd[1], msg, strlen(msg)) == -1) {   /* 커널 버퍼에 들어간다 */
        perror("write");
        return 1;
    }
    close(fd[1]);               /* 쓰기 끝이 모두 닫혀야 EOF가 생긴다 */

    char buf[64];                   /* read가 채울 자리. NUL로 끝나지 않는다 */
    ssize_t n;                      /* read의 반환값: 받은 바이트 수, 0, -1 */
    while ((n = read(fd[0], buf, sizeof buf)) > 0)    /* 0이면 EOF */
        fwrite(buf, 1, (size_t)n, stdout);  /* 받은 n바이트만 그대로 낸다 */
    close(fd[0]);                   /* 양 끝이 모두 닫히면 파이프도 사라진다 */
    return 0;
}
