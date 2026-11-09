/* forkpool.c — 부모가 fork로 자식 여러 개에게 일을 나눠 주고,
 *              동시에 도는 수를 제한하고, 종료 코드로 성공·실패를 판정해
 *              실패한 일은 한 번 더 시킨다. (시그널 없이 fork·waitpid만)
 * 사용법: ./forkpool NJOBS MAXPAR        예: ./forkpool 6 2
 *   작업 i는 (i % 3 + 1) × 200 ms 동안 일하고 끝난다.
 *   작업 3은 첫 시도에서 실패(exit 1)하도록 만들어 두었다.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXJOBS  32
#define FAIL_JOB 3
#define MAXTRIES 2

struct job { pid_t pid; int tries; };
static struct job jobs[MAXJOBS];
static struct timespec t0;

static long ms(void)  /* 시작 뒤 경과 시간(ms) */
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (t.tv_sec - t0.tv_sec) * 1000
         + (t.tv_nsec - t0.tv_nsec) / 1000000;
}

static void child_work(int id, int attempt)  /* 자식에서만 실행된다 */
{
    struct timespec d = { 0, (id % 3 + 1) * 200000000L };
    nanosleep(&d, NULL);
    if (id == FAIL_JOB && attempt == 1)
        _exit(1);  /* 첫 시도는 실패로 보고 */
    _exit(0);  /* 성공 */
}

static void start(int id)
{
    jobs[id].tries++;
    fflush(stdout);  /* 버퍼를 비우고 fork */
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }
    if (pid == 0)
        child_work(id, jobs[id].tries); /* 돌아오지 않는다 */
    jobs[id].pid = pid;
    printf("%5ld ms  start job %d (try %d)  pid %d\n",
           ms(), id, jobs[id].tries, (int)pid);
}

static int job_of(pid_t pid, int njobs) /* PID → 작업 번호 */
{
    for (int i = 0; i < njobs; i++)
        if (jobs[i].pid == pid) return i;
    return -1;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s NJOBS MAXPAR\n", argv[0]);
        return 2;
    }
    int njobs = atoi(argv[1]), maxpar = atoi(argv[2]);
    if (njobs < 1 || njobs > MAXJOBS || maxpar < 1) {
        fprintf(stderr, "NJOBS 1..%d, MAXPAR >= 1\n", MAXJOBS);
        return 2;
    }
    clock_gettime(CLOCK_MONOTONIC, &t0);
    int next = 0, running = 0, finished = 0, failed = 0;

    while (finished < njobs) {
        while (running < maxpar && next < njobs) {  /* 빈 자리만큼 시작 */
            start(next++);
            running++;
        }
        int status;
        pid_t pid = waitpid(-1, &status, 0);  /* 아무 자식이나 */
        if (pid < 0) {
            if (errno == EINTR) continue;
            perror("waitpid");
            return 1;
        }
        running--;
        int id = job_of(pid, njobs);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("%5ld ms  done  job %d  pid %d\n", ms(), id, (int)pid);
            finished++;
        } else if (jobs[id].tries < MAXTRIES) {
            printf("%5ld ms  FAIL  job %d  pid %d (exit %d) -> retry\n",
                   ms(), id, (int)pid, WEXITSTATUS(status));
            start(id);
            running++;
        } else {
            printf("%5ld ms  give up job %d\n", ms(), id);
            finished++;
            failed++;
        }
    }
    printf("%5ld ms  all %d jobs finished, %d failed\n", ms(), njobs, failed);
    return failed ? 1 : 0;
}
