#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 2) { fprintf(stderr, "usage: %s NEW_PATH\n", argv[0]); return 2; }
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd == -1) { perror(argv[1]); return 1; }
    int result = 1;
    if (lseek(fd, (off_t)1024 * 1024, SEEK_SET) == (off_t)-1) {
        perror("lseek"); goto done;
    }
    ssize_t n;
    do { n = write(fd, "X", 1); } while (n == -1 && errno == EINTR);
    if (n != 1) {
        if (n == 0) errno = EIO;
        perror("write"); goto done;
    }
    struct stat st;
    if (fstat(fd, &st) == -1) { perror("fstat"); goto done; }
    printf("size=%jd blocks=%jd blksize=%jd\n", (intmax_t)st.st_size,
           (intmax_t)st.st_blocks, (intmax_t)st.st_blksize);
    result = 0;
done:
    if (close(fd) == -1) { perror("close"); result = 1; }
    if (fflush(stdout) == EOF) { perror("stdout"); result = 1; }
    return result;
}
