/* pipemsg.c — 부모가 파이프로 보내고 자식이 읽는다.
 * 끝점 닫기와 EOF, 읽는 쪽이 없을 때의 SIGPIPE·EPIPE, 용량을 확인한다.
 * 사용법: ./pipemsg ok | leak | sigpipe | epipe | cap
 */
#define _GNU_SOURCE                 /* F_GETPIPE_SZ를 쓰려고 (Linux 전용) */
#include <errno.h>                  /* errno, EPIPE, EAGAIN */
#include <fcntl.h>                  /* fcntl, O_NONBLOCK, F_GETPIPE_SZ */
#include <limits.h>                 /* PIPE_BUF */
#include <signal.h>                 /* signal, SIGPIPE, SIG_IGN */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>               /* waitpid, WEXITSTATUS */
#include <unistd.h>                 /* pipe, fork, read, write, close, usleep */

static void die(const char *what)   /* 실패한 호출 이름과 errno 메시지를 내고 끝낸다 */
{
    perror(what);
    exit(1);
}

/* 자식: 읽기 끝에서 EOF까지 읽는다. leak이면 쓰기 끝을 닫지 않는다. */
static void child_reader(int fd[2], int leak)
{
    char buf[64];                   /* NUL로 끝나지 않는다: 길이 n과 함께 쓴다 */
    ssize_t n;                      /* read의 반환값 */

    if (!leak)                      /* leak 모드는 다음 줄만 건너뛴다 */
        close(fd[1]);           /* 내가 쓰지 않는 쓰기 끝을 닫는다 */
    while ((n = read(fd[0], buf, sizeof buf)) > 0)  /* 비면 잠들고, 쓰기 끝이 0개면 0 */
        printf("[자식] read %zd바이트: %.*s", n, (int)n, buf);  /* %.*s: n바이트만 */
    if (n == -1)                    /* -1은 오류, 0은 EOF */
        die("read");
    printf("[자식] read 0 → EOF, 종료\n");
    exit(0);                        /* 부모의 코드로 돌아가지 않는다 */
}

/* ok · leak: 부모가 두 번 쓰고 쓰기 끝을 닫은 뒤 자식을 기다린다. */
static int run_msg(int leak)
{
    int fd[2];                      /* fd[0]: 읽기 끝, fd[1]: 쓰기 끝 */
    const char *msg[] = { "첫 번째 줄\n", "두 번째 줄\n" };  /* 각 15바이트 */

    if (pipe(fd) == -1)             /* fork보다 먼저: 자식이 물려받아야 한다 */
        die("pipe");
    pid_t pid = fork();         /* 끝점 두 개가 자식에게 복사된다 */
    if (pid == -1)
        die("fork");
    if (pid == 0)                   /* 자식은 여기로 들어가 돌아오지 않는다 */
        child_reader(fd, leak);

    close(fd[0]);               /* 부모는 읽지 않는다 */
    for (int i = 0; i < 2; i++) {   /* 두 줄을 0.2초 간격으로 보낸다 */
        if (write(fd[1], msg[i], strlen(msg[i])) == -1)
            die("write");
        usleep(200 * 1000);         /* 안 쉬면 두 줄이 read 한 번에 올 수 있다 */
    }
    close(fd[1]);               /* 부모의 쓰기 끝을 닫는다 */
    printf("[부모] 쓰기 끝을 닫고 자식을 기다린다\n");

    int st;                         /* 자식의 종료 상태 */
    if (waitpid(pid, &st, 0) == -1)  /* close(fd[1]) 뒤에 기다린다. 바꾸면 교착 */
        die("waitpid");
    printf("[부모] 자식 종료 코드 %d\n", WEXITSTATUS(st));
    return 0;
}

/* sigpipe · epipe: 읽기 끝이 하나도 없는 파이프에 쓴다. */
static int run_noreader(int ignore)
{
    int fd[2];                      /* fd[0]: 읽기 끝, fd[1]: 쓰기 끝 */

    if (pipe(fd) == -1)
        die("pipe");
    close(fd[0]);               /* 읽기 끝이 하나도 남지 않는다 */
    if (ignore)                     /* epipe 모드에서만 */
        signal(SIGPIPE, SIG_IGN);   /* 시그널 대신 오류 값으로 받는다 */
    printf("읽는 쪽 없이 write를 시도한다\n");
    if (write(fd[1], "x", 1) == -1) {  /* sigpipe: 여기서 죽는다 (141) */
        printf("write 실패: %s (errno %d)\n", strerror(errno), errno);
        return 1;                   /* epipe: Broken pipe를 보고 끝난다 */
    }
    printf("이 줄은 출력되지 않는다\n");
    return 0;
}

/* cap: 아무도 읽지 않을 때 몇 바이트까지 들어가는지 센다. */
static int run_cap(void)
{
    int fd[2];                      /* 읽기 끝은 열려 있지만 아무도 읽지 않는다 */
    char block[4096];               /* 한 번에 쓸 덩어리 (PIPE_BUF 크기) */
    long total = 0;                 /* 들어간 바이트 수 */
    ssize_t n;                      /* write의 반환값 */

    if (pipe(fd) == -1)
        die("pipe");
    memset(block, 'x', sizeof block);  /* 내용은 중요하지 않다 */
    fcntl(fd[1], F_SETFL, O_NONBLOCK);  /* 가득 차면 기다리지 않고 EAGAIN */
    while ((n = write(fd[1], block, sizeof block)) > 0)  /* 가득 차면 -1로 끝난다 */
        total += n;
    printf("가득 찰 때까지 쓴 양 = %ld바이트 (그 다음 write: %s)\n",
           total, strerror(errno));
    printf("PIPE_BUF = %d\n", PIPE_BUF);  /* 이 크기 이하의 write는 섞이지 않는다 */
#ifdef F_GETPIPE_SZ                 /* Linux에만 있다 */
    printf("F_GETPIPE_SZ = %d\n", fcntl(fd[1], F_GETPIPE_SZ));
#endif
    return 0;
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IOLBF, 0);   /* 어디로 나가든 줄 단위로 낸다 */
    if (argc == 2 && strcmp(argv[1], "ok") == 0)
        return run_msg(0);          /* 정상: 자식이 쓰기 끝을 닫는다 */
    if (argc == 2 && strcmp(argv[1], "leak") == 0)
        return run_msg(1);          /* 자식이 쓰기 끝을 닫지 않는다 */
    if (argc == 2 && strcmp(argv[1], "sigpipe") == 0)
        return run_noreader(0);     /* SIGPIPE를 그대로 받는다 */
    if (argc == 2 && strcmp(argv[1], "epipe") == 0)
        return run_noreader(1);     /* SIGPIPE를 무시하고 EPIPE를 본다 */
    if (argc == 2 && strcmp(argv[1], "cap") == 0)
        return run_cap();           /* 용량을 잰다 */
    fprintf(stderr, "usage: %s ok|leak|sigpipe|epipe|cap\n", argv[0]);
    return 2;                       /* 사용법 오류 */
}
