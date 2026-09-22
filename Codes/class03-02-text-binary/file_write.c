#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "io_helpers.h"

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s truncate|append FILE TEXT\n", argv[0]);
        return 2;
    }
    int flags = O_WRONLY | O_CREAT;
    if (strcmp(argv[1], "truncate") == 0) flags |= O_TRUNC;
    else if (strcmp(argv[1], "append") == 0) flags |= O_APPEND;
    else { fprintf(stderr, "Unknown mode: %s\n", argv[1]); return 2; }

    int fd = open(argv[2], flags, 0666);
    if (fd == -1) { perror(argv[2]); return 1; }
    int status = 0;
    /* Store exactly TEXT bytes, with no added newline or terminating NUL. */
    if (write_all(fd, argv[3], strlen(argv[3])) == -1) {
        perror("write");
        status = 1;
    }
    /* On Linux, report close errors without retrying close(). */
    if (close(fd) == -1) { perror("close"); status = 1; }
    return status;
}
