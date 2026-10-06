#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 환경은 "NAME=value" 문자열의 널 종료 포인터 배열이다.
   environ 전역 변수로 순회하고 getenv·setenv·unsetenv로
   조회·변경하는 계약을 관찰한다. */
extern char **environ;

static int count_env(void)
{
    int n = 0;
    for (char **e = environ; *e != NULL; e++) {
        n++;
    }
    return n;
}

static void dump_env(void)
{
    for (char **e = environ; *e != NULL; e++) {
        printf("%s\n", *e);
    }
}

static int find_env(const char *name)
{
    const char *value = getenv(name);
    if (value == NULL) {
        printf("%s=(unset)\n", name);
    } else {
        printf("%s=%s\n", name, value);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc > 3) {
        fprintf(stderr, "usage: %s [--find NAME | --set NAME=VALUE"
                        " | --unset NAME]\n", argv[0]);
        return 2;
    }
    if (argc == 1) {
        printf("env_count=%d\n", count_env());
        dump_env();
        return 0;
    }
    if (strcmp(argv[1], "--find") == 0 && argc == 3) {
        return find_env(argv[2]);
    }
    /* setenv의 세 번째 인자 1은 같은 이름이 있으면 덮어쓴다는 뜻이다.
       바꾼 뒤 개수와 값을 다시 조회해 변경이 이 프로세스 안에서만
       일어났음을 관찰한다. 부모 셸에는 영향을 주지 않는다. */
    if (strcmp(argv[1], "--set") == 0 && argc == 3) {
        char *eq = strchr(argv[2], '=');
        if (eq == NULL || eq == argv[2]) {
            fprintf(stderr, "--set expects NAME=VALUE\n");
            return 2;
        }
        size_t name_len = (size_t)(eq - argv[2]);
        char name[name_len + 1];
        memcpy(name, argv[2], name_len);
        name[name_len] = '\0';
        if (setenv(name, eq + 1, 1) == -1) {
            perror("setenv");
            return 1;
        }
        printf("env_count=%d\n", count_env());
        return find_env(name);
    }
    if (strcmp(argv[1], "--unset") == 0 && argc == 3) {
        if (unsetenv(argv[2]) == -1) {
            perror("unsetenv");
            return 1;
        }
        printf("env_count=%d\n", count_env());
        return find_env(argv[2]);
    }
    fprintf(stderr, "usage: %s [--find NAME | --set NAME=VALUE"
                    " | --unset NAME]\n", argv[0]);
    return 2;
}
