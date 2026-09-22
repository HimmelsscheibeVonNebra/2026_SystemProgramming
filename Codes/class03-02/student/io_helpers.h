#ifndef IO_HELPERS_H
#define IO_HELPERS_H
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
/* TODO: blocking FD, small buffers. Success 0, failure -1 with errno.
 * Handle partial writes, EINTR, zero progress, and count == 0.
 */
static int write_all(int fd, const void *data, size_t count)
{
    (void)fd; (void)data; (void)count;
    errno = ENOSYS;
    return -1;
}
#endif
