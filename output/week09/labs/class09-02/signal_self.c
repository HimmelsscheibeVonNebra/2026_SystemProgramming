#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static void on_signal(int sig)
{
    int saved_errno = errno;
    if (sig == SIGINT)
        (void)write(STDOUT_FILENO, "received SIGINT\n", 16);
    else
        (void)write(STDOUT_FILENO, "received SIGUSR1\n", 17);
    errno = saved_errno;
}

int main(void)
{
    struct sigaction sa = {0};
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1 ||
        sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    printf("ready PID=%ld\n", (long)getpid());
    fflush(stdout);
    puts("before raise");
    fflush(stdout);
    if (raise(SIGUSR1) != 0)
        return 1;
    puts("after raise");
    return 0;
}
