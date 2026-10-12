#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* 종료와 회수 사이의 상태를 일부러 늘려 관찰한다.
   zombie: 자식은 곧바로 종료 상태 7로 끝나고, 부모는 [seconds]초
           동안 회수하지 않는다. 그동안 ps에 Z <defunct>가 보인다.
           그 뒤 waitpid로 회수하면 항목이 사라진다.
   orphan: 부모가 먼저 끝나고, 자식은 2초 뒤 자기 PPID를 다시 본다.
           새 부모는 PID 1이거나 가장 가까운 subreaper다.
   fork·waitpid의 정식 계약은 10주차에서 다룬다. 여기서는 관찰 도구다. */

static int flush_or_die(void)
{
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return -1;
    }
    return 0;
}

static int zombie(long seconds)
{
    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return 1;
    }
    if (child == 0)
        _exit(7);               /* 자식: 즉시 종료 상태 7 */
    printf("parent=%ld child=%ld (child exited, not reaped yet)\n",
           (long)getpid(), (long)child);
    printf("observe: ps -o pid,ppid,stat,command -p %ld\n", (long)child);
    if (flush_or_die() == -1)
        return 1;
    unsigned int left = (unsigned int)seconds;
    while (left != 0)
        left = sleep(left);
    int status;
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }
    if (WIFEXITED(status))
        printf("reaped child=%ld exit_status=%d\n",
               (long)child, WEXITSTATUS(status));
    return flush_or_die() == -1 ? 1 : 0;
}

static int orphan(void)
{
    if (flush_or_die() == -1)
        return 1;
    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return 1;
    }
    if (child > 0) {
        printf("parent=%ld exits now, child=%ld\n",
               (long)getpid(), (long)child);
        return flush_or_die() == -1 ? 1 : 0;
    }
    printf("child=%ld ppid_before=%ld\n", (long)getpid(), (long)getppid());
    if (flush_or_die() == -1)
        _exit(1);
    sleep(2);
    printf("child=%ld ppid_after=%ld (adopted)\n",
           (long)getpid(), (long)getppid());
    _exit(flush_or_die() == -1 ? 1 : 0);
}

int main(int argc, char **argv)
{
    if (argc >= 2 && argc <= 3 && strcmp(argv[1], "zombie") == 0) {
        long seconds = 10;
        if (argc == 3) {
            char *end = NULL;
            seconds = strtol(argv[2], &end, 10);
            if (end == argv[2] || *end != '\0' || seconds < 0 || seconds > 600)
                seconds = -1;
        }
        if (seconds >= 0)
            return zombie(seconds);
    } else if (argc == 2 && strcmp(argv[1], "orphan") == 0) {
        return orphan();
    }
    fprintf(stderr, "usage: %s zombie [seconds 0-600] | orphan\n", argv[0]);
    return 2;
}
