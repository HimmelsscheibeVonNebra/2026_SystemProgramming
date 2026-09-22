#include <stdio.h>

/* Same-environment native int format, exactly three values, no header. */
int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s FILE\n", argv[0]);
        return 2;
    }
    FILE *fp = fopen(argv[1], "wb");
    if (!fp) { perror(argv[1]); return 1; }
    const int values[] = {10, 20, 30};
    size_t n = fwrite(values, sizeof values[0], 3, fp);
    int status = 0;
    if (n != 3) {
        fprintf(stderr, "fwrite: wrote %zu of 3 items\n", n);
        status = 1;
    }
    if (fclose(fp) == EOF) { perror("fclose"); status = 1; }
    return status;
}
