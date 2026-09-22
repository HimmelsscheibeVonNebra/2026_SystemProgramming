#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#ifdef STUDENT
#include "student/io_helpers.h"
#else
#include "io_helpers.h"
#endif
/* Caller owns both descriptors. Copy bytes, not metadata. */
static int copy_fd(int in, int out)
{
    unsigned char buf[4096];
    for (;;) {
        ssize_t n = read(in, buf, sizeof buf);
        if (n == 0) return 0;
        if (n == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (write_all(out, buf, (size_t)n) == -1) return -1;
    }
}
int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s INPUT NEW_OUTPUT\n", argv[0]);
        return 2;
    }
    int in = open(argv[1], O_RDONLY);
    if (in == -1) { perror(argv[1]); return 1; }
    int out = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (out == -1) {
        perror(argv[2]);
        if (close(in) == -1) perror("close input");
        return 1;
    }
    int status = 0;
    if (copy_fd(in, out) == -1) {
        perror("copy"); status = 1;
    }
    if (close(in) == -1) { perror("close input"); status = 1; }
    if (close(out) == -1) { perror("close output"); status = 1; }
    return status;
}
