/* pipetalk.c — 파이프 두 개로 부모와 자식이 묻고 답한다.
 * 양방향 통신의 규약과, 서로 기다리다 멈추는 교착 세 가지를 확인한다.
 * 사용법: ./pipetalk ask | both | mute | flood N | fix N
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>               /* waitpid, WEXITSTATUS */
#include <unistd.h>                 /* pipe, fork, read, write, close */

enum { CALC, CALC_MUTE, ECHO };     /* 자식이 맡을 역할 */

static void die(const char *what)   /* 실패한 호출 이름과 errno 메시지를 내고 끝낸다 */
{
    perror(what);
    exit(1);
}

/* 자식(계산): "A B\n"을 받을 때마다 합을 한 줄로 답한다. EOF면 끝낸다. */
static void child_calc(int in, int out, int mute)
{
    char buf[64];                   /* 질문 한 줄 */
    ssize_t n;                      /* read의 반환값 */
    FILE *fp = fdopen(out, "w");    /* 답은 stdio로 쓴다 (4주차) */

    if (fp == NULL)
        die("fdopen");
    while ((n = read(in, buf, sizeof buf - 1)) > 0) {  /* 질문 하나가 read 한 번 */
        int a, b;
        buf[n] = '\0';              /* sscanf가 읽도록 문자열로 만든다 */
        if (sscanf(buf, "%d %d", &a, &b) != 2)
            continue;               /* 형식이 틀린 줄은 버린다 */
        fprintf(fp, "%d\n", a + b); /* 아직 fp의 버퍼 안에만 있다 */
        if (!mute)                  /* mute 모드는 다음 줄만 건너뛴다 */
            fflush(fp);             /* 버퍼를 비워야 파이프로 나간다 */
    }
    fclose(fp);                     /* 남은 버퍼를 비우고 out을 닫는다 */
    exit(0);
}

/* 자식(메아리): 받은 바이트를 그대로 돌려보낸다. cat과 같다. */
static void child_echo(int in, int out)
{
    char buf[4096];                 /* 한 번에 옮길 덩어리 */
    ssize_t n;

    while ((n = read(in, buf, sizeof buf)) > 0)
        if (write(out, buf, (size_t)n) == -1)   /* rep가 가득 차면 여기서 잠든다 */
            die("write");
    exit(0);                        /* EOF: 끝점은 종료하면서 닫힌다 */
}

/* 파이프 둘을 만들고 자식을 띄운다. 부모가 쓸 끝점 둘을 돌려준다. */
static pid_t spawn(int role, int *to_child, int *from_child)
{
    int req[2], rep[2];             /* req: 부모 → 자식, rep: 자식 → 부모 */

    if (pipe(req) == -1 || pipe(rep) == -1)
        die("pipe");
    pid_t pid = fork();             /* 끝점 넷이 여덟이 된다 */
    if (pid == -1)
        die("fork");
    if (pid == 0) {
        close(req[1]);              /* 자식 ①: req의 쓰기 끝 */
        close(rep[0]);              /* 자식 ②: rep의 읽기 끝 */
        if (role == ECHO)
            child_echo(req[0], rep[1]);
        child_calc(req[0], rep[1], role == CALC_MUTE);
    }
    close(req[0]);                  /* 부모 ①: req의 읽기 끝 */
    close(rep[1]);                  /* 부모 ②: rep의 쓰기 끝 */
    *to_child = req[1];
    *from_child = rep[0];
    return pid;
}

/* ask · both · mute: 질문 셋을 차례로 보내고 답을 받는다. */
static int run_ask(int role, int read_first)
{
    const char *q[] = { "1 2\n", "30 40\n", "500 600\n" };
    char buf[64];
    ssize_t n;
    int to, from;                   /* 부모의 쓰기 끝, 읽기 끝 */
    pid_t pid = spawn(role, &to, &from);

    if (read_first) {               /* both: 묻기 전에 답부터 기다린다 */
        printf("[부모] 먼저 읽는다 (자식도 읽는 중)\n");
        fflush(stdout);
        n = read(from, buf, sizeof buf);        /* 영원히 돌아오지 않는다 */
    }
    for (int i = 0; i < 3; i++) {
        if (write(to, q[i], strlen(q[i])) == -1)    /* 질문 하나 = write 한 번 */
            die("write");
        printf("[부모] 질문: %s", q[i]);
        n = read(from, buf, sizeof buf);        /* 답이 올 때까지 잠든다 */
        if (n <= 0)
            die("read");
        printf("[부모] 답:   %.*s", (int)n, buf);
    }
    close(to);                      /* 더 물을 것이 없다 → 자식이 EOF를 받는다 */
    close(from);
    int st;
    waitpid(pid, &st, 0);
    printf("[부모] 자식 종료 코드 %d\n", WEXITSTATUS(st));
    return 0;
}

/* 쓰기 끝에 total바이트를 4096씩 보낸다. 32768바이트마다 진행을 알린다. */
static void send_all(int to, long total)
{
    char block[4096];
    memset(block, 'x', sizeof block);
    for (long sent = 0; sent < total; ) {
        size_t len = total - sent < 4096 ? (size_t)(total - sent) : 4096;
        if (write(to, block, len) == -1)        /* req가 가득 차면 여기서 잠든다 */
            die("write");
        sent += (long)len;
        if (sent % 32768 == 0 || sent == total)
            fprintf(stderr, "[보냄] %ld바이트\n", sent);
    }
}

/* flood · fix: N바이트를 메아리 자식에게 보내고 모두 돌려받는다. */
static int run_flood(long total, int fix)
{
    char buf[4096];
    long got = 0;                   /* 돌려받은 바이트 수 */
    ssize_t n;
    int to, from;
    pid_t pid = spawn(ECHO, &to, &from);
    pid_t writer = -1;              /* fix에서만 쓰는 둘째 자식 */

    if (fix) {                      /* fix: 쓰는 일을 다른 프로세스에 맡긴다 */
        writer = fork();
        if (writer == -1)
            die("fork");
        if (writer == 0) {
            close(from);            /* 쓰는 자식은 읽지 않는다 */
            send_all(to, total);
            exit(0);                /* to가 닫힌다 */
        }
    } else {
        send_all(to, total);        /* flood: 다 쓴 뒤에야 읽으러 간다 */
    }
    close(to);                      /* 부모의 쓰기 끝: 닫아야 메아리가 EOF를 받는다 */
    while ((n = read(from, buf, sizeof buf)) > 0)   /* 쓰는 동안에도 읽는다 (fix) */
        got += n;
    printf("[부모] 돌려받은 양 = %ld바이트\n", got);
    close(from);
    waitpid(pid, NULL, 0);
    if (fix)
        waitpid(writer, NULL, 0);
    return got == total ? 0 : 1;
}

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "ask") == 0)
        return run_ask(CALC, 0);
    if (argc == 2 && strcmp(argv[1], "both") == 0)
        return run_ask(CALC, 1);
    if (argc == 2 && strcmp(argv[1], "mute") == 0)
        return run_ask(CALC_MUTE, 0);
    if (argc == 3 && strcmp(argv[1], "flood") == 0)
        return run_flood(atol(argv[2]), 0);
    if (argc == 3 && strcmp(argv[1], "fix") == 0)
        return run_flood(atol(argv[2]), 1);
    fprintf(stderr, "사용법: %s ask | both | mute | flood N | fix N\n", argv[0]);
    return 2;
}
