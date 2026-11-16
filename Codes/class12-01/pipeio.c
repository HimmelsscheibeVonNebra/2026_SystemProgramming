/* pipeio.c — 파이프를 쓰는 다른 두 방법: popen과 FIFO(이름 있는 파이프).
 * 사용법: ./pipeio popen "COMMAND"    셸 명령의 출력을 줄 단위로 읽는다
 *         ./pipeio recv FIFO          FIFO를 만들고 EOF까지 읽는다
 *         ./pipeio send FIFO TEXT     FIFO에 한 줄을 쓴다
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static void die(const char *what)
{
    perror(what);
    exit(1);
}

/* popen: pipe · fork · dup2 · exec(sh -c)를 한 번에 하는 stdio 함수 */
static int run_popen(const char *cmd)
{
    char line[256];
    int count = 0;

    FILE *fp = popen(cmd, "r");         /* "r": 명령의 표준 출력을 읽는다 */
    if (fp == NULL)
        die("popen");
    while (fgets(line, sizeof line, fp) != NULL)
        printf("%2d: %s", ++count, line);
    int st = pclose(fp);              /* fclose가 아니다. 자식을 기다린다 */
    if (st == -1)
        die("pclose");
    printf("줄 수 %d, 명령의 종료 코드 %d\n", count, WEXITSTATUS(st));
    return 0;
}

/* recv: FIFO를 만들고, 쓰는 쪽이 열 때까지 기다렸다가 EOF까지 읽는다. */
static int run_recv(const char *path)
{
    char buf[256];
    ssize_t n;

    if (mkfifo(path, 0600) == -1 && errno != EEXIST)
        die("mkfifo");
    printf("[recv] %s 를 연다 (쓰는 쪽을 기다림)\n", path);
    int fd = open(path, O_RDONLY);      /* 상대가 열 때까지 여기서 멈춘다 */
    if (fd == -1)
        die("open");
    while ((n = read(fd, buf, sizeof buf)) > 0)
        printf("[recv] %zd바이트: %.*s", n, (int)n, buf);
    if (n == -1)
        die("read");
    printf("[recv] EOF — 쓰는 쪽이 모두 닫았다\n");
    close(fd);
    unlink(path);                       /* 이름을 지운다 */
    return 0;
}

/* send: 이미 있는 FIFO를 쓰기로 열고 한 줄을 보낸다. */
static int run_send(const char *path, const char *text)
{
    char line[256];

    int fd = open(path, O_WRONLY);      /* 읽는 쪽이 열 때까지 멈춘다 */
    if (fd == -1)
        die("open");
    int len = snprintf(line, sizeof line, "%s\n", text);
    if (write(fd, line, (size_t)len) == -1)
        die("write");
    close(fd);
    return 0;
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    if (argc == 3 && strcmp(argv[1], "popen") == 0)
        return run_popen(argv[2]);
    if (argc == 3 && strcmp(argv[1], "recv") == 0)
        return run_recv(argv[2]);
    if (argc == 4 && strcmp(argv[1], "send") == 0)
        return run_send(argv[2], argv[3]);
    fprintf(stderr, "usage: %s popen CMD | recv FIFO | send FIFO TEXT\n", argv[0]);
    return 2;
}
