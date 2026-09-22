#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static int show(const char *label, int fd)
{
    struct stat st;
    if (fstat(fd, &st) == -1) { perror("fstat"); return 1; }
    printf("%s device=%ju inode=%ju links=%ju size=%jd\n", label,
           (uintmax_t)st.st_dev, (uintmax_t)st.st_ino,
           (uintmax_t)st.st_nlink, (intmax_t)st.st_size);
    return 0;
}
int main(int argc, char **argv)
{
    if (argc != 1) { fprintf(stderr, "usage: %s\n", argv[0]); return 2; }
    char path[] = "/tmp/lsp-file-attrs-XXXXXX";
    int fd = mkstemp(path);
    if (fd == -1) { perror("mkstemp"); return 1; }
    int result = 1, named = 1;
    const char message[] = "hello";
    size_t done = 0;
    while (done < 5) {
        ssize_t n = write(fd, message + done, 5 - done);
        if (n == -1 && errno == EINTR) continue;
        if (n <= 0) { if (n == 0) errno = EIO; perror("write"); goto cleanup; }
        done += (size_t)n;
    }
    printf("path=%s\n", path);
    if (show("before", fd) != 0) goto cleanup;
    if (unlink(path) == -1) { perror("unlink"); goto cleanup; }
    named = 0;
    if (show("after", fd) != 0) goto cleanup;
    struct stat st;
    if (stat(path, &st) != -1 || errno != ENOENT) {
        fputs("expected missing path after unlink\n", stderr); goto cleanup;
    }
    puts("path_lookup=ENOENT");
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) { perror("lseek"); goto cleanup; }
    char buf[6] = {0};
    done = 0;
    while (done < 5) {
        ssize_t n = read(fd, buf + done, 5 - done);
        if (n == -1 && errno == EINTR) continue;
        if (n <= 0) { if (n == 0) errno = EIO; perror("read"); goto cleanup; }
        done += (size_t)n;
    }
    printf("read=%s\n", buf);
    result = 0;
cleanup:
    if (named && unlink(path) == -1) { perror("cleanup unlink"); result = 1; }
    if (close(fd) == -1) { perror("close"); result = 1; }
    if (fflush(stdout) == EOF) { perror("stdout"); result = 1; }
    return result;
}
