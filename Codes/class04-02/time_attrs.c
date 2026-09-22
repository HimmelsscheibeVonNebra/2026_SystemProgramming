#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static int show(const char *label, int fd)
{
    struct stat st;
    if (fstat(fd, &st) == -1) { perror("fstat"); return 1; }
    printf("%s mode=%04jo atime=%jd mtime=%jd ctime=%jd\n", label,
           (uintmax_t)(st.st_mode & 07777), (intmax_t)st.st_atime,
           (intmax_t)st.st_mtime, (intmax_t)st.st_ctime);
    return 0;
}
int main(int argc, char **argv)
{
    if (argc != 2) { fprintf(stderr, "usage: %s NEW_PATH\n", argv[0]); return 2; }
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd == -1) { perror(argv[1]); return 1; }
    int result = 1;
    struct timespec ts[2] = {{1704067200, 0}, {1704153600, 0}};
    if (futimens(fd, ts) == -1) { perror("futimens"); goto done; }
    if (show("after_futimens", fd) != 0) goto done;
    if (fchmod(fd, 0640) == -1) { perror("fchmod"); goto done; }
    if (show("after_fchmod", fd) != 0) goto done;
    result = 0;
done:
    if (close(fd) == -1) { perror("close"); result = 1; }
    if (fflush(stdout) == EOF) { perror("stdout"); result = 1; }
    return result;
}
