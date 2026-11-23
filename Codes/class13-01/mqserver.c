/* mqserver.c — 요청 큐 하나로 여러 클라이언트의 요청을 받아 대문자로 바꿔 돌려준다
 *   ./mqserver        (끝내기: ./mqclient quit 또는 Ctrl-C)
 */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "proto.h"

static volatile sig_atomic_t stop;
static void on_signal(int sig) { (void)sig; stop = 1; }

int main(void)
{
    struct sigaction sa = { .sa_handler = on_signal };  /* SA_RESTART 없음 */
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    struct mq_attr attr = { .mq_maxmsg = 10,
                            .mq_msgsize = sizeof(struct request) };
    mq_unlink(REQ_QUEUE);               /* 지난 실행이 남긴 큐를 치운다 */
    mqd_t reqq = mq_open(REQ_QUEUE, O_CREAT | O_EXCL | O_RDONLY, 0622, &attr);
    if (reqq == (mqd_t)-1) { perror("mq_open"); return 1; }
    fprintf(stderr, "[서버 %d] %s 에서 기다림\n", (int)getpid(), REQ_QUEUE);

    for (int seq = 1; !stop; seq++) {
        struct request req;
        if (mq_receive(reqq, (char *)&req, sizeof req, NULL) == -1) {
            if (errno == EINTR) continue;       /* 시그널: stop을 다시 본다 */
            perror("mq_receive");
            break;
        }
        req.text[TEXT_MAX - 1] = '\0';          /* 받은 것은 믿지 않는다 */
        if (!strcmp(req.text, "quit")) break;

        struct reply rep = { .seq = seq };
        for (int i = 0; i < TEXT_MAX; i++)
            rep.text[i] = (char)toupper((unsigned char)req.text[i]);

        char name[32];
        reply_name(name, sizeof name, req.pid);
        mqd_t repq = mq_open(name, O_WRONLY | O_NONBLOCK);
        if (repq == (mqd_t)-1) {                /* 클라이언트가 이미 떠났다 */
            fprintf(stderr, "[서버] #%d PID %d: 응답 큐 없음 (%s)\n", seq,
                    (int)req.pid, strerror(errno));
            continue;
        }
        if (mq_send(repq, (char *)&rep, sizeof rep, 0) == -1)
            perror("mq_send");
        mq_close(repq);
        fprintf(stderr, "[서버] #%d PID %d: %s → %s\n", seq, (int)req.pid,
                req.text, rep.text);
    }

    mq_close(reqq);
    mq_unlink(REQ_QUEUE);               /* 이름을 지워야 큐가 사라진다 */
    fprintf(stderr, "[서버] 끝: %s 지움\n", REQ_QUEUE);
    return 0;
}
