/* pipeline.c — 셸의 CMD1 | CMD2 | ... | CMDN 을 반복문 하나로 만든다.
 * 단계가 몇 개든 부모가 들고 다니는 것은 "앞 파이프의 읽기 끝" 하나다.
 * 사용법: ./pipeline [-v] [-k] CMD [ARG...] -- CMD [ARG...] [-- ...]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>               /* waitpid, WIFEXITED, WTERMSIG */
#include <unistd.h>                 /* pipe, fork, dup2, execvp, close */

#define MAXCMD 16                   /* 받을 수 있는 단계 수 */

static void die(const char *what)   /* 실패한 호출 이름과 errno 메시지를 내고 끝낸다 */
{
    perror(what);
    exit(1);
}

/* 자식: 0번과 1번을 바꿔 끼우고 명령으로 바뀐다. 돌아오지 않는다. */
static void run_stage(char **cmd, int in, int out, int unused)
{
    if (unused != -1)               /* 새 파이프의 읽기 끝: 다음 단계의 것이다 */
        close(unused);
    if (in != -1) {                 /* 첫 단계가 아니면 입력을 앞 파이프로 */
        if (dup2(in, 0) == -1)
            die("dup2");
        close(in);                  /* 0번이 가리키니 원래 번호는 닫는다 */
    }
    if (out != -1) {                /* 마지막 단계가 아니면 출력을 새 파이프로 */
        if (dup2(out, 1) == -1)
            die("dup2");
        close(out);
    }
    execvp(cmd[0], cmd);            /* 성공하면 돌아오지 않는다 */
    perror(cmd[0]);                 /* 여기 왔다면 exec 실패 */
    _exit(127);                     /* 셸의 "명령을 찾을 수 없음" 코드 (10주차) */
}

int main(int argc, char *argv[])
{
    char **cmd[MAXCMD];             /* cmd[i]: i번째 단계의 인자 배열 */
    pid_t pid[MAXCMD];
    int n = 0;                      /* 단계 수 */
    int verbose = 0, keep = 0;      /* -v: 장부 출력, -k: 쓰기 끝을 안 닫는다 */
    int i = 1;

    for (; i < argc && argv[i][0] == '-' && strcmp(argv[i], "--") != 0; i++) {
        if (strcmp(argv[i], "-v") == 0)
            verbose = 1;
        else if (strcmp(argv[i], "-k") == 0)
            keep = 1;
    }
    for (int start = i; i <= argc; i++) {       /* "--"를 NULL로 바꿔 자른다 */
        if (i < argc && strcmp(argv[i], "--") != 0)
            continue;
        if (i == start || n == MAXCMD) {        /* 빈 단계이거나 너무 많다 */
            n = 0;
            break;
        }
        argv[i] = NULL;             /* execvp가 요구하는 끝 표시 (argv[argc]도 NULL) */
        cmd[n++] = &argv[start];
        start = i + 1;
    }
    if (n < 2) {
        fprintf(stderr, "사용법: %s [-v] [-k] CMD [ARG...] -- CMD [ARG...] [-- ...]\n",
                argv[0]);
        return 2;
    }

    int prev = -1;                  /* 앞 파이프의 읽기 끝. 첫 단계는 없다 */
    for (i = 0; i < n; i++) {
        int fd[2] = { -1, -1 };     /* 이 단계와 다음 단계를 잇는 파이프 */

        if (i < n - 1 && pipe(fd) == -1)        /* 마지막 단계 뒤에는 파이프가 없다 */
            die("pipe");
        pid[i] = fork();
        if (pid[i] == -1)
            die("fork");
        if (pid[i] == 0)
            run_stage(cmd[i], prev, fd[1], fd[0]);

        if (verbose)
            fprintf(stderr,
                    "[부모] %d단계 %-6s 입력 %2d  출력 %2d  (새 파이프 %d,%d)\n",
                    i + 1, cmd[i][0], prev, fd[1], fd[0], fd[1]);
        if (prev != -1)
            close(prev);            /* 방금 자식에게 넘겨준 읽기 끝 */
        if (fd[1] != -1 && !keep)   /* -k는 다음 줄만 건너뛴다 */
            close(fd[1]);           /* 부모가 쥐고 있으면 다음 단계에 EOF가 안 온다 */
        prev = fd[0];               /* 다음 바퀴의 입력이 된다 */
    }

    int last = 0;                   /* 마지막 단계의 종료 상태 */
    for (i = 0; i < n; i++) {       /* 모두 띄운 뒤에 기다린다: 동시에 돈다 */
        int st;
        if (waitpid(pid[i], &st, 0) == -1)
            die("waitpid");
        if (WIFEXITED(st))
            fprintf(stderr, "[%d] %s: 종료 코드 %d\n", i + 1, cmd[i][0],
                    WEXITSTATUS(st));
        else
            fprintf(stderr, "[%d] %s: 시그널 %d\n", i + 1, cmd[i][0], WTERMSIG(st));
        last = WIFEXITED(st) ? WEXITSTATUS(st) : 128 + WTERMSIG(st);
    }
    return last;                    /* 셸처럼 마지막 명령의 상태를 돌려준다 */
}
