#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "io_helpers.h"

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s FILE BYTE_OFFSET TEXT\n", argv[0]);
        return 2;
    }
    char *end;
    errno = 0;
    intmax_t value = strtoimax(argv[2], &end, 10);
    if (errno != 0 || end == argv[2] || *end != '\0' || value < 0) {
        fprintf(stderr, "Offset must be a nonnegative integer\n");
        return 2;
    }
    off_t offset = (off_t)value;
    if ((intmax_t)offset != value) {
        fprintf(stderr, "Offset is too large for off_t\n");
        return 2;
    }
    /* Existing, seekable file. No O_TRUNC and no O_APPEND. */
    int fd = open(argv[1], O_RDWR);
    if (fd == -1) { perror(argv[1]); return 1; }
    int status = 1;
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        perror("lseek before read"); goto done;
    }
    unsigned char buf[16];
    ssize_t n;
    do { n = read(fd, buf, sizeof buf); } while (n == -1 && errno == EINTR);
    if (n == -1) { perror("read"); goto done; }
    fprintf(stderr, "Before edit: %zd byte(s) from offset %jd\n", n, value);
    if (write_all(STDOUT_FILENO, buf, (size_t)n) == -1) {
        perror("write stdout"); goto done;
    }
    /* read advanced the offset: seek back before replacing bytes. */
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        perror("lseek before write"); goto done;
    }
    if (write_all(fd, argv[3], strlen(argv[3])) == -1) {
        perror("write file"); goto done;
    }
    status = 0;
done:
    if (close(fd) == -1) { perror("close"); status = 1; }
    return status;
}
