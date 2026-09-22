#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static ssize_t scripted_write(int fd, const void *data, size_t count);
#define write scripted_write
#ifdef STUDENT
#include "student/io_helpers.h"
#else
#include "io_helpers.h"
#endif
#undef write
static const unsigned char input[] = "ABCDEFGHIJ";
static unsigned char output[32];
static ssize_t results[8];
static int errors[8];
static size_t steps, calls, sent;
static ssize_t scripted_write(int fd, const void *data, size_t count)
{
    assert(fd == 9);
    assert(calls < steps);
    assert(data == input + sent);
    assert(count == 10 - sent);
    ssize_t result = results[calls];
    int error = errors[calls++];
    if (result < 0) { errno = error; return -1; }
    assert((size_t)result <= count);
    memcpy(output + sent, data, (size_t)result);
    sent += (size_t)result;
    return result;
}
static void reset(const ssize_t *r, const int *e, size_t n)
{
    memset(output, 0, sizeof output);
    memcpy(results, r, n * sizeof *r);
    memcpy(errors, e, n * sizeof *e);
    steps = n; calls = 0; sent = 0;
}
int main(void)
{
    (void)scripted_write; /* Starter may not call write yet. */
    reset((ssize_t[]){10}, (int[]){0}, 1);
    errno = EIO; /* stale errno must not turn success into failure */
    assert(write_all(9, input, 10) == 0);
    assert(calls == 1 && sent == 10 && memcmp(input, output, 10) == 0);
    puts("PASS full write and stale errno");
    reset((ssize_t[]){4,-1,2,4}, (int[]){0,EINTR,0,0}, 4);
    assert(write_all(9, input, 10) == 0);
    assert(calls == 4 && sent == 10 && memcmp(input, output, 10) == 0);
    puts("PASS partial writes and EINTR: requests 10,6,6,4");
    reset((ssize_t[]){-1,-1,10}, (int[]){EINTR,EINTR,0}, 3);
    assert(write_all(9, input, 10) == 0 && calls == 3);
    puts("PASS repeated EINTR");
    reset((ssize_t[]){4,0}, (int[]){0,0}, 2);
    assert(write_all(9, input, 10) == -1 && errno == EIO);
    assert(calls == 2 && sent == 4);
    puts("PASS zero progress becomes EIO");
    reset((ssize_t[]){4,-1}, (int[]){0,ENOSPC}, 2);
    assert(write_all(9, input, 10) == -1 && errno == ENOSPC);
    assert(calls == 2 && sent == 4 && memcmp(input, output, 4) == 0);
    puts("PASS partial data remains after ENOSPC");
    reset((ssize_t[]){-1}, (int[]){EBADF}, 1);
    assert(write_all(9, input, 10) == -1 && errno == EBADF && calls == 1);
    puts("PASS immediate error");
    reset((ssize_t[]){-1}, (int[]){EAGAIN}, 1);
    assert(write_all(9, input, 10) == -1 && errno == EAGAIN && calls == 1);
    puts("PASS EAGAIN is returned, not busy retried");
    reset((ssize_t[]){0}, (int[]){0}, 0);
    assert(write_all(9, input, 0) == 0 && calls == 0);
    puts("PASS empty request makes no write call");
    return 0;
}
