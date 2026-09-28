#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

static void print_limit(const char *label)
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
        perror("getrlimit");
        exit(1);
    }
    if (rl.rlim_cur == RLIM_INFINITY)
        printf("%s soft=unlimited", label);
    else
        printf("%s soft=%ju", label, (uintmax_t)rl.rlim_cur);
    if (rl.rlim_max == RLIM_INFINITY)
        printf(" hard=unlimited\n");
    else
        printf(" hard=%ju\n", (uintmax_t)rl.rlim_max);
}

int main(int argc, char **argv)
{
    int raise_requested = argc == 3 && strcmp(argv[1], "--raise") == 0;
    if (!(argc == 1 || raise_requested)) {
        fprintf(stderr, "usage: %s [--raise N]\n", argv[0]);
        return 2;
    }
    print_limit("before");

    if (raise_requested) {
        char *end = NULL;
        errno = 0;
        unsigned long long want = strtoull(argv[2], &end, 10);
        if (end == argv[2] || *end != '\0' || errno != 0) {
            fprintf(stderr, "invalid number: %s\n", argv[2]);
            return 2;
        }
        struct rlimit rl;
        if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
            perror("getrlimit");
            return 1;
        }
        /* 일반 사용자는 hard까지 상향할 수 있다. */
        rlim_t target = (rlim_t)want;
        if (rl.rlim_max != RLIM_INFINITY && target > rl.rlim_max)
            target = rl.rlim_max;
        struct rlimit next = { target, rl.rlim_max };
        if (setrlimit(RLIMIT_NOFILE, &next) == -1) {
            perror("setrlimit");
            return 1;
        }
        print_limit("after ");
    }

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
        perror("getrlimit");
        return 1;
    }
    rlim_t cap = rl.rlim_cur == RLIM_INFINITY ? 65536 : rl.rlim_cur;
    int *fds = malloc(sizeof *fds * (size_t)cap);
    if (fds == NULL) {
        perror("malloc");
        return 1;
    }
    int opened = 0;
    while (opened < (int)cap) {
        int fd = open("/dev/null", O_RDONLY);
        if (fd == -1) {
            if (errno == EMFILE)
                break;
            perror("open");
            while (opened > 0)
                close(fds[--opened]);
            free(fds);
            return 1;
        }
        fds[opened++] = fd;
    }
    printf("opened=%d stopped=", opened);
    if (opened == (int)cap)
        printf("cap\n");
    else
        printf("EMFILE\n");
    int close_failed = 0;
    while (opened > 0)
        if (close(fds[--opened]) == -1)
            close_failed = 1;
    free(fds);
    if (close_failed) {
        perror("close");
        return 1;
    }
    printf("closed all descriptors\n");
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
