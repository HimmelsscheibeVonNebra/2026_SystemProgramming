#include <inttypes.h>
#include <stdio.h>
#include <sys/resource.h>

struct limit_entry {
    const char *name;
    int resource;
};

static int show(const struct limit_entry *entry)
{
    struct rlimit rl;
    if (getrlimit(entry->resource, &rl) == -1) {
        perror(entry->name);
        return 1;
    }
    printf("%-7s", entry->name);
    if (rl.rlim_cur == RLIM_INFINITY)
        printf(" soft=unlimited");
    else
        printf(" soft=%ju", (uintmax_t)rl.rlim_cur);
    if (rl.rlim_max == RLIM_INFINITY)
        printf(" hard=unlimited\n");
    else
        printf(" hard=%ju\n", (uintmax_t)rl.rlim_max);
    return 0;
}

int main(void)
{
    static const struct limit_entry table[] = {
        {"nofile", RLIMIT_NOFILE},
        {"as",     RLIMIT_AS},
        {"stack",  RLIMIT_STACK},
        {"fsize",  RLIMIT_FSIZE},
        {"cpu",    RLIMIT_CPU},
        {"nproc",  RLIMIT_NPROC},
        {"core",   RLIMIT_CORE},
    };
    int status = 0;
    for (size_t i = 0; i < sizeof table / sizeof table[0]; ++i)
        status |= show(&table[i]);
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return status;
}
