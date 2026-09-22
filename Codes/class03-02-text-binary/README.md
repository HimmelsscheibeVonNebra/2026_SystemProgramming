# 텍스트와 바이너리 파일 입출력 실습

교안: [ODP](../../Presentations/class03-02-text-binary.odp) · [PDF](../../Presentations/class03-02-text-binary.pdf). 저장소 루트에서 `cd Codes/class03-02-text-binary`로 이동합니다.

발표자료 `class03-02-text-binary.odp/pdf`에 대응하는 실행 가능한 예제입니다.

## 구성

| 수업 | 파일 | 사용 함수 |
| --- | --- | --- |
| 1부 복습 | file_write.c | open, write, close |
| 1부 복습 | file_read.c | open, read, close |
| 1부 복습 | file_seek.c | lseek, read, write |
| 2부 | binary_write.c | fopen, fwrite, fclose |
| 2부 | binary_read.c | fopen, fread, fclose |
| 2부 | binary_seek.c | fseek, fread, fwrite |

두 함수군 모두 텍스트와 바이너리를 처리할 수 있습니다. 수업에서는 텍스트 예제로 FD 함수를 복습하고 바이너리 예제로 FILE* 함수를 학습합니다.

## 빌드와 테스트

C 컴파일러, make, Python 3가 필요합니다. 일반 사용자로 실행합니다.

```sh
make
make test
```

기본 옵션은 `-std=c17 -Wall -Wextra -Wpedantic -Werror -g`입니다.
`test_write_all.c`는 부분 쓰기, EINTR, 진행 없는 0 반환, 쓰기 오류를 가짜 write로 재현합니다.
`test_io.py`는 임시 폴더에서 실제 프로그램의 종료 상태와 파일 바이트를 비교합니다.
Linux에서는 `/dev/full`을 사용한 쓰기 실패 검사도 실행합니다.

## 1부: 텍스트

```sh
RUN=$(mktemp -d ./run.XXXXXX)
./file_write truncate "$RUN/text.txt" ABCDEFGHIJ
./file_read "$RUN/text.txt"
./file_seek "$RUN/text.txt" 3 XY
./file_read "$RUN/text.txt"
```

최종 파일은 `ABCXYFGHIJ`입니다. 파일에 줄바꿈이나 NUL을 추가하지 않습니다.
발표자료는 위치 설명을 위해 2바이트 `DE`를 읽습니다. 기존 실습을 재사용한 `file_seek`는 수정 전 데이터를 최대 16바이트 읽어 `DEFGHIJ`를 출력합니다. 두 예제 모두 다시 위치 3으로 이동한 뒤 XY를 씁니다.

## 2부: 바이너리

```sh
./binary_write "$RUN/numbers.bin"
./binary_read "$RUN/numbers.bin"
./binary_seek "$RUN/numbers.bin" 1
./binary_seek "$RUN/numbers.bin" 1 99
./binary_read "$RUN/numbers.bin"
```

순서대로 전체 값 `10 20 30`, 선택한 값 `20`, 수정한 값 `99`, 전체 값 `10 99 30`을 출력합니다.

- 파일 형식: 헤더 없이 native int 3개, 총 `3 * sizeof(int)`바이트
- `binary_write`: wb로 열어 기존 내용을 지우고 10, 20, 30 저장
- `binary_read`: 완전한 int 3개를 읽고 출력. 부족하거나 뒤에 데이터가 더 있으면 오류
- `binary_seek FILE INDEX`: 0~2 중 한 항목 선택 읽기
- `binary_seek FILE INDEX VALUE`: r+b로 열어 해당 항목 수정
- 수정 전 파일 크기와 완전한 항목 수를 확인하므로 잘린 파일, 추가 데이터, 잘못된 인덱스는 수정하지 않음
- 반환 상태: 성공 0, 파일·입출력 오류 1, 인자 오류 2

발표자료의 핵심 호출에 인자 검증, 실패 후 fclose, 출력 오류 검사를 추가했습니다.
같은 환경에서 만든 파일을 사용하는 예제입니다. int 크기와 바이트 순서가 다른 환경의 교환 형식으로 사용하지 않습니다.

## Linux에서 호출 관찰

```sh
strace -o "$RUN/text.trace" -e trace=openat,read,write,lseek,close \
  ./file_seek "$RUN/text.txt" 3 XY
strace -o "$RUN/binary.trace" -e trace=openat,read,write,lseek,close \
  ./binary_seek "$RUN/numbers.bin" 1 99
```

strace에는 fread/fwrite/fseek 자체가 아니라 이 함수들이 내부에서 호출하는 read/write/lseek가 보입니다. stdio 버퍼링 때문에 호출 횟수, 요청 길이, 위치 이동이 소스의 호출과 일대일로 대응하지 않을 수 있습니다.

검증 환경과 결과는 [TEST_RESULTS.md](TEST_RESULTS.md)를 참고하세요.
