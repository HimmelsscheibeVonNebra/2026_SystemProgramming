#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s FILE\n", argv[0]);
        return 2;
    }
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd == -1) { perror(argv[1]); return 1; }
    if (close(fd) == -1) { perror("close"); return 1; }
    return 0;
}
