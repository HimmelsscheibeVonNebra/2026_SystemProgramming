#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* /proc/PID/stat 한 줄을 해석해 미니 top 한 줄을 만든다(Linux 전용).
   두 번째 필드 comm은 괄호로 감싼 실행 파일 이름인데 공백이나 ')'가
   들어갈 수 있으므로, 마지막 ')'를 strrchr로 찾은 뒤부터 나눈다.
   utime·stime은 클럭 틱 단위이므로 sysconf(_SC_CLK_TCK)로 나눈다.
   [samples]번 1초 간격으로 읽어 두 표본의 CPU 틱 차이로 %CPU를 낸다. */

struct pstat {
    char comm[64];
    char state;
    long ppid;
    unsigned long minflt, majflt, utime, stime;
    long threads;
    unsigned long vsize;
    long rss_pages;
};

static int read_stat(long pid, struct pstat *ps)
{
    char path[64], line[1024];
    snprintf(path, sizeof path, "/proc/%ld/stat", pid);
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return -1;
    char *ok = fgets(line, sizeof line, fp);
    fclose(fp);
    if (ok == NULL)
        return -1;
    char *open = strchr(line, '(');
    char *close = strrchr(line, ')');
    if (open == NULL || close == NULL || close < open)
        return -1;
    size_t n = (size_t)(close - open - 1);
    if (n >= sizeof ps->comm)
        n = sizeof ps->comm - 1;
    memcpy(ps->comm, open + 1, n);
    ps->comm[n] = '\0';
    /* 필드 3(state)부터: %*를 붙인 필드는 읽고 버린다 */
    int got = sscanf(close + 2,
        "%c %ld %*d %*d %*d %*d %*u %lu %*u %lu %*u %lu %lu "
        "%*d %*d %*d %*d %ld %*d %*u %lu %ld",
        &ps->state, &ps->ppid, &ps->minflt, &ps->majflt,
        &ps->utime, &ps->stime, &ps->threads, &ps->vsize,
        &ps->rss_pages);
    return got == 9 ? 0 : -1;
}

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: %s PID|self [samples 1-60]\n", argv[0]);
        return 2;
    }
    long pid;
    char *end = NULL;
    if (strcmp(argv[1], "self") == 0) {
        pid = (long)getpid();
    } else {
        pid = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || pid < 1) {
            fprintf(stderr, "usage: %s PID|self [samples 1-60]\n", argv[0]);
            return 2;
        }
    }
    long samples = 1;
    if (argc == 3) {
        samples = strtol(argv[2], &end, 10);
        if (end == argv[2] || *end != '\0' || samples < 1 || samples > 60) {
            fprintf(stderr, "usage: %s PID|self [samples 1-60]\n", argv[0]);
            return 2;
        }
    }
    long tick = sysconf(_SC_CLK_TCK);
    long page = sysconf(_SC_PAGESIZE);
    struct pstat prev, cur;
    if (read_stat(pid, &prev) == -1) {
#ifdef __linux__
        fprintf(stderr, "/proc/%ld/stat: cannot read or parse\n", pid);
        return 1;
#else
        printf("stat=unsupported (no /proc on this system)\n");
        return 0;
#endif
    }
    printf("pid=%ld comm=(%s) ppid=%ld threads=%ld clk_tck=%ld\n",
           pid, prev.comm, prev.ppid, prev.threads, tick);
    for (long i = 0; i < samples; i++) {
        if (samples > 1)
            sleep(1);
        if (read_stat(pid, &cur) == -1) {
            printf("gone: pid %ld exited\n", pid);
            break;
        }
        unsigned long d = (cur.utime + cur.stime) - (prev.utime + prev.stime);
        /* 표본이 하나면 비교할 구간이 없으므로 %CPU는 '-'로 둔다 */
        if (samples > 1)
            printf("state=%c cpu=%3.0f%% ", cur.state,
                   100.0 * (double)d / (double)tick);
        else
            printf("state=%c cpu=-    ", cur.state);
        printf("utime=%.2f stime=%.2f vsz_kib=%lu rss_kib=%ld "
               "minflt=%lu majflt=%lu\n",
               (double)cur.utime / tick, (double)cur.stime / tick,
               cur.vsize / 1024, cur.rss_pages * page / 1024,
               cur.minflt, cur.majflt);
        prev = cur;
    }
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}
