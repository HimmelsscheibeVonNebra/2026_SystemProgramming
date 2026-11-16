/* sigctl.c — 부모가 시그널로 자식(worker)을 멈추고, 다시 돌리고,
 *            상태를 묻고, 끝낸다. 자식의 상태 변화는 waitpid로 확인한다.
 * 사용법: ./sigctl [term|kill|late]
 *   term  마지막에 SIGTERM — 자식이 정리하고 exit 0 (기본)
 *   kill  마지막에 SIGKILL — 정리 없이 즉시 종료
 *   late  자식이 핸들러를 늦게 설치 — 그 전에 온 SIGUSR1에 죽는다
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

static volatile sig_atomic_t want_report = 0, want_quit = 0;
static void on_usr1(int sig) { (void)sig; want_report = 1; }
static void on_term(int sig) { (void)sig; want_quit = 1; }

static void install(int sig, void (*handler)(int))
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sig, &sa, NULL) < 0) { perror("sigaction"); exit(1); }
}

static void sleep_ms(long n)
{
    struct timespec d = { n / 1000, (n % 1000) * 1000000L };
    while (nanosleep(&d, &d) < 0)  /* 시그널로 깨면 남은 시간만큼 */
        ;
}

static void worker(int late)  /* 자식: 100 ms마다 한 칸씩 일한다 */
{
    if (late) {  /* 일부러 늦게 설치 */
        sleep_ms(300);
        install(SIGUSR1, on_usr1);
        install(SIGTERM, on_term);
    }
    long ticks = 0;
    struct timespec tick = { 0, 100000000L };
    while (!want_quit) {
        nanosleep(&tick, NULL);  /* 시그널이 오면 일찍 깰 수 있다 */
        ticks++;
        if (want_report) {
            want_report = 0;
            printf("    [child %d] report: %ld ticks\n", (int)getpid(), ticks);
            fflush(stdout);
        }
    }
    printf("    [child %d] got SIGTERM: cleanup, exit 0 (%ld ticks)\n",
           (int)getpid(), ticks);
    fflush(stdout);
    _exit(0);
}

static void show(pid_t pid, int st)  /* waitpid 결과를 사람 말로 */
{
    int sig = WIFSTOPPED(st)  ? WSTOPSIG(st)
            : WIFSIGNALED(st) ? WTERMSIG(st) : 0;
    printf("parent: child %d ", (int)pid);
    if (WIFSTOPPED(st))
        printf("stopped by signal %d (%s)\n", sig, strsignal(sig));
    else if (WIFCONTINUED(st))
        printf("continued\n");
    else if (WIFEXITED(st))
        printf("exited with %d\n", WEXITSTATUS(st));
    else if (WIFSIGNALED(st))
        printf("killed by signal %d (%s)\n", sig, strsignal(sig));
    fflush(stdout);
}

/* 시그널을 보내고, wait이 참이면 상태 변화를 기다려 보여 준다 */
static void send(pid_t pid, int sig, const char *name, int wait)
{
    printf("parent: kill(%d, %s)\n", (int)pid, name);
    fflush(stdout);
    if (kill(pid, sig) < 0) { perror("kill"); exit(1); }
    if (!wait) return;
    int status;
    if (waitpid(pid, &status, WUNTRACED | WCONTINUED) < 0) {
        perror("waitpid");
        exit(1);
    }
    show(pid, status);
}

int main(int argc, char *argv[])
{
    const char *mode = argc > 1 ? argv[1] : "term";
    int late = strcmp(mode, "late") == 0;
    if (!late) {  /* fork 전에 설치 → 자식이 물려받는다 */
        install(SIGUSR1, on_usr1);
        install(SIGTERM, on_term);
    }
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) worker(late);

    printf("parent: started worker %d (mode %s)\n", (int)pid, mode);
    if (late) {  /* 자식이 설치하기 전에 보낸다 */
        sleep_ms(50);
        send(pid, SIGUSR1, "SIGUSR1", 1);
        return 0;
    }
    sleep_ms(350);  send(pid, SIGUSR1, "SIGUSR1", 0);
    sleep_ms(100);  send(pid, SIGSTOP, "SIGSTOP", 1);
    sleep_ms(500);  /* 멈춘 동안 자식은 일하지 않는다 */
    send(pid, SIGCONT, "SIGCONT", 1);
    sleep_ms(250);  send(pid, SIGUSR1, "SIGUSR1", 0);
    sleep_ms(100);
    if (strcmp(mode, "kill") == 0) send(pid, SIGKILL, "SIGKILL", 1);
    else                           send(pid, SIGTERM, "SIGTERM", 1);
    return 0;
}
