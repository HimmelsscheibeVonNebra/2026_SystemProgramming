#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s DIRECTORY\n", argv[0]);
        return 2;
    }

    DIR *dir = opendir(argv[1]);
    if (dir == NULL) {
        perror(argv[1]);
        return 1;
    }

    int status = 0;
    for (;;) {
        /* NULL이 끝인지 오류인지 구분하기 위해 호출 직전에 초기화한다. */
        errno = 0;
        struct dirent *entry = readdir(dir);
        if (entry == NULL) {
            if (errno != 0) {
                perror("readdir");
                status = 1;
            }
            break;
        }

        /* 이름만 출력한다. 이 기초 예제는 .과 ..도 보여 준다. */
        if (puts(entry->d_name) == EOF) {
            perror("stdout");
            status = 1;
            break;
        }
    }

    if (closedir(dir) == -1) {
        perror("closedir");
        status = 1;
    }
    if (fflush(stdout) == EOF) {
        perror("stdout");
        status = 1;
    }
    return status;
}
