/* fifosrv.c — FIFO 하나로 여러 클라이언트의 요청을 받는 덧셈 서버.
 * 요청은 모두 서버 FIFO로 오고, 답은 클라이언트마다 따로 만든 FIFO로 간다.
 * 사용법: ./fifosrv serve FIFO | ask FIFO A B | burst FIFO K | stop FIFO
 */
#include <errno.h>                  /* errno, EEXIST */
#include <fcntl.h>                  /* open, O_RDONLY, O_WRONLY */
#include <signal.h>                 /* signal, SIGPIPE, SIG_IGN */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>               /* mkfifo */
#include <sys/wait.h>               /* wait, WEXITSTATUS */
#include <unistd.h>                 /* read, write, close, unlink, getpid, fork */

struct request {                    /* 고정 길이 12바이트: write 한 번으로 보낸다 */
    pid_t pid;                      /* 보낸 클라이언트. 0이면 "서버 종료" */
    int a, b;                       /* 더할 두 수 */
};

static void die(const char *what)   /* 실패한 호출 이름과 errno 메시지를 내고 끝낸다 */
{
    perror(what);
    exit(1);
}

/* 응답 FIFO의 경로: 서버 FIFO 이름 뒤에 클라이언트 PID를 붙인다. */
static void reply_path(char *buf, size_t size, const char *fifo, pid_t pid)
{
    snprintf(buf, size, "%s.%ld", fifo, (long)pid);
}

/* serve: 요청을 하나씩 읽어 답한다. pid가 0인 요청을 받으면 끝낸다. */
static int run_serve(const char *fifo)
{
    struct request rq;
    char path[256];
    long served = 0;                /* 답한 요청 수 */
    ssize_t n;

    if (mkfifo(fifo, 0600) == -1 && errno != EEXIST)
        die("mkfifo");
    signal(SIGPIPE, SIG_IGN);       /* 답을 안 읽고 떠난 클라이언트 때문에 죽지 않는다 */
    printf("[서버 %ld] %s 에서 기다린다\n", (long)getpid(), fifo);
    fflush(stdout);

    int in = open(fifo, O_RDONLY);  /* 첫 클라이언트가 열 때까지 잠든다 */
    if (in == -1)
        die("open");
    int hold = open(fifo, O_WRONLY);    /* 서버도 쓰기 끝을 하나 쥔다: EOF가 안 온다 */
    if (hold == -1)
        die("open");

    while ((n = read(in, &rq, sizeof rq)) == (ssize_t)sizeof rq) {  /* 요청 하나씩 */
        if (rq.pid == 0)            /* stop이 보낸 종료 요청 */
            break;
        int sum = rq.a + rq.b;
        reply_path(path, sizeof path, fifo, rq.pid);
        int out = open(path, O_WRONLY);     /* 그 클라이언트의 응답 FIFO */
        if (out == -1) {
            perror(path);           /* 클라이언트가 사라졌다: 건너뛴다 */
            continue;
        }
        if (write(out, &sum, sizeof sum) == -1)
            perror("write");
        close(out);                 /* 클라이언트가 EOF를 받는다 */
        served++;
        printf("[서버] %ld번째: PID %ld  %d + %d = %d\n",
               served, (long)rq.pid, rq.a, rq.b, sum);
        fflush(stdout);
    }
    close(hold);
    close(in);
    unlink(fifo);                   /* 이름을 지운다 */
    printf("[서버] 요청 %ld개를 처리하고 끝낸다\n", served);
    return 0;
}

/* 요청 하나를 보내고 답을 받아 돌려준다. 실패하면 프로세스를 끝낸다. */
static int ask(const char *fifo, int a, int b)
{
    struct request rq = { getpid(), a, b };
    char path[256];
    int sum;

    reply_path(path, sizeof path, fifo, rq.pid);
    if (mkfifo(path, 0600) == -1 && errno != EEXIST)    /* ① 답 받을 곳부터 만든다 */
        die("mkfifo");
    int out = open(fifo, O_WRONLY);                     /* ② 서버 FIFO를 연다 */
    if (out == -1) {
        perror(fifo);               /* 서버가 없다 (ENOENT) */
        unlink(path);
        exit(1);
    }
    if (write(out, &rq, sizeof rq) != (ssize_t)sizeof rq)   /* ③ 요청: write 한 번 */
        die("write");
    close(out);
    int in = open(path, O_RDONLY);                      /* ④ 서버가 열 때까지 잠든다 */
    if (in == -1)
        die("open");
    if (read(in, &sum, sizeof sum) != (ssize_t)sizeof sum)  /* ⑤ 답을 읽는다 */
        die("read");
    close(in);
    unlink(path);                   /* 내 응답 FIFO를 지운다 */
    return sum;
}

/* burst: 자식 K개가 동시에 하나씩 묻는다. 모두 맞는 답을 받았는지 센다. */
static int run_burst(const char *fifo, int k)
{
    int ok = 0;                     /* 맞는 답을 받은 자식 수 */

    for (int i = 1; i <= k; i++) {
        pid_t pid = fork();
        if (pid == -1)
            die("fork");
        if (pid == 0)               /* 자식마다 PID가 달라 응답 FIFO도 다르다 */
            exit(ask(fifo, i, i * 1000) == i + i * 1000 ? 0 : 1);
    }
    for (int i = 0; i < k; i++) {
        int st;
        if (wait(&st) != -1 && WIFEXITED(st) && WEXITSTATUS(st) == 0)
            ok++;
    }
    printf("[묶음] 동시에 %d개를 보내 %d개가 맞는 답을 받았다\n", k, ok);
    return ok == k ? 0 : 1;
}

/* stop: pid가 0인 요청을 보내 서버를 끝낸다. */
static int run_stop(const char *fifo)
{
    struct request rq = { 0, 0, 0 };
    int out = open(fifo, O_WRONLY);

    if (out == -1) {
        perror(fifo);
        return 1;
    }
    if (write(out, &rq, sizeof rq) == -1)
        die("write");
    close(out);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc == 3 && strcmp(argv[1], "serve") == 0)
        return run_serve(argv[2]);
    if (argc == 5 && strcmp(argv[1], "ask") == 0) {
        int a = atoi(argv[3]), b = atoi(argv[4]);
        printf("[클라이언트 %ld] %d + %d = %d\n", (long)getpid(), a, b, ask(argv[2], a, b));
        return 0;
    }
    if (argc == 4 && strcmp(argv[1], "burst") == 0)
        return run_burst(argv[2], atoi(argv[3]));
    if (argc == 3 && strcmp(argv[1], "stop") == 0)
        return run_stop(argv[2]);
    fprintf(stderr, "사용법: %s serve FIFO | ask FIFO A B | burst FIFO K | stop FIFO\n",
            argv[0]);
    return 2;
}
