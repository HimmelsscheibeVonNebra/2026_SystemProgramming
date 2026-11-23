/* thcount.c — 스레드 여럿이 전역 변수 하나를 올린다
 *   ./thcount none  [THREADS [N]]   잠금 없이
 *   ./thcount mutex [THREADS [N]]   뮤텍스로 잠그고
 *   ./thcount sem   [THREADS [N]]   세마포어로 잠그고   (기본: 4개 × 1000000번)
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile long count;     /* 전역 변수: 모든 스레드가 같은 것을 본다 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static sem_t sem;
static long n = 1000000;
static char mode;

static void *worker(void *arg)
{
    (void)arg;
    for (long i = 0; i < n; i++) {
        if (mode == 'm') {
            pthread_mutex_lock(&lock);      /* 잠근 스레드가 */
            count++;
            pthread_mutex_unlock(&lock);    /* 직접 푼다 */
        } else if (mode == 's') {
            sem_wait(&sem);
            count++;
            sem_post(&sem);
        } else {
            count++;                        /* 읽고 · 더하고 · 쓴다 */
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int nth = argc > 2 ? atoi(argv[2]) : 4;
    if (argc > 3) n = atol(argv[3]);
    if (argc < 2 || (strcmp(argv[1], "none") && strcmp(argv[1], "mutex") &&
                     strcmp(argv[1], "sem")) || nth < 1 || nth > 64) {
        fprintf(stderr, "사용법: %s none|mutex|sem [THREADS [N]]\n", argv[0]);
        return 2;
    }
    mode = argv[1][0];
    sem_init(&sem, 0, 1);       /* 둘째 인자 0: 한 프로세스의 스레드 사이 */

    pthread_t t[64];
    for (int i = 0; i < nth; i++)
        pthread_create(&t[i], NULL, worker, NULL);
    for (int i = 0; i < nth; i++)
        pthread_join(t[i], NULL);           /* 스레드가 끝나기를 기다린다 */

    printf("%-5s 스레드 %d개 × %ld번 → count = %ld (기대 %ld)\n", argv[1], nth,
           n, count, nth * n);
    return 0;
}
