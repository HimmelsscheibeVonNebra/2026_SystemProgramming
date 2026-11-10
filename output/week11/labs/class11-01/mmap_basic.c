/* mmap_basic.c: 파일 없이 정수 하나를 매핑해 쓰고 읽는 입문 예제 */
#define _GNU_SOURCE             /* Linux에서 MAP_ANONYMOUS 선언 사용 */
#include <stdio.h>
#include <sys/mman.h>

int main(void)
{
    size_t len = sizeof(int);    /* 요청 길이. 실제 매핑은 페이지 단위 */

    int *p = mmap(NULL, len,     /* 주소는 커널이 선택, 길이는 바이트 */
                  PROT_READ | PROT_WRITE,   /* 읽기와 쓰기 허용 */
                  MAP_PRIVATE | MAP_ANONYMOUS, /* 비공개, 파일 없음 */
                  -1, 0);       /* 익명 매핑: fd는 -1, offset은 0 */

    if (p == MAP_FAILED) {       /* 실패는 NULL이 아니라 MAP_FAILED */
        perror("mmap");
        return 1;
    }

    /* 익명 매핑은 0으로 시작한다. 일반 포인터처럼 값을 쓴다. */
    *p = 42;
    printf("*p = %d\n", *p);     /* 같은 주소에서 값을 읽는다. */

    if (munmap(p, len) == -1) {  /* 매핑 해제. 이후 p로 접근하지 않는다. */
        perror("munmap");
        return 1;
    }
    return 0;
}
