#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static const char *kind(mode_t m)
{
    if (S_ISREG(m)) return "regular";
    if (S_ISDIR(m)) return "directory";
    if (S_ISLNK(m)) return "symlink";
    if (S_ISCHR(m)) return "char-device";
    if (S_ISBLK(m)) return "block-device";
    if (S_ISFIFO(m)) return "fifo";
    if (S_ISSOCK(m)) return "socket";
    return "unknown";
}
static void permissions(mode_t mode, char out[10])
{
    const mode_t bits[9] = {
        S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP,
        S_IROTH, S_IWOTH, S_IXOTH
    };
    for (int i = 0; i < 9; ++i)
        out[i] = (mode & bits[i]) ? "rwx"[i % 3] : '-';
    if (mode & S_ISUID) out[2] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID) out[5] = (mode & S_IXGRP) ? 's' : 'S';
    if (mode & S_ISVTX) out[8] = (mode & S_IXOTH) ? 't' : 'T';
    out[9] = '\0';
}
int main(int argc, char **argv)
{
    int follow = argc == 3 && strcmp(argv[1], "--follow") == 0;
    if (!(argc == 2 || follow)) {
        fprintf(stderr, "usage: %s [--follow] PATH\n", argv[0]);
        return 2;
    }
    const char *path = argv[follow ? 2 : 1];
    struct stat st;
    if ((follow ? stat(path, &st) : lstat(path, &st)) == -1) {
        perror(path); return 1;
    }
    char mode_text[10];
    permissions(st.st_mode, mode_text);
    printf("type=%s\n", kind(st.st_mode));
    printf("device=%ju inode=%ju links=%ju\n", (uintmax_t)st.st_dev,
           (uintmax_t)st.st_ino, (uintmax_t)st.st_nlink);
    printf("mode=%04jo rwx=%s uid=%ju gid=%ju\n",
           (uintmax_t)(st.st_mode & 07777), mode_text,
           (uintmax_t)st.st_uid, (uintmax_t)st.st_gid);
    printf("size=%jd blocks=%jd blksize=%jd\n", (intmax_t)st.st_size,
           (intmax_t)st.st_blocks, (intmax_t)st.st_blksize);
#ifdef __APPLE__
    struct timespec at = st.st_atimespec, mt = st.st_mtimespec, ct = st.st_ctimespec;
#else
    struct timespec at = st.st_atim, mt = st.st_mtim, ct = st.st_ctim;
#endif
    printf("atime=%jd.%09ld\n", (intmax_t)at.tv_sec, at.tv_nsec);
    printf("mtime=%jd.%09ld\n", (intmax_t)mt.tv_sec, mt.tv_nsec);
    printf("ctime=%jd.%09ld\n", (intmax_t)ct.tv_sec, ct.tv_nsec);
    if (fflush(stdout) == EOF) { perror("stdout"); return 1; }
    return 0;
}
