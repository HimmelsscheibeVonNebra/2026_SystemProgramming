#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

/* 오늘의 정체성 조회 네 개는 모두 인자 없이 pid_t를 돌려주고
   실패 경로를 정의하지 않는다. pid_t의 크기는 시스템마다 다를
   수 있으므로 출력 전에 (long)으로 변환한다. */
int main(int argc, char **argv)
{
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 2;
    }
    printf("pid=%ld\n", (long)getpid());
    printf("ppid=%ld\n", (long)getppid());
    printf("pgid=%ld\n", (long)getpgrp());
    printf("sid=%ld\n", (long)getsid(0));
    /* 표준 입력이 터미널이 아니면(입력 리다이렉션·크론 등)
       ttyname이 NULL을 돌려준다. 이것도 관찰 결과다. */
    const char *tty = ttyname(STDIN_FILENO);
    if (tty == NULL) {
        printf("tty=(none)\n");
    } else {
        printf("tty=%s\n", tty);
    }
    printf("compare: ps -o pid,ppid,pgid,tty,stat,command -p %ld\n",
           (long)getpid());
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
