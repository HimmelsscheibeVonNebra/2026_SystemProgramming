#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* 자식이 어떻게 끝났는지 부모가 종료 상태(status)로 읽는다.
   exit : 종료 코드 3으로 정상 종료      abort: abort()로 SIGABRT
   segv : 널 포인터에 써서 SIGSEGV        term : raise(SIGTERM)
   wait : 외부에서 kill로 보낸 시그널을 기다린다(STOP·CONT도 보고).
   fork·waitpid의 정식 계약은 10주차에서 다룬다. 여기서는 관찰 도구다. */

static void child_action(const char *mode)
{
    if (strcmp(mode, "exit") == 0)
        _exit(3);
    if (strcmp(mode, "abort") == 0)
        abort();
    if (strcmp(mode, "segv") == 0) {
        volatile int *p = NULL;
        *p = 1;                 /* 커널이 SIGSEGV를 보낸다 */
    }
    if (strcmp(mode, "term") == 0)
        raise(SIGTERM);
    if (strcmp(mode, "wait") == 0) {
        printf("child=%ld waiting: kill -SIGNAL %ld\n",
               (long)getpid(), (long)getpid());
        fflush(stdout);
        for (;;)
            pause();
    }
    _exit(0);
}

static int report(pid_t child, int status)
{
    if (WIFEXITED(status)) {
        printf("child=%ld exited, status=%d\n",
               (long)child, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("child=%ld killed by signal %d (%s)%s\n", (long)child,
               sig, strsignal(sig), WCOREDUMP(status) ? ", core dumped" : "");
    } else if (WIFSTOPPED(status)) {
        int sig = WSTOPSIG(status);
        printf("child=%ld stopped by signal %d (%s)\n",
               (long)child, sig, strsignal(sig));
    } else if (WIFCONTINUED(status)) {
        printf("child=%ld continued\n", (long)child);
    }
    printf("  raw status=0x%04x\n", (unsigned)status);
    fflush(stdout);
    return WIFEXITED(status) || WIFSIGNALED(status);
}

int main(int argc, char **argv)
{
    const char *modes[] = {"exit", "abort", "segv", "term", "wait"};
    int ok = 0;
    for (size_t i = 0; argc == 2 && i < sizeof modes / sizeof modes[0]; i++)
        if (strcmp(argv[1], modes[i]) == 0)
            ok = 1;
    if (!ok) {
        fprintf(stderr, "usage: %s exit|abort|segv|term|wait\n", argv[0]);
        return 2;
    }
    fflush(stdout);
    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return 1;
    }
    if (child == 0)
        child_action(argv[1]);
    for (;;) {
        int status;
        if (waitpid(child, &status, WUNTRACED | WCONTINUED) == -1) {
            perror("waitpid");
            return 1;
        }
        if (report(child, status))
            return 0;
    }
}
