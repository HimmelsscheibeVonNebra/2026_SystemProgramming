/* semcount.c — 같은 값을 세마포어로 잠그고 올린다 (shmrace init으로 만든 뒤)
 *   ./semcount add N     잠그고 올리고 푼다 × N
 *   ./semcount hold SEC  잠근 채로 SEC초 동안 쥐고 있는다
 *   ./semcount try       2초까지만 기다렸다가 한 번 올린다
 *   ./semcount value     세마포어의 지금 값
 */
#include <string.h>
#include <time.h>
#include "counter.h"

int main(int argc, char *argv[])
{
    const char *cmd = argc > 1 ? argv[1] : "";
    struct shared *s = attach(0);

    if (!strcmp(cmd, "add") && argc == 3) {
        long n = atol(argv[2]);
        for (long i = 0; i < n; i++) {
            sem_wait(&s->lock);         /* 1 → 0. 이미 0이면 여기서 잔다 */
            s->count++;                 /* 임계 구역: 한 번에 한 프로세스만 */
            sem_post(&s->lock);         /* 0 → 1. 기다리던 프로세스를 깨운다 */
        }
        printf("[%d] %ld번 올림 → count = %ld\n", (int)getpid(), n, s->count);
    } else if (!strcmp(cmd, "hold") && argc == 3) {
        sem_wait(&s->lock);
        printf("[%d] 잠금: %s초 동안 쥔다\n", (int)getpid(), argv[2]);
        fflush(stdout);
        sleep((unsigned)atoi(argv[2])); /* 이 사이에 죽으면 잠긴 채로 남는다 */
        sem_post(&s->lock);
        printf("[%d] 풂\n", (int)getpid());
    } else if (!strcmp(cmd, "try") && argc == 2) {
        struct timespec until;
        clock_gettime(CLOCK_REALTIME, &until);
        until.tv_sec += 2;              /* 마감 시각 = 지금 + 2초 */
        if (sem_timedwait(&s->lock, &until) == -1) {
            perror("잠글 수 없다: sem_timedwait");
            return 1;
        }
        s->count++;
        sem_post(&s->lock);
        printf("[%d] 잠그고 한 번 올림 → count = %ld\n", (int)getpid(), s->count);
    } else if (!strcmp(cmd, "value") && argc == 2) {
        int v;
        sem_getvalue(&s->lock, &v);
        printf("세마포어 값 = %d (%s)\n", v, v > 0 ? "열림" : "잠김");
    } else {
        fprintf(stderr, "사용법: %s add N | hold SEC | try | value\n", argv[0]);
        return 2;
    }
    return 0;
}
