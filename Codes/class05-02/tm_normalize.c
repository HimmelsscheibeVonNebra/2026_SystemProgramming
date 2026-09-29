#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* struct tm을 직접 채워 mktime으로 정규화한다.
   tm_mon은 0부터 센다: 0이 1월, 11이 12월이다. */
static int normalize(const char *label, int year, int mon, int mday)
{
    struct tm tm_in;
    memset(&tm_in, 0, sizeof tm_in);
    tm_in.tm_year = year - 1900;
    tm_in.tm_mon = mon;
    tm_in.tm_mday = mday;
    tm_in.tm_hour = 12;
    tm_in.tm_min = 0;
    tm_in.tm_sec = 0;
    tm_in.tm_isdst = -1;   /* 시스템이 서머타임 여부를 정하게 한다 */

    printf("%s: %04d-%02d-%02d 입력 (tm_mon=%d)\n",
           label, year, mon + 1, mday, mon);
    time_t t = mktime(&tm_in);
    if (t == (time_t)-1) {
        perror("mktime");
        return 1;
    }
    char buf[32];
    if (strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", &tm_in) == 0) {
        fprintf(stderr, "strftime: buffer too small\n");
        return 1;
    }
    static const char *const wday[] =
        {"일", "월", "화", "수", "목", "금", "토"};
    printf("  mktime 결과 %s (%s요일, tm_wday=%d)\n",
           buf, wday[tm_in.tm_wday], tm_in.tm_wday);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 2;
    }
    int status = 0;
    status |= normalize("1월 32일", 2026, 0, 32);
    status |= normalize("12월 32일", 2026, 11, 32);
    status |= normalize("2025-02-29", 2025, 1, 29);   /* 평년 2월 29일 */
    status |= normalize("2024-02-29", 2024, 1, 29);   /* 윤년 2월 29일 */
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return 1;
    }
    return status;
}
