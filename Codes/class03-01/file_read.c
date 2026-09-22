#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "io_helpers.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s FILE\n", argv[0]);
        return 2;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) { perror(argv[1]); return 1; }
    unsigned char buf[16]; /* Small on purpose: observe repeated reads. */
    int status = 0;
    for (;;) {
        ssize_t n = read(fd, buf, sizeof buf);
        if (n == 0) break;
        if (n == -1) {
            if (errno == EINTR) continue;
            perror("read"); status = 1; break;
        }
        if (write_all(STDOUT_FILENO, buf, (size_t)n) == -1) {
            perror("write stdout"); status = 1; break;
        }
    }
    if (close(fd) == -1) { perror("close"); status = 1; }
    return status;
}
