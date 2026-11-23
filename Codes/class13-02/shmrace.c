/* shmrace.c — 공유 메모리의 값을 여러 프로세스가 잠금 없이 올린다
 *   ./shmrace init     공유 메모리를 만들고 count = 0
 *   ./shmrace add N    count를 N번 올린다 (잠금 없음)
 *   ./shmrace show     지금 값을 본다
 *   ./shmrace rm       이름을 지운다
 */
#include <string.h>
#include "counter.h"

int main(int argc, char *argv[])
{
    const char *cmd = argc > 1 ? argv[1] : "";

    if (!strcmp(cmd, "init") && argc == 2) {
        struct shared *s = attach(1);
        if (sem_init(&s->lock, 1, 1) == -1) { perror("sem_init"); return 1; }
        s->count = 0;
        printf("만듦: %s (%zu바이트), count = 0\n", SHM_NAME, sizeof *s);
    } else if (!strcmp(cmd, "add") && argc == 3) {
        struct shared *s = attach(0);
        long n = atol(argv[2]);
        for (long i = 0; i < n; i++)
            s->count++;                 /* 읽고 · 더하고 · 쓴다: 세 단계 */
        printf("[%d] %ld번 올림 → count = %ld\n", (int)getpid(), n, s->count);
    } else if (!strcmp(cmd, "show") && argc == 2) {
        printf("count = %ld\n", attach(0)->count);
    } else if (!strcmp(cmd, "rm") && argc == 2) {
        if (shm_unlink(SHM_NAME) == -1) { perror("shm_unlink"); return 1; }
        printf("지움: %s\n", SHM_NAME);
    } else {
        fprintf(stderr, "사용법: %s init | add N | show | rm\n", argv[0]);
        return 2;
    }
    return 0;
}
