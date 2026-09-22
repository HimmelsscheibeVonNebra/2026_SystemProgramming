#ifndef IO_HELPERS_H
#define IO_HELPERS_H
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
/* Blocking descriptors. Retry interruptions and advance after partial writes. */
static int write_all(int fd, const void *data, size_t count)
{
    const unsigned char *p = data;
    while (count > 0) {
        ssize_t n = write(fd, p, count);
        if (n == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) { errno = EIO; return -1; }
        p += (size_t)n;
        count -= (size_t)n;
    }
    return 0;
}
#endif
