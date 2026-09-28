#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* keys로 시작하는 줄만 화면에 남긴다. 실패는 -1, 성공은 찾은 줄 수. */
static int filter_lines(const char *path, const char *const keys[], size_t nkeys)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        return -1;
    }
    char line[256];
    int found = 0;
    while (fgets(line, sizeof line, f) != NULL) {
        for (size_t i = 0; i < nkeys; ++i) {
            if (strncmp(line, keys[i], strlen(keys[i])) == 0) {
                fputs(line, stdout);
                ++found;
            }
        }
    }
    if (ferror(f)) {
        fprintf(stderr, "%s: read error\n", path);
        fclose(f);
        return -1;
    }
    if (fclose(f) == EOF) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        return -1;
    }
    return found;
}

static int first_line(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        return -1;
    }
    char line[256];
    if (fgets(line, sizeof line, f) == NULL) {
        fprintf(stderr, "%s: empty\n", path);
        fclose(f);
        return -1;
    }
    fputs(line, stdout);
    if (fclose(f) == EOF) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        return -1;
    }
    return 0;
}

/* readdir의 NULL은 끝과 오류를 함께 표현하므로 errno로 나눈다. */
static int count_fd(void)
{
    DIR *dir = opendir("/proc/self/fd");
    if (dir == NULL) {
        fprintf(stderr, "/proc/self/fd: %s\n", strerror(errno));
        return -1;
    }
    int count = 0;
    for (;;) {
        errno = 0;
        struct dirent *ent = readdir(dir);
        if (ent == NULL) {
            if (errno != 0) {
                fprintf(stderr, "readdir: %s\n", strerror(errno));
                closedir(dir);
                return -1;
            }
            break;
        }
        if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
            ++count;
    }
    if (closedir(dir) == -1) {
        perror("closedir");
        return -1;
    }
    return count;
}

int main(void)
{
    static const char *const status_keys[] = {"VmRSS:", "Threads:"};
    int status = 0;
    printf("[/proc/self/status]\n");
    if (filter_lines("/proc/self/status", status_keys,
                     sizeof status_keys / sizeof status_keys[0]) < 0)
        status = 1;
    printf("[/proc/loadavg]\n");
    if (first_line("/proc/loadavg") == -1)
        status = 1;
    printf("[/proc/self/fd]\n");
    int count = count_fd();
    if (count < 0) {
        status = 1;
    } else {
        printf("count=%d\n", count);
    }
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return status;
}
