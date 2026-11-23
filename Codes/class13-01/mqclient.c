/* mqclient.c — 요청 하나를 보내고 자기 응답 큐에서 답을 기다린다 (최대 3초)
 *   ./mqclient TEXT      (./mqclient quit 은 서버를 끝낸다)
 */
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "proto.h"

int main(int argc, char *argv[])
{
    if (argc != 2) { fprintf(stderr, "사용법: %s TEXT\n", argv[0]); return 2; }

    struct request req = { .pid = getpid() };
    snprintf(req.text, sizeof req.text, "%s", argv[1]);
    int quit = !strcmp(req.text, "quit");       /* quit에는 응답이 없다 */

    /* ① 응답 큐를 먼저 만든다 — 요청을 보낸 뒤에 만들면 늦을 수 있다 */
    char name[32];
    reply_name(name, sizeof name, req.pid);
    struct mq_attr attr = { .mq_maxmsg = 1,
                            .mq_msgsize = sizeof(struct reply) };
    mqd_t repq = mq_open(name, O_CREAT | O_EXCL | O_RDONLY, 0622, &attr);
    if (repq == (mqd_t)-1) { perror("mq_open(응답 큐)"); return 1; }

    /* ② 서버의 요청 큐를 이름으로 찾아 요청을 넣는다 */
    int rc = 1;
    mqd_t reqq = mq_open(REQ_QUEUE, O_WRONLY);
    if (reqq == (mqd_t)-1) {
        perror("서버가 없다: mq_open(" REQ_QUEUE ")");
        goto out;
    }
    if (mq_send(reqq, (char *)&req, sizeof req, 0) == -1) {
        perror("mq_send");
        goto out;
    }
    if (quit) { rc = 0; goto out; }

    /* ③ 내 응답 큐에서 기다린다. 서버가 죽었을 수 있으므로 3초까지만 */
    struct timespec until;
    clock_gettime(CLOCK_REALTIME, &until);
    until.tv_sec += 3;
    struct reply rep;
    if (mq_timedreceive(repq, (char *)&rep, sizeof rep, NULL, &until) == -1) {
        perror("응답 없음: mq_timedreceive");
        goto out;
    }
    printf("[클라이언트 %d] #%d %s → %s\n", (int)req.pid, rep.seq, req.text,
           rep.text);
    rc = 0;

out:
    if (reqq != (mqd_t)-1) mq_close(reqq);
    mq_close(repq);
    mq_unlink(name);                    /* 내 응답 큐는 내가 지운다 */
    return rc;
}
