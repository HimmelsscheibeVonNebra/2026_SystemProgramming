#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* SIGCHLD와 좀비 수거입니다(10주차 fork·waitpid의 복선).
   부모는 N명의 자식을 만들고 자식들은 각자 0.2-1.0초를 잠들다가
   서로 다른 코드로 끝납니다. 자식이 끝나면 커널이 부모에게
   SIGCHLD를 보내고, 핸들러는 플래그만 세웁니다. main 루프가
   waitpid(-1, WNOHANG)로 좀비를 수거하며 보고합니다. 좀비는
   "끝났는데 아직 수거되지 않은" 자식이라 잠깐 생겼다가 없어집니다.
   다 수거하면 waitpid가 ECHILD로 "더 이상 자식이 없다"를 알려 줍니다. */

static volatile sig_atomic_t chlds = 0;

static void on_chld(int sig)
{
    static const char msg[] = "  [handler] SIGCHLD\n";
    (void)sig;
    ssize_t r = write(STDOUT_FILENO, msg, sizeof msg - 1);
    (void)r;
    chlds++;                    /* 수거는 main 루프가 한다 */
}

static void child_sleep(int i)
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = (long)(200 + 100 * i) * 1000 * 1000;   /* 0.2-1.0초 */
    nanosleep(&ts, NULL);
    _exit(i + 1);               /* 서로 다른 종료 코드 1..N */
}

int main(int argc, char **argv)
{
    long n = 4;
    if (argc == 2) {
        char *end = NULL;
        n = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || n < 1 || n > 8)
            n = -1;
    }
    if (argc > 2 || n == -1) {
        fprintf(stderr, "usage: %s [children 1-8]\n", argv[0]);
        return 2;
    }
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_chld;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    printf("pid=%ld forking %ld children (sleep 0.2-1.0s each)\n",
           (long)getpid(), n);
    printf("observe: ps -o pid,ppid,stat,command   (Z window, Linux server)\n");
    fflush(stdout);
    for (long i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return 1;
        }
        if (pid == 0)
            child_sleep((int)i);        /* 만들어진 순서대로 늦게 끝난다 */
    }
    long reaped = 0;
    while (reaped < n) {
        if (chlds == 0) {
            struct timespec tick = {0, 200 * 1000 * 1000};
            nanosleep(&tick, NULL);     /* 좀비 창을 잠깐 남긴다 */
            continue;
        }
        chlds = 0;
        for (;;) {
            int status;
            pid_t pid = waitpid(-1, &status, WNOHANG);
            if (pid == -1) {
                if (errno == ECHILD)
                    break;      /* 끝난 자식이 더 이상 없다 */
                perror("waitpid");
                return 1;
            }
            if (pid == 0)
                break;          /* 자식은 있지만 아직 끝난 게 없다 */
            reaped++;
            printf("reaped child=%ld exited, status=%d\n",
                   (long)pid, WEXITSTATUS(status));
            printf("  raw status=0x%04x  (%ld/%ld)\n",
                   (unsigned)status, reaped, n);
            fflush(stdout);
        }
    }
    int status;
    if (waitpid(-1, &status, WNOHANG) == -1 && errno == ECHILD)
        printf("waitpid: ECHILD -> all reaped, no zombies\n");
    return 0;
}
