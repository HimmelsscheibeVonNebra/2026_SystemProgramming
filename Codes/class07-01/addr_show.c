#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Linux의 일반적인 C 실행 환경에서 변수의 가상 주소를 관찰한다.
   [seconds] 동안 살아 있게 하여 같은 PID의 maps와 대조한다.
   함수 주소의 void * 변환이나 별개 객체의 포인터 대소 비교는
   하지 않는다. 코드 영역은 maps의 실행 권한(x)으로 찾는다. */
int global_init = 7;
int global_zero;

int main(int argc, char **argv)
{
    long seconds = 0;
    if (argc > 2) {
        fprintf(stderr, "usage: %s [seconds 0-600]\n", argv[0]);
        return 2;
    }
    if (argc == 2) {
        char *end = NULL;
        seconds = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' ||
            seconds < 0 || seconds > 600) {
            fprintf(stderr, "usage: %s [seconds 0-600]\n", argv[0]);
            return 2;
        }
    }
    int local = 3;
    static int local_static = 9;
    char *heap1 = malloc(64);
    char *heap2 = malloc(64);
    if (heap1 == NULL || heap2 == NULL) {
        perror("malloc");
        free(heap1);
        free(heap2);
        return 1;
    }
    printf("pid=%ld\n", (long)getpid());
    printf("data=%p bss=%p\n",
           (void *)&global_init, (void *)&global_zero);
    printf("data_static=%p stack=%p\n",
           (void *)&local_static, (void *)&local);
    printf("pointer_on_stack=%p\n", (void *)&heap1);
    printf("heap1=%p heap2=%p\n", (void *)heap1, (void *)heap2);
    printf("observe: cat /proc/%ld/maps\n", (long)getpid());
    printf("hold_seconds=%ld\n", seconds);
    if (fflush(stdout) == EOF) {
        perror("stdout");
        free(heap1);
        free(heap2);
        return 1;
    }
    unsigned int remaining = (unsigned int)seconds;
    while (remaining != 0)
        remaining = sleep(remaining);
    free(heap1);
    free(heap2);
    return 0;
}
