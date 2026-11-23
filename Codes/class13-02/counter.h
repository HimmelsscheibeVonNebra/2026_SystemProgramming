/* counter.h — shmrace와 semcount가 함께 쓰는 공유 메모리의 모양 */
#ifndef COUNTER_H
#define COUNTER_H

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/w13_counter"

struct shared {                 /* 공유 메모리의 맨 앞에 이 구조체가 놓인다 */
    sem_t lock;                 /* 2부에서 쓴다: 이진 세마포어 (1 = 열림) */
    volatile long count;        /* 여러 프로세스가 함께 올리는 값 */
};

/* 이름으로 열어 내 주소 공간에 붙인다. create이면 만들고 크기를 정한다 */
static inline struct shared *attach(int create)
{
    int flags = create ? O_CREAT | O_EXCL | O_RDWR : O_RDWR;
    int fd = shm_open(SHM_NAME, flags, 0600);
    if (fd == -1) { perror("shm_open"); exit(1); }
    if (create && ftruncate(fd, sizeof(struct shared)) == -1) {
        perror("ftruncate");
        exit(1);
    }
    struct shared *s = mmap(NULL, sizeof *s, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
    if (s == MAP_FAILED) { perror("mmap"); exit(1); }
    close(fd);                  /* 매핑은 fd를 닫아도 남는다 */
    return s;
}

#endif
