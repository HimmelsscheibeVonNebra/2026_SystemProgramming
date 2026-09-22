#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static int show_fd(const char *label, int fd)
{
    struct stat st;
    if (fstat(fd, &st) == -1) { perror("fstat"); return 1; }
    printf("%s mode=%04jo\n", label, (uintmax_t)(st.st_mode & 07777));
    return 0;
}
static int create_file(int dir, const char *name, mode_t mask, int change)
{
    mode_t old = umask(mask);
    int fd = openat(dir, name, O_WRONLY | O_CREAT | O_EXCL, 0666);
    int saved = errno;
    umask(old);
    if (fd == -1) { errno = saved; perror(name); return 1; }
    int result = show_fd(name, fd);
    if (result == 0 && change) {
        if (fchmod(fd, 0640) == -1) { perror("fchmod"); result = 1; }
        else result = show_fd("after_fchmod", fd);
    }
    if (close(fd) == -1) { perror("close file"); result = 1; }
    return result;
}
int main(int argc, char **argv)
{
    if (argc != 2) { fprintf(stderr, "usage: %s DIRECTORY\n", argv[0]); return 2; }
    int dir = open(argv[1], O_RDONLY | O_DIRECTORY);
    if (dir == -1) { perror(argv[1]); return 1; }
    int result = create_file(dir, "mask022.txt", 0022, 1);
    if (result == 0) result = create_file(dir, "mask077.txt", 0077, 0);
    if (result == 0) {
        mode_t old = umask(0027);
        int rc = mkdirat(dir, "groupdir", 0777);
        int saved = errno;
        umask(old);
        if (rc == -1) { errno = saved; perror("mkdirat"); result = 1; }
        else {
            struct stat st;
            if (fstatat(dir, "groupdir", &st, AT_SYMLINK_NOFOLLOW) == -1) {
                perror("fstatat"); result = 1;
            } else printf("groupdir mode=%04jo\n", (uintmax_t)(st.st_mode & 07777));
        }
    }
    if (close(dir) == -1) { perror("close directory"); result = 1; }
    if (fflush(stdout) == EOF) { perror("stdout"); result = 1; }
    return result;
}
