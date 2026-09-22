# Class03-02 · 견고한 I/O와 파일 복사

교안: [ODP](../../Presentations/class03-02.odp) · [PDF](../../Presentations/class03-02.pdf). 저장소 루트에서 `cd Codes/class03-02`로 이동합니다.

120분 수업: 복습 15분, read 20분, write_all 25분, 휴식 10분, 복사·테스트 40분, 퀴즈·제출 10분.
기존 Ubuntu 일반 사용자 계정에서 진행한다. C 컴파일러, make, Python 3가 필요하다. strace는 Linux 선택 실습이다.

## 파일 구성

- `read_chunks.c`: 4바이트씩 읽는다. 데이터는 stdout, 반환값은 stderr로 보낸다.
- `io_helpers.h`: 지난 차시의 write_all 완성본.
- `student/io_helpers.h`: 직접 구현할 파일. 수정 전 테스트 실패가 정상이다.
- `file_copy.c`: 4096바이트 버퍼로 새 파일에 복사한다.
- `test_write_all.c`: 가짜 write로 부분 성공·EINTR·0·오류를 재현한다.
- `test_copy.py`: 실제 프로그램의 바이트 결과와 오류 경로를 비교한다.

## 1. 읽기 길이 관찰

이 폴더에서 실행하고 같은 셸의 RUN 변수를 계속 사용한다.

```sh
make
RUN=$(mktemp -d ./run.XXXXXX)
printf ABCDEFGHIJ > "$RUN/in"
./read_chunks "$RUN/in" > "$RUN/out"
cmp "$RUN/in" "$RUN/out"
echo $?
```

표준 오류에는 4, 4, 2, 0이 나타난다. 반환 2도 성공이다. `cmp` 직후 상태 0은 내용이 같다는 뜻이다.
빈 파일을 생성해 비교하고, read_chunks의 buf 크기를 3으로 바꿔 다시 빌드한다. 관찰 후 4로 복원한다.

## 2. write_all 직접 구현

`student/io_helpers.h`의 TODO를 구현한다. 작은 버퍼와 blocking FD를 대상으로 한다.
성공하면 0, 실패하면 errno와 -1을 전달한다. 길이 0이면 write를 호출하지 않는다.
양수 반환만큼 포인터를 옮기고 남은 길이를 줄인다. -1/EINTR이면 같은 위치에서 재시도한다.
그 밖의 오류는 중단한다. 양수 길이 요청의 반환이 0이면 EIO로 처리한다.

```sh
make test-student
```

테스트 실패 메시지의 조건을 읽고 수정한다. 통과 후 완성본과 비교한다.
테스트는 시스템 호출 대신 가짜 함수를 사용한다. 실제 신호 발생이나 디스크 부족을 재현한 결과는 아니다.
무한 반복을 작성했다면 Ctrl-C로 종료하고 0/EINTR 분기를 확인한다.

## 3. 파일 복사

```sh
./file_copy "$RUN/in" "$RUN/copy"
cmp "$RUN/in" "$RUN/copy"
echo $?
./file_copy "$RUN/in" "$RUN/copy"
echo $?
make test
```

첫 복사는 성공한다. 두 번째는 기존 목적지를 보존하고 상태 1로 실패한다.
매 실험에 새로운 목적지 이름을 사용한다. O_EXCL 정책 때문에 같은 경로와 기존 링크도 거부한다.
권한·시간 등 메타데이터는 복제하지 않는다. 실패하면 부분 목적지 파일이 남을 수 있다.
성공한 write/close가 정전에 대비한 영구 저장까지 보장하지는 않는다.

학생 구현으로 복사기를 실행하려면 다음을 사용한다.

```sh
make file_copy_student
./file_copy_student "$RUN/in" "$RUN/student-copy"
cmp "$RUN/in" "$RUN/student-copy"
python3 test_copy.py file_copy_student
```

## 4. Linux에서 호출 관찰

```sh
strace -o "$RUN/trace.log" \
  -e trace=openat,read,write,close \
  ./file_copy "$RUN/in" "$RUN/traced"
cat "$RUN/trace.log"
cmp "$RUN/in" "$RUN/traced"
```

입력·출력 경로가 열린 FD를 먼저 찾는다. 라이브러리 로딩의 호출도 섞여 있다.
일반 파일에서는 부분 쓰기가 관찰되지 않을 수 있다. 부분 쓰기 분기는 제공된 가짜 write 테스트로 검증한다.
Linux에서는 다음으로 실제 쓰기 오류도 관찰할 수 있다.

```sh
./read_chunks "$RUN/in" > /dev/full
echo $?
```

예상 종료 상태는 1이다. file_copy는 새 파일만 만들므로 /dev/full을 목적지로 쓰는 실습에는 read_chunks를 사용한다.

## 제출

`student/io_helpers.h`, 작성한 `REPORT.md`, 함수 테스트 및 복사 비교 결과를 제출한다.
기존 파일의 권한을 바꾸거나 관리자 권한으로 실행할 필요는 없다.

## 참고

- [read(2)](https://man7.org/linux/man-pages/man2/read.2.html)
- [write(2)](https://man7.org/linux/man-pages/man2/write.2.html)
- [open(2)](https://man7.org/linux/man-pages/man2/open.2.html)
- [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)
