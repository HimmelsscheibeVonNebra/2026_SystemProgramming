# Week03-class02 · 견고한 반복문과 파일 복사

120분 수업. 기존 3주차 첫 번째 수업에 이어 진행합니다.


## 1. 저수준 파일 입출력 · 견고한 반복문과 파일 복사




## 2. 오늘의 구성 · 120분

1부  지난 시간 복습 · 15분
2부  read와 오류 처리 · 20분
3부  write_all 구현 · 25분
휴식 · 10분
4부  파일 복사와 테스트 · 40분
퀴즈와 제출 · 10분


## 3. 1부 · 지난 시간 복습

파일 프로그램 4개의 반환값과 동작을 확인합니다.


## 4. 파일 프로그램 4개

file_create: 새로운 빈 파일 생성
file_write: 덮어쓰기 또는 추가 기록
file_read: EOF까지 읽고 출력
file_seek: 바이트 위치를 이동해 수정

교수자 메모: 지난 차시 코드는 labs/class03-01에 있다. 오늘은 제공받았던 write_all을 직접 구현한다.


## 5. 반환값 복습

open: 성공한 FD는 0 이상
read / write: 실제 처리한 바이트 수
오류를 나타내는 -1은 ssize_t로 받는다.
close: 성공 0, 실패 -1

참고: [open(2)](https://man7.org/linux/man-pages/man2/open.2.html), [read(2)](https://man7.org/linux/man-pages/man2/read.2.html), [write(2)](https://man7.org/linux/man-pages/man2/write.2.html), [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)


## 6. 결과 예측

read는 읽은 길이만큼 위치를 옮긴다.
쓰기 전 위치를 다시 지정한다.
O_APPEND는 매 write를 끝에 기록한다.

```c

/* ABCDEFGHIJ 파일, O_RDWR */
lseek(fd, 3, SEEK_SET);
read(fd, buf, 2);
/* 현재 위치는? */
lseek(fd, 3, SEEK_SET);
write(fd, "XY", 2);

```

교수자 메모: 정답: 읽은 결과 DE, 현재 위치 5, 최종 ABCXYFGHIJ. 흐름 발췌이며 완성 코드에서는 반환값을 검사한다.

참고: [lseek(2)](https://man7.org/linux/man-pages/man2/lseek.2.html), [read(2)](https://man7.org/linux/man-pages/man2/read.2.html), [write(2)](https://man7.org/linux/man-pages/man2/write.2.html)


## 7. 2부 · read와 오류 처리

EOF, 짧은 읽기, 오류를 구별합니다.


## 8. read의 세 가지 결과

n > 0: n바이트를 처리하고 계속 읽는다.
n == 0: 일반 파일의 EOF로 종료한다.
n == -1: errno를 확인한다.
이번 실습은 양수 크기로 읽는 일반 파일이다.

참고: [read(2)](https://man7.org/linux/man-pages/man2/read.2.html)


## 9. 짧은 읽기와 EOF

10바이트 파일을 4바이트씩 읽는다.
예상 반환값은 4, 4, 2, 0이다.
2를 반환한 호출도 성공이다.
요청보다 적게 읽었다고 종료하지 않는다.

교수자 메모: 인터럽트 등 없이 일반 파일을 읽는 예시. 실제 호출별 길이는 환경에 따라 달라질 수 있다.

참고: [read(2)](https://man7.org/linux/man-pages/man2/read.2.html)


## 10. 버퍼에서 유효한 범위

이번에 읽은 데이터는 n바이트다.
read는 끝에 NUL을 붙이지 않는다.
바이너리 데이터에 strlen을 쓰지 않는다.

```c

unsigned char buf[4];
ssize_t n = read(fd, buf, sizeof buf);
if (n > 0) {
    /* buf[0]부터 buf[n-1]까지 */
    use_bytes(buf, (size_t)n);
}

```

교수자 메모: use_bytes는 개념 설명용 이름이다. 완성 복사기에서는 write_all을 호출한다.

참고: [read(2)](https://man7.org/linux/man-pages/man2/read.2.html)


## 11. errno와 EINTR

errno는 실패 반환값을 확인한 뒤 읽는다.
EINTR: 데이터 처리 전 신호로 중단
EINTR이면 같은 요청을 다시 시도한다.
양수 반환이면 실제 처리량을 반영한다.
그 밖의 오류는 호출자에게 전달한다.

교수자 메모: 신호 자체는 후속 차시에서 다룬다. 이번에는 데이터 처리 전 중단(-1/EINTR)과 일부 처리 뒤 양수 반환을 구분한다. 성공 직후 errno 값으로 성공 여부를 판정하지 않는다.

참고: [read(2)](https://man7.org/linux/man-pages/man2/read.2.html), [write(2)](https://man7.org/linux/man-pages/man2/write.2.html)


## 12. 읽기 반복문의 분기

0과 -1은 다른 종료 경로다.
EINTR은 재시도한다.
양수인 n만 다음 단계에 전달한다.

```c

for (;;) {
    ssize_t n = read(fd, buf, sizeof buf);
    if (n == 0) break;
    if (n == -1) {
        if (errno == EINTR) continue;
        return -1;
    }
    /* 읽은 n바이트를 처리 */
}

```

참고: [read(2)](https://man7.org/linux/man-pages/man2/read.2.html)


## 13. 실습 1 · 읽은 길이 관찰

새 실습 폴더에서 make를 실행한다.
표준 오류의 n 값을 확인한다.
입력과 출력 바이트를 비교한다.

```sh

make
RUN=$(mktemp -d ./run.XXXXXX)
printf ABCDEFGHIJ > "$RUN/in"
./read_chunks "$RUN/in" > "$RUN/out"
cmp "$RUN/in" "$RUN/out"
echo $?

```

교수자 메모: 4, 4, 2, 0 출력. cmp 직후 종료 상태 0이면 같다. RUN은 이후에도 같은 셸에서 사용한다.


## 14. 3부 · write_all 구현

처리한 위치와 남은 길이를 갱신합니다.


## 15. 한 번의 write와 전체 기록

write(fd, buf, 10)이 4를 반환할 수 있다.
앞의 4바이트는 이미 기록했다.
다음 요청은 buf + 4에서 6바이트다.
성공한 양수 반환도 완료 여부를 확인한다.

참고: [write(2)](https://man7.org/linux/man-pages/man2/write.2.html)


## 16. 진행 상태 계산

시작: 위치 0, 남은 길이 10
4바이트 성공: 위치 4, 남은 길이 6
2바이트 성공: 위치 6, 남은 길이 4
4바이트 성공: 위치 10, 남은 길이 0

교수자 메모: 항상 처리한 길이 + 남은 길이 = 원래 길이. ABCDEFGHIJ라면 요청 시작은 A, E, G로 이동한다.


## 17. write_all의 계약

성공: 요청한 바이트를 모두 쓰고 0 반환
실패: -1 반환, errno로 원인 전달
실패 전에 일부 바이트를 썼을 수 있다.
실습은 작은 버퍼와 blocking FD를 사용한다.

교수자 메모: write_all은 직접 작성한 함수이고 시스템 호출이 아니다. 원자적 작업이나 실패 시 롤백을 보장하지 않는다. 비차단 FD의 EAGAIN은 이 함수에서 재시도하지 않는다.

참고: [write(2)](https://man7.org/linux/man-pages/man2/write.2.html)


## 18. 반복문과 포인터 갱신

p는 다음 기록 위치다.
left는 아직 쓰지 않은 길이다.
양수 반환일 때만 두 값을 바꾼다.

```c

const unsigned char *p = data;
size_t left = count;
while (left > 0) {
    ssize_t n = write(fd, p, left);
    /* 오류와 0 처리: 다음 슬라이드 */
    p += (size_t)n;
    left -= (size_t)n;
}

```

교수자 메모: 분할 발췌다. 오류 분기가 없는 상태로 실행하지 않는다. void* 대신 unsigned char*로 바이트 단위 산술을 수행한다.

참고: [write(2)](https://man7.org/linux/man-pages/man2/write.2.html)


## 19. 오류와 진행 없는 반환

EINTR이면 위치를 유지한다.
다른 오류는 -1로 전달한다.
0은 EIO로 처리해 반복을 끝낸다.

```c

if (n == -1) {
    if (errno == EINTR) continue;
    return -1;
}
if (n == 0) {
    errno = EIO;
    return -1;
}

```

교수자 메모: 앞 슬라이드의 write 직후 삽입한다. 양수 길이를 요청했는데 0이 나오면 우리 함수가 선택한 정책으로 EIO를 설정한다. write가 반드시 EIO를 설정한다는 뜻은 아니다.

참고: [write(2)](https://man7.org/linux/man-pages/man2/write.2.html)


## 20. 실습 2 · 함수 완성

student/io_helpers.h의 TODO를 구현한다.
부분 성공 뒤 위치와 길이를 갱신한다.
제공 테스트로 분기를 확인한다.

```sh

make test-student
# 실패한 조건과 호출 인자를 확인
# student/io_helpers.h 수정 후 재실행
make test-student

```

교수자 메모: starter는 ENOSYS를 반환하므로 수정 전 실패가 정상이다. 정답은 io_helpers.h이며 스스로 구현한 다음 비교한다.


## 21. 반환값을 정한 테스트

가짜 write의 결과: 4, -1/EINTR, 2, 4
요청 길이는 10, 6, 6, 4가 되어야 한다.
EINTR 앞뒤의 포인터는 같아야 한다.
기록된 결과가 ABCDEFGHIJ인지 검사한다.

교수자 메모: 가짜 함수는 커널 동작을 바꾸지 않는다. 테스트가 write 호출을 대체해 분기를 재현한다. 실제 신호 실험이나 성능 측정 결과로 해석하지 않는다.


## 22. 휴식 · 10분

다음 실습에서 읽기와 쓰기 반복문을 연결합니다.


## 23. 4부 · 파일 복사와 테스트

두 FD를 열고 EOF까지 복사한 뒤 결과를 검증합니다.


## 24. file_copy의 구성

입력 파일: O_RDONLY로 연다.
출력 파일: O_WRONLY | O_CREAT | O_EXCL
read로 얻은 n바이트를 write_all로 쓴다.
입력과 출력 FD를 모두 닫는다.

교수자 메모: 학습용 복사기는 기존 목적지 덮어쓰기를 거부한다. 동일 경로·하드 링크·기존 심볼릭 링크도 목적지 생성 단계에서 실패한다. 내용만 복사하며 권한·시간 등 메타데이터 복제나 원자적 교체는 다루지 않는다.

참고: [open(2)](https://man7.org/linux/man-pages/man2/open.2.html), [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)


## 25. 복사 반복문

바깥 반복문은 EOF까지 읽는다.
write_all은 n바이트를 모두 쓴다.
읽기 또는 쓰기 실패는 -1로 전달한다.

```c

for (;;) {
    ssize_t n = read(in, buf, sizeof buf);
    if (n == 0) return 0;
    if (n == -1) {
        if (errno == EINTR) continue;
        return -1;
    }
    if (write_all(out, buf, (size_t)n) < 0)
        return -1;
}

```

교수자 메모: copy_fd 핵심 코드. 이 함수는 FD를 빌려 쓰고 main이 닫는다.

참고: [read(2)](https://man7.org/linux/man-pages/man2/read.2.html), [write(2)](https://man7.org/linux/man-pages/man2/write.2.html)


## 26. 실패 뒤에도 FD 닫기

출력 open 실패 시 입력 FD를 닫는다.
복사 실패 시에도 두 FD를 닫는다.
close 실패도 종료 상태에 반영한다.
Linux에서 close 실패를 일괄 재시도하지 않는다.

교수자 메모: Linux에서는 close가 오류를 반환해도 FD를 이미 해제했을 수 있다. 재시도는 다른 FD를 닫을 위험이 있다. 실패한 복사 결과는 부분 파일로 남을 수 있으므로 성공 결과로 사용하지 않는다.

참고: [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)


## 27. 실습 3 · 파일 복사

새 목적지 파일 이름을 사용한다.
cmp의 종료 상태를 확인한다.
같은 목적지로 다시 복사해 본다.

```sh

./file_copy "$RUN/in" "$RUN/copy"
cmp "$RUN/in" "$RUN/copy"
echo $?
./file_copy "$RUN/in" "$RUN/copy"
echo $?

```

교수자 메모: 첫 cmp는 0, 두 번째 복사기는 1이다. 기존 목적지 내용은 유지된다.


## 28. 경계 조건과 오류 테스트

빈 파일과 NUL 포함 바이너리
버퍼 크기 4096의 앞뒤 길이
입력 없음, 기존 목적지, 동일 경로
부분 쓰기, EINTR, 0 반환, 쓰기 오류

교수자 메모: make test는 가짜 write의 호출 인자와 바이트 결과를 검사하고 복사기 통합 테스트를 실행한다. Linux에서 /dev/full이 있으면 실제 쓰기 실패도 확인한다.


## 29. 실습 4 · 호출 관찰

Linux에서 read와 write를 추적한다.
실습 파일 FD의 호출을 찾아 읽는다.
반환값과 다음 요청 길이를 비교한다.

```sh

strace -o "$RUN/trace.log" \
  -e trace=openat,read,write,close \
  ./file_copy "$RUN/in" "$RUN/traced"
cat "$RUN/trace.log"
cmp "$RUN/in" "$RUN/traced"

```

교수자 메모: 새 목적지 traced가 필요하다. 동적 로더와 라이브러리 파일의 호출도 섞인다. 정상 일반 파일에서 부분 쓰기가 보이지 않아도 오류가 아니다. strace는 Linux 선택 실습이다.


## 30. 코드 리뷰 · 버그 찾기

요청 길이와 실제 길이를 비교한다.
음수 값을 unsigned로 바꾸는 시점을 본다.
부분 쓰기 뒤 재요청 범위를 설명한다.

```c

size_t n = read(in, buf, sizeof buf);
if (n > 0)
    write(out, buf, sizeof buf);

```

교수자 메모: 문제: -1이 큰 양수로 바뀜, n 대신 sizeof buf 기록, 부분 쓰기 및 오류 미처리, EOF까지 반복 없음. 올바른 ssize_t와 copy_fd를 비교한다.

참고: [read(2)](https://man7.org/linux/man-pages/man2/read.2.html), [write(2)](https://man7.org/linux/man-pages/man2/write.2.html)


## 31. 정리 퀴즈

1. 10바이트 write가 4를 반환하면 다음 인자는?
2. read가 2를 반환하면 EOF인가?
3. -1 / EINTR일 때 포인터는 어떻게 바꾸는가?
4. write_all 실패 시 파일은 원래 상태인가?

교수자 메모: 정답: buf+4와 6 / 아니다, 다음 read를 계속 / 바꾸지 않고 재시도 / 일부가 기록됐을 수 있다.


## 32. 제출과 다음 시간

student/io_helpers.h 구현본
REPORT.md의 예측·실행 결과·오류 분석
make test-student와 복사 비교 결과
다음 시간: FILE*와 stdio 버퍼링

교수자 메모: 오늘 총 120분. 마지막 10분에 퀴즈와 제출을 진행한다. 함수 테스트는 make test-student, 학생 함수로 만든 복사기는 make file_copy_student로 실행한다.


## 33. 참고 자료

Linux man-pages: read(2), write(2)
Linux man-pages: open(2), close(2)
지난 차시: 파일 프로그램 4개
실습 안내: labs/class03-02/README.md

참고: [read(2)](https://man7.org/linux/man-pages/man2/read.2.html), [write(2)](https://man7.org/linux/man-pages/man2/write.2.html), [open(2)](https://man7.org/linux/man-pages/man2/open.2.html), [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)


# 보충 자료 · 버퍼와 표준 입출력

기존 120분 수업에 추가한 약 20~25분 분량입니다.

## 34. 보충 · 버퍼와 표준 입출력

fread와 fwrite의 크기, 반환값, 버퍼링을 살펴봅니다.

교수자 메모: 기존 120분 수업 이후 또는 다음 차시 도입에 사용할 보충 자료. 약 20~25분.

## 35. 버퍼의 메모리 경계

buf[4]에 쓰면 끝을 넘는다.
buf[-1]에 쓰면 시작 앞을 침범한다.
두 접근 모두 정의되지 않은 동작이다.

```c
char buf[4];

buf[0] = 'A';  /* 정상 */
buf[3] = 'D';  /* 정상 */

/* 다음 코드는 실행하지 않는다. */
buf[4] = 'X';  /* overflow */
buf[-1] = 'Y'; /* underwrite */
```

교수자 메모: 유효한 인덱스는 0~3이다. 범위 밖 읽기는 over-read 또는 under-read, 쓰기는 overflow 또는 underwrite로 구분한다. 잘못된 접근은 메모리 손상, 종료 또는 겉보기 정상 등 정의되지 않은 동작을 일으킨다.

## 36. 데이터 흐름의 underflow

버퍼는 데이터를 잠시 저장하는 공간이다.
Underflow: 소비할 데이터가 부족한 상태
예: 오디오 데이터 도착이 늦어 재생이 끊김
Overflow: 빈 공간보다 더 많은 데이터가 도착
메모리 경계 침범과 구별해 문맥을 확인한다.

교수자 메모: 빈 큐의 읽기와 꽉 찬 큐의 쓰기는 구현에 따라 대기, 실패 또는 데이터 손실로 처리한다. stdio 입력 버퍼가 비면 라이브러리가 다시 채울 수 있다. 버퍼가 비었다는 사실만으로 EOF나 메모리 오류라고 판단하지 않는다.

## 37. read / write와 stdio

read / write: 정수 FD, 반환값은 바이트 수
fread / fwrite: FILE*, 반환값은 항목 수
stdio는 C 라이브러리 버퍼를 사용할 수 있다.
fread는 스트림에서 메모리로 읽는다.
fwrite는 메모리에서 스트림으로 쓴다.

교수자 메모: read/write는 POSIX 인터페이스, fread/fwrite는 C 표준 라이브러리 인터페이스다. stdio 호출 한 번과 시스템 호출 한 번이 항상 대응하지는 않는다.

참고: [read.2](https://man7.org/linux/man-pages/man2/read.2.html), [write.2](https://man7.org/linux/man-pages/man2/write.2.html), [fread.3](https://man7.org/linux/man-pages/man3/fread.3.html), [stdio.3](https://man7.org/linux/man-pages/man3/stdio.3.html)

## 38. fread와 fwrite의 인자

ptr: 데이터를 담은 메모리 주소
size: 항목 하나의 바이트 크기
nmemb: 요청할 항목 개수

```c
#include <stdio.h>

fread(ptr, size, nmemb, stream);
fwrite(ptr, size, nmemb, stream);

/* 요청 크기 = size * nmemb */
/* stream: FILE * */
/* 두 함수의 반환형: size_t */
```

교수자 메모: 호출 형태를 보여 주는 발췌다. fread의 ptr은 쓰기 가능한 메모리, fwrite의 ptr은 읽을 데이터다. 호출자는 요청 크기에 맞는 버퍼를 제공해야 한다. 크기 곱셈이 size_t 범위를 넘지 않는지도 확인한다.

참고: [fread.3](https://man7.org/linux/man-pages/man3/fread.3.html)

## 39. 반환값의 단위

첫 호출의 n은 완전히 읽은 int 개수다.
size가 1이면 반환값은 바이트 수다.
요청 개수와 반환값을 비교한다.

```c
int values[10];
size_t n = fread(values,
                 sizeof values[0], 10, fp);
/* n == 10: int 10개를 읽음 */

unsigned char buf[100];
size_t bytes = fread(buf, 1, sizeof buf, fp);
/* bytes: 실제로 읽은 바이트 수 */
```

교수자 메모: size가 1보다 크면 마지막 불완전한 항목은 반환 개수에 포함되지 않는다. 짧은 fread 결과는 EOF 또는 오류를 뜻하므로 feof/ferror를 확인한다. fwrite는 요청보다 적은 항목을 반환하면 쓰기 오류다.

참고: [fread.3](https://man7.org/linux/man-pages/man3/fread.3.html)

## 40. fread와 버퍼 크기

최대 99바이트를 읽고 NUL을 붙인다.
fread는 NUL을 자동으로 붙이지 않는다.
바이너리는 읽은 n바이트로 처리한다.

```c
char text[100];
size_t n = fread(text, 1,
                 sizeof text - 1, fp);
text[n] = '\0';

/* 위험한 요청: 실행하지 않는다. */
char small[10];
fread(small, 1, 100, fp);
```

교수자 메모: 위 코드는 크기 처리 발췌이며 완성 프로그램에서는 ferror도 확인한다. 마지막 fread는 충분한 입력이 있으면 버퍼를 넘쳐 쓴다. fwrite(small,1,100,fp)는 버퍼 밖 읽기를 일으킬 수 있다. 문자열 ABCD는 NUL까지 5바이트가 필요하다.

참고: [fread.3](https://man7.org/linux/man-pages/man3/fread.3.html)

## 41. fwrite와 텍스트 출력

fwrite는 메모리의 바이트를 기록한다.
fprintf는 값을 텍스트로 변환한다.
두 파일은 저장 형식이 다르다.

```c
int value = 1234;

size_t n = fwrite(&value,
                   sizeof value, 1, fp);
/* 성공한 항목 수 n == 1 확인 */

int rc = fprintf(text_fp, "%d\n", value);
/* 오류이면 rc < 0 */
```

교수자 메모: 서로 다른 출력 스트림을 가정한 호출 비교다. int의 원시 바이트는 크기와 바이트 순서가 환경마다 다를 수 있다. 구조체에는 패딩이 있어 그대로 저장하면 이식성이 떨어진다.

참고: [fread.3](https://man7.org/linux/man-pages/man3/fread.3.html), [fprintf.3](https://man7.org/linux/man-pages/man3/fprintf.3.html)

## 42. EOF와 오류 판별

먼저 fread의 반환값을 확인한다.
ferror가 참이면 읽기 오류다.
feof는 읽기 후 EOF 상태를 알려 준다.

```c
size_t n = fread(buf, 1, sizeof buf, fp);
/* 먼저 읽은 n바이트를 처리 */

if (n < sizeof buf) {
    if (ferror(fp)) {
        /* 읽기 오류 처리 */
    } else if (feof(fp)) {
        /* 파일 끝 도달 */
    }
}
```

교수자 메모: sizeof buf가 양수인 배열이라고 가정한다. while (!feof(fp))는 다음 읽기의 성공을 예측하지 못한다. 마지막 데이터를 버퍼 크기만큼 읽었다면 다음 읽기를 시도해야 EOF 표시가 설정될 수 있다.

참고: [fread.3](https://man7.org/linux/man-pages/man3/fread.3.html), [ferror.3](https://man7.org/linux/man-pages/man3/ferror.3.html)

## 43. stdio 파일 복사 반복문

읽은 n바이트만 출력한다.
fwrite의 항목 수를 검사한다.
읽기 오류도 실패 결과에 반영한다.

```c
unsigned char buf[4096];
size_t n;
int failed = 0;
while ((n = fread(buf, 1, sizeof buf, src)) > 0) {
    if (fwrite(buf, 1, n, dst) != n) {
        failed = 1;
        break;
    }
}
if (ferror(src)) failed = 1;
```

교수자 메모: 유효하게 연 src와 dst를 가정한 반복문 발췌다. 앞뒤의 fopen과 fclose 처리는 다음 슬라이드와 함께 설명한다. 실패한 출력 파일에는 일부 데이터가 남을 수 있다.

참고: [fread.3](https://man7.org/linux/man-pages/man3/fread.3.html), [ferror.3](https://man7.org/linux/man-pages/man3/ferror.3.html)

## 44. fopen과 fclose

fopen 실패는 NULL로 확인한다.
rb는 읽기, wb는 생성 또는 내용 삭제다.
fclose의 출력 오류도 검사한다.

```c
FILE *src = fopen("input.bin", "rb");
if (!src) return 1;
FILE *dst = fopen("output.bin", "wb");
if (!dst) { fclose(src); return 1; }

/* 앞 슬라이드의 복사 반복문 삽입 */

if (fclose(src) == EOF) failed = 1;
if (fclose(dst) == EOF) failed = 1;
return failed;
```

교수자 메모: main 안에서 앞 슬라이드 반복문과 결합하는 예시다. 서로 다른 입력과 출력 파일을 사용한다. wb는 기존 파일을 지우므로 기존 목적지를 보존하는 본 수업의 O_EXCL 복사기와 동작이 다르다. ab는 끝에 추가한다. Linux에서는 b가 동작을 바꾸지 않는다. fclose 뒤에는 해당 FILE*를 다시 사용하지 않는다.

참고: [fopen.3](https://man7.org/linux/man-pages/man3/fopen.3.html), [fclose.3](https://man7.org/linux/man-pages/man3/fclose.3.html)

## 45. 출력 버퍼와 fflush

fwrite 성공 뒤에도 출력 버퍼에 남을 수 있다.
fflush는 stdio 출력 버퍼를 커널로 내보낸다.
fflush와 fclose는 성공 0, 실패 EOF를 반환한다.
디스크 반영이 필요하면 fsync 등도 고려한다.
fseek는 스트림의 파일 위치를 변경한다.

교수자 메모: 일반적인 버퍼링 출력의 흐름은 프로그램 메모리, C 라이브러리 버퍼, 커널, 저장 장치 순이다. fflush만으로 영구 저장을 보장하지 않는다. fsync(fileno(fp))를 사용할 때에는 먼저 fflush(fp)의 성공을 확인한다. fseek는 성공 0, 실패 시 0이 아닌 값을 반환한다. 입력 버퍼 비우기 용도로 fflush(stdin)을 일반화하지 않는다.

참고: [fflush.3](https://man7.org/linux/man-pages/man3/fflush.3.html), [fclose.3](https://man7.org/linux/man-pages/man3/fclose.3.html), [fsync.2](https://man7.org/linux/man-pages/man2/fsync.2.html), [fseek.3](https://man7.org/linux/man-pages/man3/fseek.3.html)

## 46. 확인 문제 · 버퍼와 stdio

1. int 10개를 읽었을 때 fread의 반환값은?
2. char buf[4]에 "ABCD"와 NUL을 담을 수 있는가?
3. fread가 요청보다 적게 읽으면 무엇을 확인하는가?
4. fwrite 성공만으로 디스크 저장을 보장하는가?

교수자 메모: 정답: 1. size=sizeof(int), nmemb=10이면 10. 2. 불가능, 5바이트 필요. 3. ferror와 feof. 4. 보장하지 않는다. fflush와 fclose의 오류도 확인하며 지속성 요구는 별도로 다룬다.
