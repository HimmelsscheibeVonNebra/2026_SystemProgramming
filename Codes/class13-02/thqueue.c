/* thqueue.c — 뮤텍스와 조건 변수로 만드는 생산자–소비자 (스레드)
 *   ./thqueue P C N [MS]   생산자 P개가 N개씩 넣고 소비자 C개가 나눠 꺼낸다
 *                          (MS: 소비자가 하나 꺼낼 때마다 쉬는 시간)
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 8
static long buf[SIZE];
static int head, tail, used, done;      /* used: 든 칸의 수 */
static long full_waits, empty_waits;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
static pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

static int verbose;

static void put(long v, long who)
{
    pthread_mutex_lock(&lock);
    while (used == SIZE) {              /* if가 아니라 while: 깨면 다시 본다 */
        full_waits++;
        pthread_cond_wait(&not_full, &lock);    /* 잠금을 풀고 잠든다 */
    }                                   /* 돌아왔을 때는 다시 잠근 상태다 */
    buf[tail] = v;
    tail = (tail + 1) % SIZE;
    used++;
    if (verbose) printf("[생산자 %ld] 넣음 %ld (든 칸 %d)\n", who, v, used);
    pthread_cond_signal(&not_empty);    /* 기다리는 소비자 하나를 깨운다 */
    pthread_mutex_unlock(&lock);
}

static long get(long who)
{
    pthread_mutex_lock(&lock);
    while (used == 0 && !done) {
        empty_waits++;
        pthread_cond_wait(&not_empty, &lock);
    }
    if (used == 0) {                    /* 비었고, 더 올 것도 없다 */
        pthread_mutex_unlock(&lock);
        return -1;
    }
    long v = buf[head];
    head = (head + 1) % SIZE;
    used--;
    if (verbose) printf("[소비자 %ld] 꺼냄 %ld (든 칸 %d)\n", who, v, used);
    pthread_cond_signal(&not_full);     /* 기다리는 생산자 하나를 깨운다 */
    pthread_mutex_unlock(&lock);
    return v;
}

static long n, ms;

static void *producer(void *arg)
{
    long id = (long)arg;
    for (long i = 0; i < n; i++)
        put(id * 100 + i, id);
    return NULL;
}

static void *consumer(void *arg)
{
    long id = (long)arg, sum = 0, v;
    while ((v = get(id)) != -1) {       /* -1: 비었고 더 올 것이 없다 */
        sum += v;
        if (ms) usleep((useconds_t)ms * 1000);
    }
    return (void *)sum;                 /* pthread_join이 받아 간다 */
}

int main(int argc, char *argv[])
{
    if (argc != 4 && argc != 5) {
        fprintf(stderr, "사용법: %s P C N [MS]\n", argv[0]);
        return 2;
    }
    long np = atol(argv[1]), nc = atol(argv[2]);
    n = atol(argv[3]);
    ms = argc == 5 ? atol(argv[4]) : 0;
    if (np < 1 || np > 16 || nc < 1 || nc > 16 || n < 1) return 2;
    verbose = np * n <= 20;
    setvbuf(stdout, NULL, _IOLBF, 0);

    pthread_t p[16], c[16];
    for (long i = 0; i < nc; i++)
        pthread_create(&c[i], NULL, consumer, (void *)(i + 1));
    for (long i = 0; i < np; i++)
        pthread_create(&p[i], NULL, producer, (void *)(i + 1));
    for (long i = 0; i < np; i++) pthread_join(p[i], NULL);

    pthread_mutex_lock(&lock);
    done = 1;                           /* 조건이 바뀌었으니 */
    pthread_cond_broadcast(&not_empty); /* 기다리는 소비자를 모두 깨운다 */
    pthread_mutex_unlock(&lock);

    long total = 0, want = 0;
    for (long i = 0; i < nc; i++) {
        void *sum;
        pthread_join(c[i], &sum);
        total += (long)sum;
    }
    for (long id = 1; id <= np; id++)
        for (long i = 0; i < n; i++) want += id * 100 + i;
    printf("꺼낸 값의 합 = %ld (기대 %ld)\n", total, want);
    printf("가득 차서 기다림 %ld번 · 비어서 기다림 %ld번\n", full_waits,
           empty_waits);
    return total == want ? 0 : 1;
}
