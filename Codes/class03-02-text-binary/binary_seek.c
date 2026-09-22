#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>

static int parse_number(const char *text, intmax_t *value)
{
    char *end;
    errno = 0;
    *value = strtoimax(text, &end, 10);
    return errno == 0 && end != text && *end == '\0';
}

/* Select or update one of the three ints in the teaching file. */
int main(int argc, char *argv[])
{
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Usage: %s FILE INDEX [NEW_VALUE]\n", argv[0]);
        return 2;
    }
    intmax_t index, replacement = 0;
    if (!parse_number(argv[2], &index) || index < 0 || index > 2) {
        fprintf(stderr, "INDEX must be 0, 1 or 2\n"); return 2;
    }
    if (argc == 4 && (!parse_number(argv[3], &replacement) ||
                     replacement < INT_MIN || replacement > INT_MAX)) {
        fprintf(stderr, "NEW_VALUE must fit in int\n"); return 2;
    }
    FILE *fp = fopen(argv[1], argc == 4 ? "r+b" : "rb");
    if (!fp) { perror(argv[1]); return 1; }
    int status = 1;
    int values[3];
    /* Validate before editing so a malformed file remains unchanged. */
    if (fread(values, sizeof values[0], 3, fp) != 3) {
        if (ferror(fp)) perror("fread validation");
        else fprintf(stderr, "Expected three complete ints\n");
        goto done;
    }
    int extra = fgetc(fp);
    if (ferror(fp)) { perror("fgetc"); goto done; }
    if (extra != EOF) {
        fprintf(stderr, "Unexpected trailing data\n"); goto done;
    }
    long offset = (long)index * (long)sizeof(int);
    if (fseek(fp, offset, SEEK_SET) != 0) { perror("fseek"); goto done; }
    int value;
    if (fread(&value, sizeof value, 1, fp) != 1) {
        if (ferror(fp)) perror("fread selected item");
        else fprintf(stderr, "Selected item is incomplete\n");
        goto done;
    }
    if (argc == 4) {
        /* Reposition after reading, before switching to writing. */
        if (fseek(fp, offset, SEEK_SET) != 0) { perror("fseek"); goto done; }
        value = (int)replacement;
        if (fwrite(&value, sizeof value, 1, fp) != 1) {
            fprintf(stderr, "fwrite: failed to write the selected item\n");
            goto done;
        }
    }
    if (printf("%d\n", value) < 0) { perror("printf"); goto done; }
    status = 0;
done:
    if (fclose(fp) == EOF) { perror("fclose"); status = 1; }
    if (fflush(stdout) == EOF) { perror("stdout"); status = 1; }
    return status;
}
