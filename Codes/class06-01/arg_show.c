#include <stdio.h>

/* argv는 널 종료 문자열의 포인터 배열이다.
   argv[argc]는 NULL이 보장되므로 개수와 널 두 가지로
   끝을 확인할 수 있다. argv[0]은 관례상 프로그램 이름이지만
   호출 쪽이 무엇이든 넣을 수 있으므로 보증으로 쓰지 않는다. */
int main(int argc, char **argv)
{
    printf("argc=%d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]=%s\n", i, argv[i]);
    }
    printf("argv[%d]=%p\n", argc, (void *)argv[argc]);
    /* exec -a 등으로 argv[0]을 바꾼 실행과 비교할 때
       문자열 길이도 함께 출력하면 길이 검증의 출발점이 된다. */
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
