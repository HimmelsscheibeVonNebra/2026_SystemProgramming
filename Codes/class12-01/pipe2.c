/* pipe2.c — 셸의 "CMD1 | CMD2 > FILE"을 pipe · fork · dup2 · exec로 만든다.
 * 사용법: ./pipe2 [-k] [-o FILE] CMD1 [ARG...] -- CMD2 [ARG...]
 *   -o FILE  CMD2의 표준 출력을 FILE로 보낸다 (셸의 > FILE)
 *   -k       부모가 쓰기 끝을 닫지 않는다 (EOF가 오지 않는 실험)
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void die(const char *what)
{
    perror(what);
    exit(1);
}

static void usage(const char *prog)
{
    fprintf(stderr, "usage: %s [-k] [-o FILE] CMD1 ... -- CMD2 ...\n", prog);
    exit(2);
}

/* 자식 하나를 기다려 끝난 방식을 stderr에 적고, 셸의 $? 값을 돌려준다. */
static int report(const char *name, pid_t pid)
{
    int st;

    if (waitpid(pid, &st, 0) == -1)
        die("waitpid");
    if (WIFSIGNALED(st)) {
        fprintf(stderr, "[pipe2] %s: 시그널 %d (%s)\n", name, WTERMSIG(st),
                strsignal(WTERMSIG(st)));
        return 128 + WTERMSIG(st);
    }
    fprintf(stderr, "[pipe2] %s: 종료 코드 %d\n", name, WEXITSTATUS(st));
    return WEXITSTATUS(st);
}

int main(int argc, char *argv[])
{
    int keep = 0, i = 1;
    const char *out = NULL;

    while (i < argc && argv[i][0] == '-') {     /* 옵션: -k, -o FILE */
        if (strcmp(argv[i], "-k") == 0)
            keep = 1;
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
            out = argv[++i];
        else
            usage(argv[0]);
        i++;
    }
    char **left = &argv[i], **right = NULL;
    for (; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {   /* NULL로 바꿔 둘로 자른다 */
            argv[i] = NULL;
            right = &argv[i + 1];
            break;
        }
    }
    if (right == NULL || left[0] == NULL || right[0] == NULL)
        usage(argv[0]);

    int fd[2];
    if (pipe(fd) == -1)                 /* fork보다 먼저 만든다 */
        die("pipe");

    pid_t p1 = fork();
    if (p1 == -1)
        die("fork");
    if (p1 == 0) {                      /* 왼쪽 명령: 표준 출력 → 파이프 */
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);                   /* 1번이 대신하므로 닫는다 */
        execvp(left[0], left);
        perror(left[0]);
        _exit(127);
    }

    pid_t p2 = fork();
    if (p2 == -1)
        die("fork");
    if (p2 == 0) {                    /* 오른쪽 명령: 표준 입력 ← 파이프 */
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        if (out != NULL) {              /* > FILE: 표준 출력을 파일로 */
            int o = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (o == -1) {
                perror(out);
                _exit(1);
            }
            dup2(o, STDOUT_FILENO);
            close(o);
        }
        execvp(right[0], right);
        perror(right[0]);
        _exit(127);
    }

    close(fd[0]);                       /* 부모는 파이프를 쓰지 않는다 */
    if (!keep)
        close(fd[1]);                   /* 안 닫으면 CMD2에 EOF가 안 온다 */
    report(left[0], p1);
    return report(right[0], p2);        /* 마지막 명령의 상태로 끝난다 */
}
