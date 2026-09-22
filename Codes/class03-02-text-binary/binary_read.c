#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s FILE\n", argv[0]);
        return 2;
    }
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) { perror(argv[1]); return 1; }
    int values[3];
    int status = 1;
    size_t n = fread(values, sizeof values[0], 3, fp);
    if (n != 3) {
        if (ferror(fp)) perror("fread");
        else fprintf(stderr, "EOF: read %zu of 3 complete items\n", n);
        goto done;
    }
    /* This teaching format has exactly three ints; reject trailing bytes. */
    int extra = fgetc(fp);
    if (ferror(fp)) { perror("fgetc"); goto done; }
    if (extra != EOF) {
        fprintf(stderr, "Unexpected data after the third int\n");
        goto done;
    }
    if (printf("%d %d %d\n", values[0], values[1], values[2]) < 0) {
        perror("printf"); goto done;
    }
    status = 0;
done:
    if (fclose(fp) == EOF) { perror("fclose"); status = 1; }
    if (fflush(stdout) == EOF) { perror("stdout"); status = 1; }
    return status;
}
