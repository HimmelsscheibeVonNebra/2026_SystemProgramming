#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static char file_kind(mode_t mode)
{
    if (S_ISREG(mode)) return '-';
    if (S_ISDIR(mode)) return 'd';
    if (S_ISLNK(mode)) return 'l';
    if (S_ISCHR(mode)) return 'c';
    if (S_ISBLK(mode)) return 'b';
    if (S_ISFIFO(mode)) return 'p';
    if (S_ISSOCK(mode)) return 's';
    return '?';
}

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
    int dfd = dirfd(dir);
    if (dfd == -1) {
        perror("dirfd");
        if (closedir(dir) == -1) perror("closedir");
        return 1;
    }
    int status = 0;
    for (;;) {
        errno = 0;
        struct dirent *ent = readdir(dir);
        if (ent == NULL) {
            if (errno != 0) {
                perror("readdir");
                status = 1;
            }
            break;
        }
        if (strcmp(ent->d_name, ".") == 0 ||
            strcmp(ent->d_name, "..") == 0) continue;
        struct stat st;
        if (fstatat(dfd, ent->d_name, &st,
                    AT_SYMLINK_NOFOLLOW) == -1) {
            perror(ent->d_name);
            status = 1;
            continue;
        }
        if (printf("%c %ju %jd %s\n", file_kind(st.st_mode),
                   (uintmax_t)st.st_ino, (intmax_t)st.st_size,
                   ent->d_name) < 0) {
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
