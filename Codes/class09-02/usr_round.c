#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* 두 프로세스가 SIGUSR1·SIGUSR2로 왕복합니다. 부모가 fork한 자식에게
   kill(child, SIGUSR1)을 보내면, 자식의 핸들러가 kill(getppid(),
   SIGUSR2)로 회신합니다. 이런 왕복을 R번(기본 4) 한 뒤 각자 세어 본
   수를 보고하고 정상 종료합니다. kill()은 PID로 대상을 고르므로
   같은 시그널이라도 누구에게 가는지가 중요합니다.
   표준 시그널은 큐잉되지 않습니다: 보류 중에 같은 번호가 다시
   도착하면 하나로 합쳐집니다(개수·순서 보장이 없습니다). 여기서는
   한 번씩 주고받아 충돌을 만들지 않습니다. 부모는 회신을 짧은
   폴링으로 기다립니다. 검사 뒤 pause()로 기다리면 회신이 그 사이에
   도착해 놓치는 경쟁이 생기는데, 경쟁 없는 대기는 이 폴더의
   suspend_demo가 다루는 sigsuspend로 없앱니다. */

static volatile sig_atomic_t got = 0;

static void on_usr1(int sig)            /* 자식: 회신한다 */
{
    static const char msg[] = "  [child handler] USR1 -> replying USR2\n";
    (void)sig;
    ssize_t r = write(STDOUT_FILENO, msg, sizeof msg - 1);
    (void)r;
    got++;
    pid_t pp = getppid();               /* 시스템 호출이라 안전하다 */
    kill(pp, SIGUSR2);
}

static void on_usr2(int sig)            /* 부모: 받기만 한다 */
{
    static const char msg[] = "  [parent handler] USR2 received\n";
    (void)sig;
    ssize_t r = write(STDOUT_FILENO, msg, sizeof msg - 1);
    (void)r;
    got++;
}

static int set_handler(int sig, void (*handler)(int))
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    return sigaction(sig, &sa, NULL);
}

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char **argv)
{
    long rounds = 4;
    if (argc == 2) {
        char *end = NULL;
        rounds = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || rounds < 1 || rounds > 10)
            rounds = -1;
    }
    if (argc > 2 || rounds == -1) {
        fprintf(stderr, "usage: %s [rounds 1-10]\n", argv[0]);
        return 2;
    }
    int sync[2];
    if (pipe(sync) == -1) {
        perror("pipe");
        return 1;
    }
    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return 1;
    }
    if (child == 0) {
        close(sync[0]);
        if (set_handler(SIGUSR1, on_usr1) == -1) {
            perror("sigaction");
            _exit(1);
        }
        alarm(10);              /* 안전망: 영원히 매달리지 않는다 */
        printf("child=%ld ready for %ld rounds\n", (long)getpid(), rounds);
        fflush(stdout);
        char b = 'r';
        ssize_t w = write(sync[1], &b, 1);      /* 핸들러 설치를 알린다 */
        (void)w;
        close(sync[1]);
        while (got < rounds)
            pause();            /* USR1 핸들러가 끝나면 돌아온다 */
        printf("child=%ld done: received %d USR1, replied %d USR2\n",
               (long)getpid(), (int)got, (int)got);
        fflush(stdout);
        _exit(0);
    }
    close(sync[1]);
    if (set_handler(SIGUSR2, on_usr2) == -1) {
        perror("sigaction");
        return 1;
    }
    char b;
    if (read(sync[0], &b, 1) != 1) {    /* 자식이 준비될 때까지 동기화 */
        perror("read");
        return 1;
    }
    close(sync[0]);
    printf("parent=%ld child=%ld rounds=%ld\n",
           (long)getpid(), (long)child, rounds);
    fflush(stdout);
    for (long r = 1; r <= rounds; r++) {
        got = 0;
        kill(child, SIGUSR1);
        double t0 = now();
        while (got == 0) {      /* 짧은 폴링: 검사-대기 경쟁을 피한다 */
            struct timespec spin = {0, 10 * 1000 * 1000};
            nanosleep(&spin, NULL);
            if (now() - t0 > 5.0) {
                fprintf(stderr, "parent: no reply in 5s (round %ld)\n", r);
                kill(child, SIGKILL);
                waitpid(child, NULL, 0);
                return 1;
            }
        }
        printf("round %ld/%ld done (parent got %d USR2)\n", r, rounds, got);
        fflush(stdout);
    }
    int status;
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }
    if (WIFEXITED(status))
        printf("child=%ld exited, status=%d\n",
               (long)child, WEXITSTATUS(status));
    else
        printf("child=%ld killed by signal %d\n",
               (long)child, WTERMSIG(status));
    printf("parent done: received %ld rounds of USR2\n", rounds);
    return 0;
}
