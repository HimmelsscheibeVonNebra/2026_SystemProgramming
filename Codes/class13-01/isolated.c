/* isolated.c — fork 뒤에는 변수를 고쳐도 서로 보이지 않는다 (IPC가 필요한 이유) */
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int counter = 100;                      /* 전역 변수 (데이터 영역) */

int main(void)
{
    int *heap = malloc(sizeof *heap);   /* 힙 */
    if (heap == NULL) { perror("malloc"); return 1; }
    *heap = 200;

    pid_t pid = fork();
    if (pid == -1) { perror("fork"); return 1; }

    if (pid == 0) {                     /* 자식: 두 값을 고친다 */
        counter += 1;
        *heap += 1;
        printf("[자식 %d] counter=%d (%p)  *heap=%d (%p)\n", (int)getpid(),
               counter, (void *)&counter, *heap, (void *)heap);
        return 42;                      /* 부모에게 가는 것은 이 8비트뿐 */
    }

    int status;
    waitpid(pid, &status, 0);           /* 자식이 고친 뒤에 확인한다 */
    printf("[부모 %d] counter=%d (%p)  *heap=%d (%p)\n", (int)getpid(),
           counter, (void *)&counter, *heap, (void *)heap);
    printf("[부모 %d] 자식에게서 받은 것: 종료 상태 %d\n", (int)getpid(),
           WEXITSTATUS(status));
    return 0;
}
