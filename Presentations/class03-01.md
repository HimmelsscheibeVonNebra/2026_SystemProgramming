# Week03-class01 · 복습과 파일 프로그램 4개

## 1. 저수준 파일 입출력

WEEK 03 · CLASS 01

복습과 파일 프로그램 4개

시스템프로그래밍

## 2. 오늘의 구성 · 120분

섹션 1  지난 시간 복습 · 15분

섹션 2  파일 생성 · 15분

섹션 3  파일 쓰기 · 25분

휴식 · 10분

섹션 4  파일 읽기 · 20분

섹션 5  지정 위치 수정 · 25분

퀴즈와 제출 · 10분

## 3. 섹션 1 · 지난 시간 복습

C 소스를 컴파일하고 프로그램 실행

Makefile로 빌드 과정 반복

표준 출력 리다이렉션과 종료 상태 확인

## 4. 컴파일과 Makefile

소스 파일과 실행 파일을 구별한다.

make는 Makefile의 규칙을 실행한다.

소스를 수정하면 다시 빌드한다.

```sh
cc -Wall -Wextra -g helloworld.c -o helloworld
./helloworld
make
```

교수자 메모: 지난 시간의 Hello World 폴더에서 먼저 실행한다. 이번 차시 Makefile도 비교한다.

## 5. 표준 출력과 종료 상태

>는 파일을 새로 만들거나 비운다.

>>는 파일 끝에 추가한다.

$?는 직전 명령의 종료 상태다.

```sh
./helloworld > hello.txt
./helloworld >> hello.txt
cat hello.txt
echo $?
```

교수자 메모: 여기의 $?는 바로 앞 cat의 종료 상태다. Hello World의 종료 상태를 보려면 실행 직후 echo $?를 실행하게 한다.

## 6. 섹션 2 · 파일 생성

프로그램 1: file_create

새로운 빈 파일을 만든다.

이미 존재하는 파일은 보존하고 실패한다.

핵심 함수: open, close

## 7. 경로, FD, 열린 파일 상태

경로: open에 전달하는 파일 이름

FD: 프로세스가 사용하는 0 이상의 정수

열린 파일 상태: 현재 오프셋과 상태 flags

read/write는 경로 대신 FD를 사용한다

교수자 메모: 한 파일을 두 번 open하면 별도의 열린 파일 상태가 생긴다. dup/fork의 공유는 후속 차시에 설명한다.

## 8. 표준 입력·출력·오류

0  STDIN_FILENO    표준 입력

1  STDOUT_FILENO   표준 출력

2  STDERR_FILENO   표준 오류

보통 터미널에 연결되지만 리다이렉션할 수 있다.

새 FD가 반드시 3이라는 보장은 없다.

## 9. open과 close

성공한 open은 FD를 반환한다.

실패는 -1이며 perror로 이유를 출력한다.

close는 성공 0, 실패 -1이다.

사용을 끝낸 FD는 닫는다.

```c
int fd = open("sample.txt", O_RDONLY);
if (fd == -1) {
    perror("open");
    return 1;
}
/* fd를 사용하는 작업 */
if (close(fd) == -1) {
    perror("close");
    return 1;
}
```

## 10. 생성 권한과 umask

0666은 rw-rw-rw-를 요청한다.

기본 ACL이 없으면 mode & ~umask를 적용한다.

umask 022이면 0644

umask 027이면 0640

mode는 이미 존재하는 파일의 권한을 바꾸지 않는다.

```c
int fd = open("note.txt",
    O_WRONLY | O_CREAT | O_EXCL,
    0666);
```

교수자 메모: 8진수 표기 0에 주의. O_CREAT 사용 시 mode를 반드시 전달한다. 심화: O_TMPFILE에도 mode가 필요하다.

## 11. file_create의 핵심 코드

O_CREAT와 O_EXCL을 함께 사용한다.

생성 권한은 0666을 요청한다.

기록하지 않고 닫으면 빈 파일이 남는다.

```c
int fd = open(argv[1],
    O_WRONLY | O_CREAT | O_EXCL, 0666);
if (fd == -1) {
    perror(argv[1]); return 1;
}
if (close(fd) == -1) {
    perror("close"); return 1;
}
```

## 12. 실습 1 · 빈 파일 만들기

같은 파일을 두 번 생성한다.

두 번째 실행은 종료 상태 1이다.

파일 크기와 권한도 확인한다.

```sh
make
RUN=$(mktemp -d ./run.XXXXXX)
./file_create "$RUN/note.txt"
wc -c < "$RUN/note.txt"
./file_create "$RUN/note.txt"
echo $?
ls -l "$RUN/note.txt"
```

## 13. 섹션 3 · 파일 쓰기

프로그램 2: file_write

문자열을 덮어쓰거나 끝에 추가한다.

핵심 함수: open, write, close

## 14. 파일의 내용은 바이트의 나열

텍스트 파일도 바이트로 저장한다.

개행도 저장 공간을 차지한다.

C 문자열 끝의 NUL은 자동 저장되지 않는다.

한글의 문자 수와 바이트 수는 다를 수 있다.

```sh
printf 'ABC' > sample.txt
wc -c < sample.txt
od -An -tx1 sample.txt
```

## 15. 접근 모드와 추가 flags

접근 모드는 아래 중 하나를 선택한다.

O_RDONLY: 읽기

O_WRONLY: 쓰기

O_RDWR: 읽기와 쓰기

추가 flags는 비트 OR 연산자 |로 결합한다.

교수자 메모: O_RDONLY | O_WRONLY로 읽기/쓰기를 표현하지 않는다. Linux에서 O_RDONLY는 0이다.

## 16. 생성과 기록 방식

O_CREAT: 파일이 없으면 생성

O_EXCL + O_CREAT: 이미 있으면 실패

O_TRUNC: 쓰기용으로 연 일반 파일의 길이를 0으로

O_APPEND: 매 write를 파일 끝에 기록

교수자 메모: O_TRUNC는 open 성공 시 기존 내용을 없앤다. 쓰기를 한 번도 하지 않아도 데이터가 사라질 수 있다.

## 17. write와 부분 쓰기

write의 결과는 실제 기록한 바이트 수다.

요청한 길이보다 적게 기록할 수 있다.

남은 위치와 길이를 갱신해 다시 쓴다.

이번에는 제공한 write_all을 사용한다.

다음 시간에는 반복문을 직접 구현한다.

교수자 메모: write_all은 blocking FD용이다. EINTR를 재시도하고 0바이트 쓰기는 EIO로 처리해 무한 반복을 피한다. write 성공이 영구 저장 완료를 보장하지 않는다.

## 18. 파일 기록기의 핵심

flags가 기록 방식을 결정한다.

strlen은 끝의 NUL을 제외한다.

쓰기 실패 뒤에도 FD를 닫는다.

```c
int fd = open(path, flags, 0666);
if (fd == -1) { perror(path); return 1; }
int status = 0;
if (write_all(fd, text, strlen(text)) == -1) {
    perror("write"); status = 1;
}
if (close(fd) == -1) {
    perror("close"); status = 1;
}
return status;
```

교수자 메모: file_write.c 핵심 발췌. 변수 path/text는 완성 코드에서 argv[2]/argv[3]이다. Linux에서는 close 실패를 일괄 재시도하지 않는다.

## 19. 실습 2 · 덮어쓰기와 추가

truncate는 원래 내용을 비운다.

append는 기존 내용을 유지한다.

최종 내용과 바이트 수를 예상한다.

```sh
./file_write truncate "$RUN/note.txt" ABCDEFGHIJ
./file_write truncate "$RUN/note.txt" XY
./file_write append "$RUN/note.txt" ZZ
cat "$RUN/note.txt"; printf '\n'
wc -c < "$RUN/note.txt"
```

교수자 메모: XYZZ, 4바이트. O_TRUNC를 제거하는 실험은 선택 과제다.

## 20. 섹션 4 · 파일 읽기

프로그램 3: file_read

파일을 읽어 표준 출력에 기록한다.

핵심 함수: open, read, write, close

## 21. read의 반환값

n > 0: 실제로 읽은 바이트 수

n == 0: 일반 파일의 EOF

n == -1: 오류

요청보다 적게 읽어도 다음 read를 계속한다.

```c
unsigned char buf[16];
ssize_t n = read(fd, buf, sizeof buf);
```

교수자 메모: count > 0이고 일반 파일을 읽는 수업 상황이다. count == 0 요청의 반환 0을 EOF로 해석하지 않는다.

## 22. 읽은 바이트만 사용하기

read는 문자열 끝에 NUL을 붙이지 않는다.

버퍼 크기와 실제 읽은 길이를 구별한다.

바이너리 파일에도 같은 방식으로 적용한다.

```c
/* n > 0인 경우 */
write_all(STDOUT_FILENO, buf, (size_t)n);

/* 피해야 할 가정 */
printf("%s", buf);
/* NUL 종료가 보장되지 않는다. */
```

교수자 메모: write_all은 제공된 함수. 반환값 검사는 다음 슬라이드와 완성 소스에서 수행한다.

## 23. EOF까지 반복 읽기

오류와 EOF는 서로 다른 종료 경로다.

EINTR는 읽기 중 신호로 중단된 경우다.

완성 소스는 EINTR 때 다시 읽는다.

```c
for (;;) {
    ssize_t n = read(fd, buf, sizeof buf);
    if (n == 0) break;
    if (n == -1) {
        if (errno == EINTR) continue;
        perror("read"); status = 1; break;
    }
    if (write_all(1, buf, (size_t)n) == -1) {
        perror("write"); status = 1; break;
    }
}
```

## 24. 실습 3 · 버퍼와 EOF

16바이트 버퍼로 32바이트 파일을 읽는다.

빈 파일과 없는 파일도 비교한다.

버퍼를 7바이트로 바꿔 결과를 관찰한다.

```sh
printf '0123456789abcdefghijklmnopqrstuv' > "$RUN/long.txt"
./file_read "$RUN/long.txt" > "$RUN/copy.txt"
cmp "$RUN/long.txt" "$RUN/copy.txt"
./file_create "$RUN/empty.txt"
./file_read "$RUN/empty.txt"; echo $?
./file_read "$RUN/missing.txt"; echo $?
```

## 25. 섹션 5 · 지정 위치 수정

프로그램 4: file_seek

지정 위치의 내용을 읽고 일부 바이트를 바꾼다.

핵심 함수: lseek, read, write

문자열 삽입이 아니라 기존 바이트 덮어쓰기

## 26. lseek와 세 가지 기준

SEEK_SET: 파일 시작 기준

SEEK_CUR: 현재 위치 기준

SEEK_END: 파일 끝 기준

성공하면 새 위치, 실패하면 (off_t)-1

위치의 단위는 바이트이며 첫 위치는 0이다.

```c
off_t pos = lseek(fd, 3, SEEK_SET);
if (pos == (off_t)-1) {
    perror("lseek");
}
```

## 27. 오프셋 3에 XY를 쓰면

변경 전: ABCDEFGHIJ

위치:     0123456789

변경 후: ABCXYFGHIJ

D와 E를 바꾸며 파일 크기는 10바이트다.

한글은 여러 바이트일 수 있으므로 ASCII로 실습한다.

## 28. 읽은 뒤에는 위치가 이동한다

위치 3에서 읽으면 오프셋이 전진한다.

수정 전에 다시 위치 3으로 이동한다.

O_RDWR로 읽기와 쓰기를 허용한다.

O_APPEND와 O_TRUNC는 사용하지 않는다.

```c
lseek(fd, offset, SEEK_SET);
n = read(fd, buf, sizeof buf);

/* 원래 수정할 위치로 복귀 */
lseek(fd, offset, SEEK_SET);
write_all(fd, text, strlen(text));
```

교수자 메모: 흐름을 보여 주는 발췌 코드. 완성 코드에서는 lseek/read/write_all의 반환값을 모두 검사한다. O_APPEND이면 lseek 후에도 write는 끝에 기록한다.

## 29. 실습 4 · 지정 위치 수정

수정 전 일부 내용을 표준 출력에 표시한다.

다시 파일 전체를 읽어 결과를 확인한다.

오프셋 0과 파일 끝 위치도 실험한다.

```sh
./file_write truncate "$RUN/note.txt" ABCDEFGHIJ
./file_seek "$RUN/note.txt" 3 XY
printf '\n'
./file_read "$RUN/note.txt"; printf '\n'
wc -c < "$RUN/note.txt"
```

교수자 메모: 수정 전 출력 DEFGHIJ, 수정 후 ABCXYFGHIJ. 명령의 TEXT에는 개행이나 NUL을 자동 추가하지 않는다.

## 30. 위치 이동과 파일 크기

lseek만으로 파일 크기가 늘어나지는 않는다.

파일 끝에서 쓰면 길이가 늘어난다.

끝 너머에서 쓰면 사이 영역은 읽을 때 0이다.

파이프처럼 위치 이동이 불가능한 대상도 있다.

교수자 메모: 선택 실습: 10바이트 파일의 offset 12에 Z 기록, 크기 13. od로 00 00 확인. offset 인자는 음수/잘못된 숫자를 거부한다.

## 31. 확인 퀴즈

1. open의 실패 반환값은?

2. 빈 파일과 없는 파일은 어떻게 구별하는가?

3. O_TRUNC와 O_APPEND의 차이는?

4. lseek(fd, 0, SEEK_CUR)는 무엇을 반환하는가?

5. read 후 같은 위치에 쓰려면 무엇이 필요한가?

교수자 메모: 정답: -1 / 빈 파일은 read 0, 없는 파일은 open 실패 / 길이 0으로 초기화와 끝에 쓰기 / 현재 오프셋, 실패 -1 / 다시 lseek. 일반 파일 count>0 전제.

## 32. 제출과 다음 시간

제출: C 프로그램 4개와 REPORT.md

생성·쓰기·읽기·지정 위치 수정 결과 기록

수정 과제: 버퍼 크기와 위치 이동 기준 변경

다음 시간: 부분 쓰기 반복문과 파일 복사기