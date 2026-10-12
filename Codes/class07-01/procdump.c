#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

/* 프로세스의 상태와 자원을 한 보고서로 묶는다. 표준 API인
   getrlimit·getrusage로 자원 한계와 사용량을 얻고, Linux에서는
   /proc/self/status와 /proc/self/fd가 상태·메모리·FD 정보를
   보충한다. /proc가 없으면 unavailable로 알린다. */

static void show_rlimit(int resource, const char *name)
{
    struct rlimit lim;
    if (getrlimit(resource, &lim) == -1) {
        perror("getrlimit");
        exit(1);
    }
    for (int hard = 0; hard <= 1; hard++) {
        rlim_t v = hard ? lim.rlim_max : lim.rlim_cur;
        if (v == RLIM_INFINITY)
            printf("%s_%s=unlimited\n", name, hard ? "hard" : "soft");
        else
            printf("%s_%s=%llu\n", name, hard ? "hard" : "soft",
                   (unsigned long long)v);
    }
}

int main(int argc, char **argv)
{
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 2;
    }
    printf("pid=%ld ppid=%ld\n",
           (long)getpid(), (long)getppid());
    printf("uid=%ld euid=%ld\n",
           (long)getuid(), (long)geteuid());
    show_rlimit(RLIMIT_NOFILE, "rlimit_nofile");
    show_rlimit(RLIMIT_AS, "rlimit_as");
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) == -1) {
        perror("getrusage");
        return 1;
    }
    /* suseconds_t는 시스템마다 int·long이 달라 (long)으로 맞춘다 */
    printf("cpu_user=%ld.%06ld cpu_sys=%ld.%06ld\n",
           (long)ru.ru_utime.tv_sec, (long)ru.ru_utime.tv_usec,
           (long)ru.ru_stime.tv_sec, (long)ru.ru_stime.tv_usec);
    /* 현재 RSS와 구분되는 실행 중 최대 상주 메모리다. */
#ifdef __APPLE__
    printf("maxrss=%ld unit=bytes\n", ru.ru_maxrss);
#else
    printf("maxrss=%ld unit=KiB\n", ru.ru_maxrss);
#endif
    FILE *fp = fopen("/proc/self/status", "r");
    if (fp == NULL) {
#ifdef __linux__
        perror("/proc/self/status");
        return 1;
#else
        printf("proc_status=unavailable (no /proc)\n");
#endif
    } else {
        char *line = NULL;
        size_t cap = 0;
        while (getline(&line, &cap, fp) != -1) {
            if (strncmp(line, "State:", 6) == 0 ||
                strncmp(line, "VmSize:", 7) == 0 ||
                strncmp(line, "VmRSS:", 6) == 0 ||
                strncmp(line, "Threads:", 8) == 0) {
                char *colon = strchr(line, ':');
                *colon = '\0';
                char *value = colon + 1;
                while (*value == ' ' || *value == '\t')
                    value++;
                value[strcspn(value, "\n")] = '\0';
                printf("status_%s=%s\n", line, value);
            }
        }
        int failed = ferror(fp);
        if (failed) perror("getline");
        free(line);
        if (fclose(fp) == EOF) {
            perror("fclose");
            failed = 1;
        }
        if (failed) return 1;
    }
    DIR *dir = opendir("/proc/self/fd");
    if (dir == NULL) {
#ifdef __linux__
        perror("/proc/self/fd");
        return 1;
#else
        printf("fd_count=unavailable (no /proc)\n");
#endif
    } else {
        /* 스캔 자체가 FD 하나를 쓰므로 Linux에서는 표준 3개보다
           하나 더 세는 것이 정상이다. 관찰도 증거에 영향을 준다. */
        struct dirent *ent;
        long count = 0;
        errno = 0;
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] != '.')
                count++;
        }
        int failed = errno != 0;
        if (failed) perror("readdir");
        printf("fd_count=%ld includes_scan_fd=yes\n", count);
        if (closedir(dir) == -1) {
            perror("closedir");
            failed = 1;
        }
        if (failed) return 1;
    }
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
