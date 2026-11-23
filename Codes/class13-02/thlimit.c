/* thlimit.c — 계수 세마포어: 한 번에 K개 스레드만 들어간다
 *   ./thlimit THREADS K     (예: ./thlimit 6 2)
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static sem_t room;              /* 남은 자리의 수: 처음에 K */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int inside, peak;        /* 지금 안에 있는 수 · 가장 많았을 때 */

static void *worker(void *arg)
{
    long id = (long)arg;

    sem_wait(&room);                    /* 자리가 없으면 여기서 잔다 */

    pthread_mutex_lock(&lock);          /* inside · peak는 뮤텍스가 지킨다 */
    inside++;
    if (inside > peak) peak = inside;
    printf("[스레드 %ld] 들어감 (안에 %d개)\n", id, inside);
    pthread_mutex_unlock(&lock);

    usleep(100 * 1000);                 /* 안에서 0.1초 동안 일한다 */

    pthread_mutex_lock(&lock);
    inside--;
    printf("[스레드 %ld] 나옴\n", id);
    pthread_mutex_unlock(&lock);

    sem_post(&room);                    /* 자리 +1: 기다리던 스레드가 들어온다 */
    return NULL;
}

int main(int argc, char *argv[])
{
    long nth = argc == 3 ? atol(argv[1]) : 0, k = argc == 3 ? atol(argv[2]) : 0;
    if (nth < 1 || nth > 64 || k < 1) {
        fprintf(stderr, "사용법: %s THREADS K\n", argv[0]);
        return 2;
    }
    setvbuf(stdout, NULL, _IOLBF, 0);
    sem_init(&room, 0, (unsigned)k);    /* 스레드 사이(0) · 처음 값 K */

    pthread_t t[64];
    for (long i = 0; i < nth; i++)
        pthread_create(&t[i], NULL, worker, (void *)(i + 1));
    for (long i = 0; i < nth; i++)
        pthread_join(t[i], NULL);

    printf("가장 많을 때 안에 %d개 (한도 %ld)\n", peak, k);
    return 0;
}
