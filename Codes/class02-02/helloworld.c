#include <stdio.h>
#include <unistd.h>

int main(void)
{
    /* 1은 항상 참이므로 아래 작업을 계속 반복합니다. */
    while (1) {
        printf("Hello, world!\n");
        /* 서비스 로그에도 출력이 바로 보이도록 합니다. */
        fflush(stdout);
        /* 2초간 기다린 뒤 다음 반복을 실행합니다. */
        sleep(2);
    }
}
