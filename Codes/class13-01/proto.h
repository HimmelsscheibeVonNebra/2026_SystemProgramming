/* proto.h — mqserver와 mqclient가 함께 쓰는 약속(프로토콜) */
#ifndef PROTO_H
#define PROTO_H

#include <stdio.h>
#include <sys/types.h>

#define REQ_QUEUE "/w13_req"            /* 서버의 요청 큐: 모두가 아는 이름 */
#define TEXT_MAX  64

struct request {                        /* 클라이언트 → 서버 */
    pid_t pid;                          /* 응답을 돌려받을 곳 */
    char  text[TEXT_MAX];
};

struct reply {                          /* 서버 → 클라이언트 */
    int  seq;                           /* 서버가 처리한 순번 */
    char text[TEXT_MAX];
};

/* 응답 큐의 이름은 클라이언트마다 다르다: /w13_rep.PID */
static inline void reply_name(char *buf, size_t size, pid_t pid)
{
    snprintf(buf, size, "/w13_rep.%d", (int)pid);
}

#endif
