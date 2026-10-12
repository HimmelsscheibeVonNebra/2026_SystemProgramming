#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* /proc/self/maps는 이 프로세스의 가상 주소 공간을 행 단위로 보여
   준다. 각 행은 "시작-끝 권한 오프셋 장치 아이노드 경로" 형식이다.
   힙([heap])과 스택([stack]) 구역, 공유 라이브러리(.so) 매핑을
   골라 표시하고 전체 구역 수를 센다. /proc가 없는 환경에서는
   unsupported로 알린다. Linux에서 열기에 실패하면 실제 오류를
   보고한다. .so 매핑 수는 라이브러리 파일의 개수와 다르다. */

int main(int argc, char **argv)
{
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 2;
    }
    FILE *fp = fopen("/proc/self/maps", "r");
    if (fp == NULL) {
#ifdef __linux__
        perror("/proc/self/maps");
        return 1;
#else
        printf("maps=unsupported (no /proc on this system)\n");
        if (fflush(stdout) == EOF) {
            perror("stdout");
            return 1;
        }
        return 0;
#endif
    }
    char *line = NULL;
    size_t cap = 0;
    ssize_t len;
    long total = 0, heap = 0, stack = 0, so = 0;
    while ((len = getline(&line, &cap, fp)) != -1) {
        total++;
        if (strstr(line, "[heap]") != NULL) {
            heap++;
            printf("heap_region: %s", line);
        } else if (strstr(line, "[stack]") != NULL) {
            stack++;
            printf("stack_region: %s", line);
        } else if (strstr(line, ".so") != NULL) {
            so++;
        }
    }
    if (ferror(fp)) {
        perror("getline");
        free(line);
        fclose(fp);
        return 1;
    }
    printf("regions=%ld heap_regions=%ld stack_regions=%ld"
           " so_regions=%ld\n", total, heap, stack, so);
    free(line);
    if (fclose(fp) == EOF) {
        perror("fclose");
        return 1;
    }
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
