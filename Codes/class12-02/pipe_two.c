/* pipe_two.c — 파이프 두 개로 한 번 묻고 한 번 답을 받는다. */
#include <stdio.h>
#include <sys/wait.h>               /* waitpid */
#include <unistd.h>                 /* pipe, fork, read, write, close */

int main(void)
{
    int req[2], rep[2];             /* req: 부모 → 자식, rep: 자식 → 부모 */
    char buf[64];                   /* NUL로 끝나지 않는다: 길이 n과 함께 쓴다 */
    ssize_t n;                      /* read의 반환값 */

    if (pipe(req) == -1 || pipe(rep) == -1) {   /* 방향마다 파이프 하나 */
        perror("pipe");
        return 1;
    }
    printf("req = %d,%d  rep = %d,%d\n", req[0], req[1], rep[0], rep[1]);
    fflush(stdout);                 /* fork 전에 stdio 버퍼를 비운다 (4주차) */

    pid_t pid = fork();             /* 끝점 넷이 복사되어 여덟 개가 된다 */
    if (pid == -1) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {                 /* 자식: req에서 읽고 rep에 쓴다 */
        close(req[1]);              /* 자식은 req에 쓰지 않는다 */
        close(rep[0]);              /* 자식은 rep에서 읽지 않는다 */
        n = read(req[0], buf, sizeof buf);      /* 질문이 올 때까지 잠든다 */
        printf("[자식] 질문 %zd바이트: %.*s", n, (int)n, buf);
        fflush(stdout);
        if (write(rep[1], "pong\n", 5) == -1)   /* 반대 방향 파이프로 답한다 */
            perror("write");
        _exit(0);                   /* 남은 끝점 둘은 종료하면서 닫힌다 */
    }

    close(req[0]);                  /* 부모는 req에서 읽지 않는다 */
    close(rep[1]);                  /* 부모는 rep에 쓰지 않는다 */
    if (write(req[1], "ping\n", 5) == -1)       /* 먼저 묻는다 */
        perror("write");
    n = read(rep[0], buf, sizeof buf);          /* 그다음에 답을 기다린다 */
    printf("[부모] 답 %zd바이트: %.*s", n, (int)n, buf);
    close(req[1]);
    close(rep[0]);
    waitpid(pid, NULL, 0);          /* 자식을 거둔다 (10주차) */
    return 0;
}
