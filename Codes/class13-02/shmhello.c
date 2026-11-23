/* shmhello.c — 이름 있는 공유 메모리에 글을 써 두고, 다른 프로세스가 읽는다
 *   ./shmhello write TEXT   만들고(없으면) 글을 써 둔다
 *   ./shmhello read         붙여서 읽는다
 *   ./shmhello rm           이름을 지운다
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/w13_hello"
#define SIZE 4096

int main(int argc, char *argv[])
{
    const char *cmd = argc > 1 ? argv[1] : "";

    if (!strcmp(cmd, "write") && argc == 3) {
        int fd = shm_open(NAME, O_CREAT | O_RDWR, 0600);   /* ① 이름으로 연다 */
        if (fd == -1) { perror("shm_open"); return 1; }
        if (ftruncate(fd, SIZE) == -1) { perror("ftruncate"); return 1; }  /* ② */
        char *p = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (p == MAP_FAILED) { perror("mmap"); return 1; } /* ③ 붙인다 */
        close(fd);                      /* 붙인 뒤에는 fd가 필요 없다 */
        snprintf(p, SIZE, "%s", argv[2]);       /* 그냥 메모리에 쓴다 */
        printf("[%d] 씀: \"%s\" (주소 %p)\n", (int)getpid(), p, (void *)p);
        munmap(p, SIZE);
    } else if (!strcmp(cmd, "read") && argc == 2) {
        int fd = shm_open(NAME, O_RDONLY, 0);
        if (fd == -1) { perror("shm_open"); return 1; }
        char *p = mmap(NULL, SIZE, PROT_READ, MAP_SHARED, fd, 0);
        if (p == MAP_FAILED) { perror("mmap"); return 1; }
        close(fd);
        printf("[%d] 읽음: \"%s\" (주소 %p)\n", (int)getpid(), p, (void *)p);
        munmap(p, SIZE);
    } else if (!strcmp(cmd, "rm") && argc == 2) {
        if (shm_unlink(NAME) == -1) { perror("shm_unlink"); return 1; }
        printf("지움: %s\n", NAME);
    } else {
        fprintf(stderr, "사용법: %s write TEXT | read | rm\n", argv[0]);
        return 2;
    }
    return 0;
}
