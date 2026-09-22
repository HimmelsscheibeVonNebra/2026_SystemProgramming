# 파일 입출력 · 텍스트와 바이너리

1부: read/write/lseek 복습. 2부: fread/fwrite/fseek 학습.

## 1. 파일 입출력 · 텍스트와 바이너리

1부 복습: read, write, lseek
2부 학습: fread, fwrite, fseek

## 2. 수업 구성

1부 · 텍스트 저장과 읽기: read, write, lseek
2부 · 바이너리 저장과 읽기: fread, fwrite, fseek
두 함수군 모두 텍스트와 바이너리를 처리한다.
예제의 저장 형식과 API의 차이를 구분한다.

교수자 메모: 학습 순서를 위해 텍스트를 FD 함수군, 바이너리를 FILE* 함수군으로 설명한다. read/write는 텍스트 전용, fread/fwrite는 바이너리 전용이라는 분류가 아니다. 핵심 차이는 FD와 스트림, 처리 단위, 라이브러리 버퍼링이다.

## 3. 1부 · 복습 · 텍스트 저장과 읽기

read(), write(), lseek()로 파일의 바이트와 위치를 확인합니다.

## 4. 텍스트 파일의 바이트

예제는 ASCII 문자열 ABCDEFGHIJ다.
파일에는 문자 10개의 바이트를 저장한다.
C 문자열 끝의 NUL은 이번 파일에 쓰지 않는다.
파일 위치는 다음에 읽거나 쓸 바이트의 위치다.

교수자 메모: ASCII 문자 A는 0x41이다. 텍스트도 바이트의 나열이다. UTF-8 한글은 한 문자와 한 바이트가 대응하지 않으므로 이 위치 예제에서는 ASCII만 사용한다.

## 5. open과 close 복습

open은 성공하면 0 이상의 FD를 반환한다.
O_TRUNC는 기존 파일 내용을 지운다.
사용이 끝난 FD는 close로 닫는다.

```c
int fd = open("text.txt",
    O_RDWR | O_CREAT | O_TRUNC, 0644);
if (fd == -1) {
    perror("open");
    return 1;
}
/* write, read, lseek 실행 */
if (close(fd) == -1) return 1;
```

교수자 메모: main의 흐름 발췌다. 헤더는 fcntl.h, unistd.h, stdio.h를 사용한다. 모든 중간 실패 경로에서도 close가 필요하다. O_RDWR은 읽기와 쓰기를 모두 허용한다.

참고: [open.2](https://man7.org/linux/man-pages/man2/open.2.html), [close.2](https://man7.org/linux/man-pages/man2/close.2.html)

## 6. write · 텍스트 저장

buf에서 count바이트를 파일에 쓴다.
반환값은 실제로 쓴 바이트 수다.
부분 쓰기이면 남은 바이트를 다시 쓴다.

```c
ssize_t write(int fd, const void *buf,
              size_t count);

const char text[] = "ABCDEFGHIJ";
ssize_t n = write(fd, text, sizeof text - 1);
/* 요청은 10바이트 */
/* 성공: 0 이상, 오류: -1 */
```

교수자 메모: 프로토타입과 호출 예시를 함께 표시했다. write가 4를 반환하면 다음 요청은 text+4에서 6바이트다. -1/EINTR은 재시도하며 그 외 오류와 진행 없는 0 반환도 처리한다. 완성 실습에서는 제공된 write_all을 사용한다.

참고: [write.2](https://man7.org/linux/man-pages/man2/write.2.html)

## 7. read · 텍스트 읽기

최대 count바이트를 버퍼에 저장한다.
양수는 읽은 길이, 0은 EOF다.
문자열로 쓸 때 NUL 공간을 남긴다.

```c
char buf[16];
ssize_t n = read(fd, buf, sizeof buf - 1);
if (n == -1) {
    /* 오류 처리 */
} else {
    buf[n] = '\0';
    printf("%s", buf);
}
```

교수자 메모: 일반 파일에 양수 크기로 읽는 예제다. read는 NUL을 자동으로 붙이지 않는다. 앞서 write했다면 현재 위치는 파일 끝이므로 읽기 전에 lseek로 되돌려야 한다. EOF까지 처리하려면 반복한다.

참고: [read.2](https://man7.org/linux/man-pages/man2/read.2.html)

## 8. lseek · 읽기와 쓰기 위치

offset은 기준점에서 이동할 바이트 수다.
성공하면 새 파일 위치를 반환한다.
실패하면 (off_t)-1을 반환한다.

```c
off_t lseek(int fd, off_t offset, int whence);

lseek(fd, 0, SEEK_SET); /* 파일 시작 */
lseek(fd, 3, SEEK_SET); /* 시작에서 3 */
lseek(fd, -2, SEEK_CUR); /* 현재에서 -2 */
lseek(fd, 0, SEEK_END); /* 파일 끝 */
```

교수자 메모: 각 호출은 독립적인 위치 예시이며 실제 코드에서는 반환값을 검사한다. lseek는 데이터를 읽거나 쓰지 않는다. 파일 위치는 바이트 단위다. 파이프 등에서는 사용할 수 없다. O_APPEND이면 각 write는 파일 끝에 기록한다.

참고: [lseek.2](https://man7.org/linux/man-pages/man2/lseek.2.html)

## 9. 텍스트 위치 예제

시작에서 3바이트 이동하면 D 위치다.
2바이트를 읽으면 DE, 다음 위치는 5다.
다시 위치 3에 XY를 덮어쓴다.

```c
/* text.txt: ABCDEFGHIJ, O_RDWR */
lseek(fd, 3, SEEK_SET);
read(fd, buf, 2);
/* 읽은 데이터: DE */

lseek(fd, 3, SEEK_SET);
write(fd, "XY", 2);
/* 파일 결과: ABCXYFGHIJ */
```

교수자 메모: 위치 흐름을 보여 주는 발췌이며 각 호출이 성공하고 요청한 바이트를 모두 처리했다고 가정한다. 실제 코드는 반환값을 검사하고 부분 읽기와 쓰기를 처리한다. 덮어쓰기는 기존 데이터를 뒤로 밀어 넣지 않는다.

참고: [read.2](https://man7.org/linux/man-pages/man2/read.2.html), [write.2](https://man7.org/linux/man-pages/man2/write.2.html), [lseek.2](https://man7.org/linux/man-pages/man2/lseek.2.html)

## 10. 1부 확인 · 반환값과 버퍼

write가 4를 반환하면 4바이트만 처리했다.
read가 2를 반환하면 읽은 2바이트를 사용한다.
char buf[4]의 유효 인덱스는 0~3이다.
끝 밖 쓰기는 overflow, 시작 앞 쓰기는 underwrite다.
데이터 부족을 뜻하는 underflow와 구별한다.

교수자 메모: read의 짧은 양수 반환만으로 EOF를 단정하지 않는다. 텍스트 ABCD를 C 문자열로 담으려면 NUL까지 5바이트가 필요하다. 스트리밍 underflow는 소비할 데이터가 부족한 상황이다. 메모리 경계 밖 읽기와 쓰기는 정의되지 않은 동작이다.

## 11. 2부 · 바이너리 저장과 읽기

fread(), fwrite(), fseek()로 정수 배열을 저장하고 읽습니다.

## 12. 텍스트 표현과 바이너리 표현

텍스트: 숫자 1234를 문자 "1234"로 저장
바이너리: 정수 값의 메모리 바이트를 저장
텍스트는 문자 해석과 숫자 변환이 필요하다.
바이너리는 자료형과 저장 형식을 알아야 한다.
이번 예제는 같은 환경에서 int 배열을 읽고 쓴다.

교수자 메모: ASCII 텍스트 1234는 줄바꿈 없이 4바이트다. int의 크기는 sizeof(int)로 확인한다. 바이너리가 항상 더 작다는 뜻은 아니다. int 크기, 엔디언, 구조체 패딩 때문에 원시 메모리 저장은 환경 간 교환 형식으로 적절하지 않을 수 있다.

## 13. FILE*와 fopen

FILE*는 C 라이브러리의 스트림이다.
wb는 새로 쓰기, rb는 읽기 모드다.
fopen 실패는 NULL로 확인한다.

```c
FILE *fp = fopen("numbers.bin", "wb");
if (fp == NULL) {
    perror("fopen");
    return 1;
}
/* fwrite로 저장 */
if (fclose(fp) == EOF) return 1;
```

교수자 메모: stdio.h를 포함한다. wb는 기존 파일 내용을 지운다. rb는 기존 파일을 읽고, r+b는 기존 파일의 읽기와 쓰기를 허용한다. Linux에서는 b가 동작을 바꾸지 않지만 다른 환경의 바이너리 모드 구분을 위해 명시한다.

참고: [fopen.3](https://man7.org/linux/man-pages/man3/fopen.3.html), [fclose.3](https://man7.org/linux/man-pages/man3/fclose.3.html)

## 14. fwrite · 바이너리 저장

size는 항목 하나의 크기다.
nmemb는 저장할 항목 개수다.
반환값은 성공적으로 쓴 항목 수다.

```c
/* fwrite(ptr, size, nmemb, stream) */
int values[] = {10, 20, 30};

size_t n = fwrite(values,
                 sizeof values[0], 3, fp);
if (n != 3) {
    /* 쓰기 오류 처리 */
}
/* 요청 크기: 3 * sizeof(int) */
```

교수자 메모: 메모리에서 fp로 바이트를 기록한다. 반환형은 size_t다. 요청보다 적은 항목을 반환하면 쓰기 오류를 처리한다. 성공 후에도 fclose에서 버퍼를 내보내다 실패할 수 있다.

참고: [fread.3](https://man7.org/linux/man-pages/man3/fread.3.html)

## 15. fread · 바이너리 읽기

같은 항목 크기와 개수로 읽는다.
n이 3이면 int 3개를 완전히 읽었다.
부족하면 EOF와 오류를 구별한다.

```c
/* fp: numbers.bin을 rb로 연 스트림 */
int values[3];
size_t n = fread(values,
                 sizeof values[0], 3, fp);
if (n != 3) {
    if (ferror(fp)) { /* 읽기 오류 */ }
    else { /* 파일 내용이 부족함 */ }
}
/* n == 3일 때 값: 10, 20, 30 */
```

교수자 메모: fread(ptr,size,nmemb,stream)의 ptr에 결과를 저장한다. 완전히 읽은 항목만 반환 개수에 포함된다. 불완전한 마지막 항목을 유효한 int 값으로 사용하지 않는다. size가 1일 때는 반환값이 읽은 바이트 수다.

참고: [fread.3](https://man7.org/linux/man-pages/man3/fread.3.html), [ferror.3](https://man7.org/linux/man-pages/man3/ferror.3.html)

## 16. 항목 수와 바이트 수

fread(values, sizeof(int), 3, fp)의 단위는 int다.
fread(buf, 1, sizeof buf, fp)의 단위는 바이트다.
요청 바이트 수는 size × nmemb다.
요청 전체를 담을 버퍼 공간이 필요하다.
fread는 문자열 끝의 NUL을 붙이지 않는다.

교수자 메모: 예를 들어 sizeof(int)==4이고 세 항목을 모두 읽으면 전송량 12바이트, 반환값 3이다. size 곱셈의 오버플로도 주의한다. fwrite에서도 버퍼 크기보다 많이 요청하면 범위 밖 메모리를 읽게 된다.

참고: [fread.3](https://man7.org/linux/man-pages/man3/fread.3.html)

## 17. fseek · 바이너리 파일 위치

offset은 항목 개수가 아닌 바이트 수다.
SEEK_SET, SEEK_CUR, SEEK_END를 사용한다.
성공은 0, 실패는 0이 아닌 값이다.

```c
int fseek(FILE *stream, long offset,
          int whence);

/* 두 번째 int: 인덱스 1 */
long offset = (long)sizeof(int);
if (fseek(fp, offset, SEEK_SET) != 0) {
    /* 위치 이동 오류 처리 */
}
```

교수자 메모: Linux 일반 바이너리 파일 기준이다. fseek는 새 위치를 반환하지 않는다. 현재 위치가 필요하면 ftell(fp)를 호출하고 -1L 오류를 확인한다. 성공한 fseek는 EOF 표시를 해제한다. 대용량 파일에서는 off_t를 쓰는 fseeko/ftello도 고려한다.

참고: [fseek.3](https://man7.org/linux/man-pages/man3/fseek.3.html)

## 18. 원하는 항목 하나 읽기

인덱스 k의 위치는 k × 항목 크기다.
두 번째 값은 인덱스 1의 20이다.
fread 반환값 1을 확인한 뒤 사용한다.

```c
/* fp: numbers.bin을 rb로 연 스트림 */
int value;
long offset = (long)sizeof value;
if (fseek(fp, offset, SEEK_SET) != 0)
    goto fail;
if (fread(&value, sizeof value, 1, fp) != 1)
    goto fail;
printf("%d\n", value); /* 20 */
```

교수자 메모: 오류 처리 레이블 fail에서 fclose 후 실패 상태를 반환하는 발췌다. 인덱스는 0부터 시작한다. sizeof(int)==4인 환경에서는 두 번째 int의 시작 위치가 4바이트다.

참고: [fseek.3](https://man7.org/linux/man-pages/man3/fseek.3.html), [fread.3](https://man7.org/linux/man-pages/man3/fread.3.html)

## 19. 원하는 항목 하나 수정

기존 파일을 r+b 모드로 연다.
두 번째 int 위치에 99를 덮어쓴다.
수정 후 배열은 10, 99, 30이다.

```c
FILE *fp = fopen("numbers.bin", "r+b");
if (!fp) return 1;
int value = 99;
if (fseek(fp, (long)sizeof value, SEEK_SET) != 0)
    goto fail;
if (fwrite(&value, sizeof value, 1, fp) != 1)
    goto fail;
if (fclose(fp) == EOF) return 1;
```

교수자 메모: fail 레이블은 스트림을 닫고 실패를 반환하는 공통 경로다. 성공 시 main이 0을 반환하도록 구성한다. r+b는 기존 내용을 보존하고 wb는 내용을 지운다. 배열 파일에는 헤더가 없고 int가 연속 저장되어 있다고 가정한다.

참고: [fopen.3](https://man7.org/linux/man-pages/man3/fopen.3.html), [fseek.3](https://man7.org/linux/man-pages/man3/fseek.3.html), [fread.3](https://man7.org/linux/man-pages/man3/fread.3.html)

## 20. 읽기와 쓰기 전환

같은 FILE*로 읽고 쓸 때 전환 규칙이 있다.
쓰기 후 읽기: fflush 또는 위치 지정이 필요하다.
읽기 후 쓰기: 보통 위치 지정이 필요하다.
이 수업에서는 fseek로 다음 위치를 지정한다.
fseek 성공 여부를 확인한 뒤 다음 작업을 한다.

교수자 메모: 업데이트 스트림 규칙이다. 읽기 후 쓰기는 앞선 읽기가 EOF를 만난 경우에 예외가 있다. fseek가 0바이트 이동이어도 위치 지정 호출의 역할을 한다. 라이브러리 버퍼와 위치가 있으므로 같은 파일에 FD 함수와 stdio 함수를 무분별하게 섞지 않는다.

참고: [fopen.3](https://man7.org/linux/man-pages/man3/fopen.3.html), [fseek.3](https://man7.org/linux/man-pages/man3/fseek.3.html)

## 21. 버퍼링과 파일 닫기

stdio는 라이브러리 버퍼로 입출력을 모을 수 있다.
fwrite 성공 뒤에도 출력 버퍼에 남을 수 있다.
fflush는 출력 버퍼의 데이터를 커널로 내보낸다.
fclose는 출력 버퍼를 처리하고 파일을 닫는다.
두 함수 모두 성공 0, 실패 EOF를 검사한다.

교수자 메모: fflush만으로 디스크 영구 저장을 보장하지 않는다. 지속성 요구에는 fflush 성공 후 fsync(fileno(fp)) 등을 별도로 고려한다. 입력 버퍼가 비면 라이브러리가 다시 채울 수 있으며, 빈 버퍼 자체가 EOF를 뜻하지 않는다.

참고: [fflush.3](https://man7.org/linux/man-pages/man3/fflush.3.html), [fclose.3](https://man7.org/linux/man-pages/man3/fclose.3.html)

## 22. 실습 · 저장, 선택 읽기, 수정

1. int 배열 {10, 20, 30}을 fwrite로 저장한다.
2. rb로 열고 fread로 전체 배열을 확인한다.
3. fseek로 두 번째 항목을 읽어 20을 확인한다.
4. r+b로 열어 두 번째 항목을 99로 바꾼다.
5. 다시 읽어 {10, 99, 30}인지 확인한다.

교수자 메모: 각 fopen, fread, fwrite, fseek, fclose의 반환값을 검사한다. 빈 파일 또는 일부 바이트만 있는 파일에서는 완전한 세 항목 읽기가 실패해야 한다. 원시 int 바이트를 텍스트 편집기에서 읽을 수 있을 것으로 기대하지 않는다.

## 23. 두 함수군의 대응

파일 열기와 닫기: open / close, fopen / fclose
읽기와 쓰기: read / write, fread / fwrite
위치 지정: lseek는 새 위치, fseek는 성공 여부 반환
파일 표현: 정수 FD, FILE* 스트림
저장 형식은 프로그램이 기록하는 바이트로 결정한다.

교수자 메모: read/write는 바이트 수, fread/fwrite는 항목 수를 반환한다. 본 수업에서는 텍스트 예제로 저수준 함수를 복습하고 바이너리 예제로 stdio를 학습했다. 두 함수군 모두 텍스트와 바이너리에 사용할 수 있다.

## 24. 정리 퀴즈

1. write에 정수 주소를 전달하면 무엇을 저장하는가?
2. int 3개를 쓴 fwrite의 성공 반환값은?
3. 세 번째 int의 시작 위치는 몇 바이트인가?
4. fseek 성공 시 반환값과 현재 위치는 같은가?

교수자 메모: 정답: 1. count로 지정한 메모리 바이트다. 자동으로 숫자 문자열로 바꾸지 않는다. 2. size=sizeof(int), nmemb=3이면 3. 3. 2*sizeof(int). 4. 다르다. fseek 성공은 0이며 현재 위치는 ftell로 확인한다.
