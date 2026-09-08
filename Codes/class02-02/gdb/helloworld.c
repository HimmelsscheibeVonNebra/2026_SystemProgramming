#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int count = 0; // 반복 횟수 초기화
    while (1) {
        count++; // 이 줄에 중단점 설정
        printf("count=%d\n", count);
        fflush(stdout);
        sleep(2);
    }
}
