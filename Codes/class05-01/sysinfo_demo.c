/* sysinfo(2)는 Linux 전용 인터페이스다.
   이 파일에서는 조건부 컴파일로 지원 환경을 구분한다. */
#if defined(__linux__)

#include <errno.h>
#include <stdio.h>
#include <sys/sysinfo.h>

int main(void)
{
    struct sysinfo si;
    if (sysinfo(&si) == -1) {
        perror("sysinfo");
        return 1;
    }
    printf("uptime=%ld sec\n", si.uptime);
    /* loads는 65536을 1로 하는 고정소수점 값이다. */
    printf("loadavg=%.2f %.2f %.2f\n",
           (double)si.loads[0] / (1 << SI_LOAD_SHIFT),
           (double)si.loads[1] / (1 << SI_LOAD_SHIFT),
           (double)si.loads[2] / (1 << SI_LOAD_SHIFT));
    printf("procs=%u\n", si.procs);
    /* 램 크기는 필드 값에 mem_unit을 곱해 바이트로 환산한다. */
    printf("totalram=%lu bytes\n", si.totalram * (unsigned long)si.mem_unit);
    printf("freeram=%lu bytes\n", si.freeram * (unsigned long)si.mem_unit);
    printf("mem_unit=%u\n", si.mem_unit);
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return 0;
}

#else /* not __linux__ */

#include <stdio.h>

int main(void)
{
    puts("sysinfo(2) is Linux-only: not available on this system");
    return 1;
}

#endif
