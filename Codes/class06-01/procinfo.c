#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* 오늘 배운 실행 문맥을 한 프로그램으로 종합한다.
   정체성(pid·ppid·pgid·sid), 자격 증명(uid·euid·gid·egid),
   작업 디렉터리, 인자, 환경 요약을 같은 규칙으로 출력한다.
   --env를 주면 환경 전체를, --argv를 주면 인자 전체를 나열한다. */
extern char **environ;

static int count_env(void)
{
    int n = 0;
    for (char **e = environ; *e != NULL; e++) {
        n++;
    }
    return n;
}

static void show_env_var(const char *name)
{
    const char *value = getenv(name);
    if (value == NULL) {
        printf("%s=(unset)\n", name);
    } else {
        printf("%s=%s\n", name, value);
    }
}

int main(int argc, char **argv)
{
    int show_env = 0;
    int show_argv = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--env") == 0) {
            show_env = 1;
        } else if (strcmp(argv[i], "--argv") == 0) {
            show_argv = 1;
        } else if (strncmp(argv[i], "--", 2) == 0) {
            fprintf(stderr, "usage: %s [--env] [--argv]"
                            " [args...]\n", argv[0]);
            return 2;
        }
        /* --로 시작하지 않는 인자는 명령행 입력으로 그대로 두고
           --argv로 관찰한다. argv는 검증 대상이지 거부 대상이 아니다. */
    }
    printf("pid=%ld ppid=%ld\n",
           (long)getpid(), (long)getppid());
    printf("pgid=%ld sid=%ld\n",
           (long)getpgrp(), (long)getsid(0));
    const char *tty = ttyname(STDIN_FILENO);
    if (tty != NULL) {
        printf("tty=%s\n", tty);
    } else {
        printf("tty=(none)\n");
    }
    printf("uid=%ld euid=%ld\n",
           (long)getuid(), (long)geteuid());
    printf("gid=%ld egid=%ld\n",
           (long)getgid(), (long)getegid());
    char cwd[4096];
    if (getcwd(cwd, sizeof cwd) == NULL) {
        perror("getcwd");
        return 1;
    }
    printf("cwd=%s\n", cwd);
    printf("argc=%d\n", argc);
    if (show_argv) {
        for (int i = 0; i < argc; i++) {
            printf("argv[%d]=%s\n", i, argv[i]);
        }
    }
    printf("env_count=%d\n", count_env());
    if (show_env) {
        for (char **e = environ; *e != NULL; e++) {
            printf("%s\n", *e);
        }
    }
    show_env_var("HOME");
    show_env_var("PATH");
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
