/* ringbuf.c — 공유 메모리의 원형 버퍼로 만드는 생산자–소비자
 *   ./ringbuf init                 버퍼(8칸)와 세마포어 셋을 만든다
 *   ./ringbuf put START COUNT [MS] START부터 COUNT개를 넣는다 (MS: 사이 쉬는 시간)
 *   ./ringbuf get COUNT [MS]       COUNT개를 꺼내 합을 출력한다
 *   ./ringbuf stat                 빈 칸 · 든 칸의 수
 *   ./ringbuf rm                   이름을 지운다
 */
#include <string.h>
#include "ring.h"

static void put(struct ring *r, long v)
{
    sem_wait(&r->slots);            /* 빈 칸이 생길 때까지 기다린다 */
    sem_wait(&r->mutex);            /* 버퍼를 잠근다 */
    r->buf[r->tail] = v;
    r->tail = (r->tail + 1) % RING_SIZE;
    sem_post(&r->mutex);
    sem_post(&r->items);            /* 든 칸 +1: 소비자를 깨운다 */
}

static long get(struct ring *r)
{
    sem_wait(&r->items);            /* 든 칸이 생길 때까지 기다린다 */
    sem_wait(&r->mutex);
    long v = r->buf[r->head];
    r->head = (r->head + 1) % RING_SIZE;
    sem_post(&r->mutex);
    sem_post(&r->slots);            /* 빈 칸 +1: 생산자를 깨운다 */
    return v;
}

int main(int argc, char *argv[])
{
    const char *cmd = argc > 1 ? argv[1] : "";
    setvbuf(stdout, NULL, _IOLBF, 0);

    if (!strcmp(cmd, "init") && argc == 2) {
        struct ring *r = ring_attach(1);
        sem_init(&r->slots, 1, RING_SIZE);  /* 둘째 인자 1: 프로세스 사이 공유 */
        sem_init(&r->items, 1, 0);
        sem_init(&r->mutex, 1, 1);
        r->head = r->tail = 0;
        printf("만듦: %s (%d칸, %zu바이트)\n", RING_NAME, RING_SIZE, sizeof *r);
    } else if (!strcmp(cmd, "put") && (argc == 4 || argc == 5)) {
        struct ring *r = ring_attach(0);
        long start = atol(argv[2]), n = atol(argv[3]);
        for (long i = 0; i < n; i++) {
            put(r, start + i);
            printf("[%d] 넣음 %ld\n", (int)getpid(), start + i);
            if (argc == 5) usleep((useconds_t)atoi(argv[4]) * 1000);
        }
    } else if (!strcmp(cmd, "get") && (argc == 3 || argc == 4)) {
        struct ring *r = ring_attach(0);
        long n = atol(argv[2]), sum = 0;
        for (long i = 0; i < n; i++) {
            long v = get(r);
            sum += v;
            printf("[%d] 꺼냄 %ld\n", (int)getpid(), v);
            if (argc == 4) usleep((useconds_t)atoi(argv[3]) * 1000);
        }
        printf("[%d] %ld개의 합 = %ld\n", (int)getpid(), n, sum);
    } else if (!strcmp(cmd, "stat") && argc == 2) {
        struct ring *r = ring_attach(0);
        int slots, items;
        sem_getvalue(&r->slots, &slots);
        sem_getvalue(&r->items, &items);
        printf("빈 칸 %d · 든 칸 %d\n", slots, items);
    } else if (!strcmp(cmd, "rm") && argc == 2) {
        if (shm_unlink(RING_NAME) == -1) { perror("shm_unlink"); return 1; }
        printf("지움: %s\n", RING_NAME);
    } else {
        fprintf(stderr, "사용법: %s init | put START COUNT [MS] | get COUNT [MS]"
                        " | stat | rm\n", argv[0]);
        return 2;
    }
    return 0;
}
