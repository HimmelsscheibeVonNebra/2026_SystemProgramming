/* pipeio.c — 파이프를 쓰는 다른 두 방법: popen과 FIFO(이름 있는 파이프).
 * 사용법: ./pipeio popen "COMMAND"    셸 명령의 출력을 줄 단위로 읽는다
 *         ./pipeio recv FIFO          FIFO를 만들고 EOF까지 읽는다
 *         ./pipeio send FIFO TEXT     FIFO에 한 줄을 쓴다
 */
#include <errno.h>                  /* errno, EEXIST */
#include <fcntl.h>                  /* open, O_RDONLY, O_WRONLY */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>               /* mkfifo */
#include <sys/wait.h>               /* WEXITSTATUS */
#include <unistd.h>                 /* read, write, close, unlink */

static void die(const char *what)
{
    perror(what);
    exit(1);
}

/* popen: pipe · fork · dup2 · exec(sh -c)를 한 번에 하는 stdio 함수 */
static int run_popen(const char *cmd)
{
    char line[256];                 /* 명령 출력의 한 줄 */
    int count = 0;                  /* 읽은 줄 수 */

    FILE *fp = popen(cmd, "r");         /* "r": 명령의 표준 출력을 읽는다 */
    if (fp == NULL)                 /* pipe나 fork가 실패했다 */
        die("popen");
    while (fgets(line, sizeof line, fp) != NULL)  /* 명령이 끝나 EOF면 NULL */
        printf("%2d: %s", ++count, line);  /* 줄 번호를 붙여 낸다 */
    int st = pclose(fp);              /* fclose가 아니다. 자식을 기다린다 */
    if (st == -1)                   /* wait가 실패했다 */
        die("pclose");
    printf("줄 수 %d, 명령의 종료 코드 %d\n", count, WEXITSTATUS(st));
    return 0;
}

/* recv: FIFO를 만들고, 쓰는 쪽이 열 때까지 기다렸다가 EOF까지 읽는다. */
static int run_recv(const char *path)
{
    char buf[256];                  /* NUL로 끝나지 않는다: 길이 n과 함께 쓴다 */
    ssize_t n;                      /* read의 반환값 */

    if (mkfifo(path, 0600) == -1 && errno != EEXIST)  /* 이미 있으면 그대로 쓴다 */
        die("mkfifo");
    printf("[recv] %s 를 연다 (쓰는 쪽을 기다림)\n", path);
    int fd = open(path, O_RDONLY);      /* 상대가 열 때까지 여기서 멈춘다 */
    if (fd == -1)                   /* 예: 권한이 없으면 EACCES */
        die("open");
    while ((n = read(fd, buf, sizeof buf)) > 0)  /* 쓰는 쪽이 모두 닫으면 0 */
        printf("[recv] %zd바이트: %.*s", n, (int)n, buf);
    if (n == -1)                    /* -1은 오류, 0은 EOF */
        die("read");
    printf("[recv] EOF — 쓰는 쪽이 모두 닫았다\n");
    close(fd);                      /* 읽기 끝을 닫는다 */
    unlink(path);                       /* 이름을 지운다 */
    return 0;
}

/* send: 이미 있는 FIFO를 쓰기로 열고 한 줄을 보낸다. */
static int run_send(const char *path, const char *text)
{
    char line[256];                 /* 보낼 한 줄 */

    int fd = open(path, O_WRONLY);      /* 읽는 쪽이 열 때까지 멈춘다 */
    if (fd == -1)                   /* FIFO가 없으면 ENOENT */
        die("open");
    int len = snprintf(line, sizeof line, "%s\n", text);  /* 줄바꿈을 붙인다 */
    if (write(fd, line, (size_t)len) == -1)  /* PIPE_BUF 이하라 섞이지 않는다 */
        die("write");
    close(fd);                      /* 쓰기 끝 0개 → recv가 EOF를 받는다 */
    return 0;
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IOLBF, 0);  /* 어디로 나가든 줄 단위로 낸다 */
    if (argc == 3 && strcmp(argv[1], "popen") == 0)
        return run_popen(argv[2]);  /* 명령의 출력을 읽는다 */
    if (argc == 3 && strcmp(argv[1], "recv") == 0)
        return run_recv(argv[2]);   /* FIFO로 받는다 */
    if (argc == 4 && strcmp(argv[1], "send") == 0)
        return run_send(argv[2], argv[3]);  /* FIFO로 보낸다 */
    fprintf(stderr, "usage: %s popen CMD | recv FIFO | send FIFO TEXT\n", argv[0]);
    return 2;                       /* 사용법 오류 */
}
