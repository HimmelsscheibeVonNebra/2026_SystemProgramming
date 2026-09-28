#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>

/* sysconf는 실패해도 errno를 바꾸지 않을 수 있으므로
   호출 직전에 0으로 만들고 -1의 의미를 나누어 본다. */
static int probe(const char *label, int name)
{
    errno = 0;
    long value = sysconf(name);
    if (value == -1) {
        if (errno != 0) {
            perror(label);
            return 1;
        }
        printf("%s=unlimited\n", label);
        return 0;
    }
    printf("%s=%ld\n", label, value);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 2;
    }
    struct utsname u;
    if (uname(&u) == -1) {
        perror("uname");
        return 1;
    }
    printf("sysname=%s\n", u.sysname);
    printf("nodename=%s\n", u.nodename);
    printf("release=%s\n", u.release);
    printf("version=%s\n", u.version);
    printf("machine=%s\n", u.machine);
    int status = 0;
    status |= probe("cpus_online", _SC_NPROCESSORS_ONLN);
    status |= probe("pagesize", _SC_PAGESIZE);
    status |= probe("clk_tck", _SC_CLK_TCK);
    status |= probe("open_max", _SC_OPEN_MAX);
    status |= probe("arg_max", _SC_ARG_MAX);
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return status;
}
