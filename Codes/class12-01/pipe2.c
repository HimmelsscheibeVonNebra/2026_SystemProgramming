/* pipe2.c — 셸의 "CMD1 | CMD2 > FILE"을 pipe · fork · dup2 · exec로 만든다.
 * 사용법: ./pipe2 [-k] [-o FILE] CMD1 [ARG...] -- CMD2 [ARG...]
 *   -o FILE  CMD2의 표준 출력을 FILE로 보낸다 (셸의 > FILE)
 *   -k       부모가 쓰기 끝을 닫지 않는다 (EOF가 오지 않는 실험)
 */
#include <fcntl.h>                  /* open, O_WRONLY, O_CREAT, O_TRUNC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>                 /* strcmp, strsignal */
#include <sys/wait.h>               /* waitpid, WIFSIGNALED 등 */
#include <unistd.h>                 /* pipe, fork, dup2, execvp, close */

static void die(const char *what)
{
    perror(what);
    exit(1);
}

static void usage(const char *prog)
{
    fprintf(stderr, "usage: %s [-k] [-o FILE] CMD1 ... -- CMD2 ...\n", prog);
    exit(2);                        /* 사용법 오류는 2 */
}

/* 자식 하나를 기다려 끝난 방식을 stderr에 적고, 셸의 $? 값을 돌려준다. */
static int report(const char *name, pid_t pid)
{
    int st;                         /* 자식의 종료 상태 */

    if (waitpid(pid, &st, 0) == -1)  /* 이 자식이 끝날 때까지 기다린다 */
        die("waitpid");
    if (WIFSIGNALED(st)) {          /* 시그널로 죽었다 (예: SIGPIPE 13) */
        fprintf(stderr, "[pipe2] %s: 시그널 %d (%s)\n", name, WTERMSIG(st),
                strsignal(WTERMSIG(st)));
        return 128 + WTERMSIG(st);  /* 셸 관례: 128 + 시그널 번호 */
    }
    fprintf(stderr, "[pipe2] %s: 종료 코드 %d\n", name, WEXITSTATUS(st));
    return WEXITSTATUS(st);         /* exit나 return으로 끝난 값 */
}

int main(int argc, char *argv[])
{
    int keep = 0, i = 1;            /* keep: -k 여부, i: argv를 훑는 위치 */
    const char *out = NULL;         /* -o FILE의 FILE */

    while (i < argc && argv[i][0] == '-') {     /* 옵션: -k, -o FILE */
        if (strcmp(argv[i], "-k") == 0)
            keep = 1;
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
            out = argv[++i];
        else
            usage(argv[0]);
        i++;
    }
    char **left = &argv[i], **right = NULL;  /* left: CMD1의 인자 배열 */
    for (; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {   /* NULL로 바꿔 둘로 자른다 */
            argv[i] = NULL;
            right = &argv[i + 1];   /* right: CMD2의 인자 배열 */
            break;
        }
    }
    if (right == NULL || left[0] == NULL || right[0] == NULL)  /* -- 가 없거나 한쪽이 비었다 */
        usage(argv[0]);

    int fd[2];                      /* fd[0]: 읽기 끝, fd[1]: 쓰기 끝 */
    if (pipe(fd) == -1)                 /* fork보다 먼저 만든다 */
        die("pipe");

    pid_t p1 = fork();              /* 첫째 자식: CMD1 */
    if (p1 == -1)
        die("fork");
    if (p1 == 0) {                      /* 왼쪽 명령: 표준 출력 → 파이프 */
        dup2(fd[1], STDOUT_FILENO);  /* 쓰기 끝을 1번 자리에 */
        close(fd[0]);               /* 읽기 끝은 쓰지 않는다 */
        close(fd[1]);                   /* 1번이 대신하므로 닫는다 */
        execvp(left[0], left);      /* 성공하면 돌아오지 않는다 */
        perror(left[0]);            /* 여기 왔다면 exec 실패 */
        _exit(127);                 /* 셸 관례: 명령을 못 찾으면 127 */
    }

    pid_t p2 = fork();              /* 둘째 자식: CMD2 */
    if (p2 == -1)
        die("fork");
    if (p2 == 0) {                    /* 오른쪽 명령: 표준 입력 ← 파이프 */
        dup2(fd[0], STDIN_FILENO);  /* 읽기 끝을 0번 자리에 */
        close(fd[0]);               /* 0번이 대신하므로 닫는다 */
        close(fd[1]);               /* 안 닫으면 자기 때문에 EOF가 안 온다 */
        if (out != NULL) {              /* > FILE: 표준 출력을 파일로 */
            int o = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (o == -1) {
                perror(out);
                _exit(1);           /* 파일을 못 열면 명령을 실행하지 않는다 */
            }
            dup2(o, STDOUT_FILENO);  /* 파일을 1번 자리에 */
            close(o);               /* 1번이 대신하므로 닫는다 */
        }
        execvp(right[0], right);    /* 0번은 파이프, 1번은 터미널이나 파일 */
        perror(right[0]);           /* 여기 왔다면 exec 실패 */
        _exit(127);
    }

    close(fd[0]);                       /* 부모는 파이프를 쓰지 않는다 */
    if (!keep)                      /* -k면 다음 줄을 건너뛴다 */
        close(fd[1]);                   /* 안 닫으면 CMD2에 EOF가 안 온다 */
    report(left[0], p1);            /* close 뒤에 기다린다. 바꾸면 안 끝난다 */
    return report(right[0], p2);        /* 마지막 명령의 상태로 끝난다 */
}
