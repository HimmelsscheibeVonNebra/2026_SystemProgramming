#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* sigaction으로 시그널마다 처리 방법을 정한다.
   SIGINT : 잡아서 센다. 세 번째에 끝난다.
   SIGTERM: 잡아서 정리 메시지를 내고 끝난다.
   SIGQUIT: 무시한다(Ctrl-\를 눌러도 아무 일 없음).
   SIGKILL: 바꾸려 해 보지만 커널이 EINVAL로 거절한다.
   핸들러는 write와 플래그만 쓰고, printf는 main에서 한다.
   플래그 검사와 pause 사이의 경쟁은 2차시(sigsuspend)에서 다룬다. */

static volatile sig_atomic_t ints = 0;
static volatile sig_atomic_t got_term = 0;

static void on_int(int sig)
{
    static const char msg[] = "  [handler] SIGINT\n";
    (void)sig;
    ssize_t r = write(STDOUT_FILENO, msg, sizeof msg - 1);
    (void)r;                    /* 핸들러 안에서는 printf 대신 write */
    ints++;                     /* 핸들러 실행 중에는 SIGINT가 막혀 있다 */
}

static void on_term(int sig)
{
    (void)sig;
    got_term = 1;
}

static int set_action(int sig, void (*handler)(int))
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    return sigaction(sig, &sa, NULL);
}

int main(void)
{
    if (set_action(SIGINT, on_int) == -1 || set_action(SIGTERM, on_term) == -1
        || set_action(SIGQUIT, SIG_IGN) == -1) {
        perror("sigaction");
        return 1;
    }
    if (set_action(SIGKILL, SIG_IGN) == -1)
        printf("sigaction(SIGKILL): %s\n", strerror(errno));
    printf("pid=%ld  Ctrl-C: count (3 to quit)  Ctrl-\\: ignored  "
           "kill -TERM %ld: clean exit\n", (long)getpid(), (long)getpid());
    fflush(stdout);

    long seen = 0;
    while (!got_term && ints < 3) {
        pause();                /* 잡은 시그널의 핸들러가 끝나면 돌아온다 */
        while (seen < ints)
            printf("main: SIGINT #%ld\n", ++seen);
        fflush(stdout);
    }
    if (got_term)
        printf("main: SIGTERM -> cleanup, exit 0\n");
    else
        printf("main: SIGINT x3 -> exit 0\n");
    return 0;
}
