#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* getcwd는 상대 경로를 절대 경로로 푸는 기준인 작업 디렉터리를
   알려 준다. 호출 형태는 두 가지다. 호출자가 버퍼를 주거나,
   (NULL, 0)으로 크기를 정해 달라고 요청하거나. chdir은 이
   프로세스의 작업 디렉터리만 바꾼다. 부모 셸은 그대로다. */
static int show_cwd(const char *label)
{
    char buf[4096];
    if (getcwd(buf, sizeof buf) == NULL) {
        perror(label);
        return 1;
    }
    printf("%s=%s\n", label, buf);
    return 0;
}

static int show_cwd_alloc(const char *label)
{
    char *p = getcwd(NULL, 0);
    if (p == NULL) {
        perror(label);
        return 1;
    }
    printf("%s=%s\n", label, p);
    free(p);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc > 2) {
        fprintf(stderr, "usage: %s [directory]\n", argv[0]);
        return 2;
    }
    if (show_cwd("cwd") != 0) {
        return 1;
    }
    if (show_cwd_alloc("cwd_alloc") != 0) {
        return 1;
    }
    if (argc == 2) {
        if (chdir(argv[1]) == -1) {
            perror(argv[1]);
            return 1;
        }
        if (show_cwd("after_chdir") != 0) {
            return 1;
        }
    }
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
