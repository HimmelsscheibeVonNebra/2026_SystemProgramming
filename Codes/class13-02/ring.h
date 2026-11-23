/* ring.h — 공유 메모리에 놓는 원형 버퍼와 세마포어 셋 */
#ifndef RING_H
#define RING_H

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define RING_NAME "/w13_ring"
#define RING_SIZE 8

struct ring {
    sem_t slots;                /* 빈 칸의 수: 처음에 RING_SIZE */
    sem_t items;                /* 든 칸의 수: 처음에 0 */
    sem_t mutex;                /* head · tail · buf를 지키는 잠금: 처음에 1 */
    int   head;                 /* 다음에 꺼낼 자리 */
    int   tail;                 /* 다음에 넣을 자리 */
    long  buf[RING_SIZE];
};

static inline struct ring *ring_attach(int create)
{
    int flags = create ? O_CREAT | O_EXCL | O_RDWR : O_RDWR;
    int fd = shm_open(RING_NAME, flags, 0600);
    if (fd == -1) { perror("shm_open"); exit(1); }
    if (create && ftruncate(fd, sizeof(struct ring)) == -1) {
        perror("ftruncate");
        exit(1);
    }
    struct ring *r = mmap(NULL, sizeof *r, PROT_READ | PROT_WRITE,
                          MAP_SHARED, fd, 0);
    if (r == MAP_FAILED) { perror("mmap"); exit(1); }
    close(fd);
    return r;
}

#endif
