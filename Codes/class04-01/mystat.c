#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdio.h>
#include <sys/stat.h>

static const char *file_kind(mode_t mode)
{
    if (S_ISREG(mode)) return "regular";
    if (S_ISDIR(mode)) return "directory";
    if (S_ISLNK(mode)) return "symlink";
    if (S_ISCHR(mode)) return "char-device";
    if (S_ISBLK(mode)) return "block-device";
    if (S_ISFIFO(mode)) return "fifo";
    if (S_ISSOCK(mode)) return "socket";
    return "unknown";
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s PATH\n", argv[0]);
        return 2;
    }
    struct stat st;
    if (lstat(argv[1], &st) == -1) {
        perror(argv[1]);
        return 1;
    }
    printf("type=%s\n", file_kind(st.st_mode));
    printf("device=%ju inode=%ju links=%ju\n",
           (uintmax_t)st.st_dev, (uintmax_t)st.st_ino,
           (uintmax_t)st.st_nlink);
    printf("mode=%04jo uid=%ju gid=%ju size=%jd\n",
           (uintmax_t)(st.st_mode & 07777),
           (uintmax_t)st.st_uid, (uintmax_t)st.st_gid,
           (intmax_t)st.st_size);
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
