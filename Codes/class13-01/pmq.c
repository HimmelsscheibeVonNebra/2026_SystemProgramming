/* pmq.c — POSIX 메시지 큐를 명령 하나씩으로 다룬다
 *   ./pmq create /NAME [MAXMSG MSGSIZE]   큐를 만든다 (기본 10개, 128바이트)
 *   ./pmq send   /NAME PRIO TEXT          메시지 하나를 넣는다
 *   ./pmq recv   /NAME [-n]               메시지 하나를 꺼낸다 (-n: 기다리지 않음)
 *   ./pmq stat   /NAME                    속성과 쌓인 개수를 본다
 *   ./pmq unlink /NAME                    이름을 지운다
 */
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void die(const char *what) { perror(what); exit(1); }

static void do_create(const char *name, long maxmsg, long msgsize)
{
    struct mq_attr attr = { .mq_maxmsg = maxmsg, .mq_msgsize = msgsize };
    /* O_EXCL: 같은 이름이 이미 있으면 EEXIST로 실패한다 */
    mqd_t q = mq_open(name, O_CREAT | O_EXCL | O_RDWR, 0600, &attr);
    if (q == (mqd_t)-1) die("mq_open");
    printf("만듦: %s (maxmsg=%ld, msgsize=%ld)\n", name, maxmsg, msgsize);
    mq_close(q);                        /* 닫아도 큐와 이름은 남는다 */
}

static void do_send(const char *name, unsigned prio, const char *text)
{
    mqd_t q = mq_open(name, O_WRONLY);  /* 만든 프로세스가 아니어도 된다 */
    if (q == (mqd_t)-1) die("mq_open");

    /* 메시지 하나 = 바이트 덩어리 + 우선순위. 가득 차 있으면 기다린다 */
    if (mq_send(q, text, strlen(text), prio) == -1) die("mq_send");
    printf("[%d] 보냄: prio=%u len=%zu \"%s\"\n", (int)getpid(), prio,
           strlen(text), text);
    mq_close(q);
}

static void do_recv(const char *name, int nonblock)
{
    mqd_t q = mq_open(name, O_RDONLY | (nonblock ? O_NONBLOCK : 0));
    if (q == (mqd_t)-1) die("mq_open");

    struct mq_attr attr;
    if (mq_getattr(q, &attr) == -1) die("mq_getattr");
    char *buf = malloc(attr.mq_msgsize + 1);    /* msgsize 이상이어야 한다 */
    if (buf == NULL) die("malloc");

    /* 우선순위가 가장 높은 것 중 가장 오래된 메시지 하나. 비면 기다린다 */
    unsigned prio;
    ssize_t n = mq_receive(q, buf, attr.mq_msgsize, &prio);
    if (n == -1) die("mq_receive");
    buf[n] = '\0';
    printf("[%d] 받음: prio=%u len=%zd \"%s\"\n", (int)getpid(), prio, n, buf);
    free(buf);
    mq_close(q);
}

static void do_stat(const char *name)
{
    mqd_t q = mq_open(name, O_RDONLY);
    if (q == (mqd_t)-1) die("mq_open");
    struct mq_attr attr;
    if (mq_getattr(q, &attr) == -1) die("mq_getattr");
    printf("%s: maxmsg=%ld msgsize=%ld curmsgs=%ld\n", name, attr.mq_maxmsg,
           attr.mq_msgsize, attr.mq_curmsgs);
    mq_close(q);
}

int main(int argc, char *argv[])
{
    const char *cmd = argc > 1 ? argv[1] : "";

    if (!strcmp(cmd, "create") && (argc == 3 || argc == 5))
        do_create(argv[2], argc == 5 ? atol(argv[3]) : 10,
                  argc == 5 ? atol(argv[4]) : 128);
    else if (!strcmp(cmd, "send") && argc == 5)
        do_send(argv[2], (unsigned)atoi(argv[3]), argv[4]);
    else if (!strcmp(cmd, "recv") && (argc == 3 || argc == 4))
        do_recv(argv[2], argc == 4 && !strcmp(argv[3], "-n"));
    else if (!strcmp(cmd, "stat") && argc == 3)
        do_stat(argv[2]);
    else if (!strcmp(cmd, "unlink") && argc == 3) {
        if (mq_unlink(argv[2]) == -1) die("mq_unlink");
        printf("지움: %s\n", argv[2]);
    } else {
        fprintf(stderr, "사용법: %s create|send|recv|stat|unlink /NAME ...\n",
                argv[0]);
        return 2;
    }
    return 0;
}
