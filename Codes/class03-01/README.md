# Week03-class01 · 복습과 파일 프로그램 4개

교안: [ODP](../../Presentations/class03-01.odp) · [PDF](../../Presentations/class03-01.pdf). 저장소 루트에서 `cd Codes/class03-01`로 이동합니다.

Ubuntu 일반 사용자 계정과 지난 시간의 C 컴파일러·Make를 사용한다.
완성 예제를 읽고 실행한 뒤 각 단계의 수정 과제를 수행한다.
`write_all`은 제공된 보조 함수로 사용하며 다음 차시에 직접 구현한다.

## 섹션 1. 지난 시간 복습 (10분)

지난 시간 Hello World 폴더에서 컴파일·실행한다.

```sh
cc -Wall -Wextra -g helloworld.c -o helloworld
./helloworld
echo $?
./helloworld > hello.txt
./helloworld >> hello.txt
cat hello.txt
make
```

소스와 실행 파일, `>`와 `>>`, Makefile의 역할을 설명한다.
`echo $?`는 직전 명령의 종료 상태를 확인한다. 다른 명령을 실행하면 값이 바뀐다.

이후 이 자료의 `Codes/class03-01` 폴더로 이동한다.

```sh
make
RUN=$(mktemp -d ./run.XXXXXX)
```

매번 새 실습 디렉터리를 사용한다. 이후 명령은 같은 셸에서 실행한다.

## 섹션 2. 파일 이해와 file_create (25분)

먼저 약 10분 동안 파일의 내용, 메타데이터, 이름과 경로, 파일 종류를 구별한다.
일반 파일의 내용은 바이트의 나열이고, 텍스트와 이미지는 바이트를 해석하는 방식이 다르다.
0바이트 파일도 존재하며 권한과 소유자 등의 메타데이터를 가진다.
디렉터리는 이름과 파일의 연결을 관리한다. 이후 15분 동안 빈 파일 생성 프로그램을 실행한다.

교수자 관찰 시연:

```sh
DEMO=$(mktemp -d)
printf 'ABC\n' > "$DEMO/note.txt"
cat "$DEMO/note.txt"
wc -c < "$DEMO/note.txt"
od -An -tx1 "$DEMO/note.txt"
ls -l "$DEMO/note.txt"
stat "$DEMO/note.txt"
```

예상 크기는 4바이트이고, 16진수로 41 42 43 0a이다. 개행도 파일 내용이다.
cat과 od는 내용을, ls -l과 stat은 파일 정보를 보여 준다. 권한과 소유자는 환경에 따라 다르다.


사용법: `./file_create FILE`

```sh
./file_create "$RUN/note.txt"
echo $?  # 0
wc -c < "$RUN/note.txt"  # 0
./file_create "$RUN/note.txt"
echo $?  # 1, 기존 파일은 유지
```

`open(..., O_WRONLY | O_CREAT | O_EXCL, 0666)`으로 열고 `close`한다.
실패는 -1이다. FD 0도 유효한 성공 값이다.

```sh
(umask 022; ./file_create "$RUN/mask022.txt")
(umask 027; ./file_create "$RUN/mask027.txt")
ls -l "$RUN"/mask*.txt
```

기본 ACL이 없는 디렉터리에서 권한은 각각 0644, 0640이다.
`mode & ~umask`는 비트를 제거하는 연산이다.

수정 과제: 성공 시 fd 값을 출력한 뒤 다시 빌드한다. 보통 3이지만 항상 3은 아니다.

## 섹션 3. file_write: 덮어쓰기와 추가 (20분)

사용법: `./file_write truncate|append FILE TEXT`

```sh
./file_write truncate "$RUN/note.txt" ABCDEFGHIJ
./file_write truncate "$RUN/note.txt" XY
cat "$RUN/note.txt"; printf '\n'  # XY
./file_write append "$RUN/note.txt" ZZ
cat "$RUN/note.txt"; printf '\n'  # XYZZ
wc -c < "$RUN/note.txt"  # 4
```

파일이 없으면 생성한다. `O_TRUNC`는 open 시점에 기존 길이를 0으로 만든다.
`O_APPEND`는 각 write를 끝에 기록한다. TEXT만 저장하므로 개행·NUL은 추가하지 않는다.
`write_all`은 부분 쓰기와 EINTR를 처리한다. 단일 write가 전부 기록한다는 가정을 피한다.

선택 수정: 소스 복사본에서 O_TRUNC를 제거하고 ABCDEFGHIJ에 XY를 쓴다.
XYCDEFGHIJ가 남는 이유를 설명한다.

이후 10분 휴식.

## 섹션 4. file_read: EOF까지 읽기 (20분)

사용법: `./file_read FILE`

```sh
printf '0123456789abcdefghijklmnopqrstuv' > "$RUN/long.txt"
./file_read "$RUN/long.txt" > "$RUN/copy.txt"
cmp "$RUN/long.txt" "$RUN/copy.txt"
./file_create "$RUN/empty.txt"
./file_read "$RUN/empty.txt"; echo $?  # 0
./file_read "$RUN/missing.txt"; echo $?  # 1
printf 'A\000B' > "$RUN/binary.dat"
./file_read "$RUN/binary.dat" > "$RUN/binary-copy.dat"
cmp "$RUN/binary.dat" "$RUN/binary-copy.dat"
```

read > 0은 실제 읽은 바이트 수, 0은 EOF, -1은 오류다(일반 파일, count > 0).
read는 NUL 종료 문자를 붙이지 않는다. 실제 읽은 n바이트만 표준 출력에 쓴다.

수정 과제: read 직후 `fprintf(stderr, "read=%zd\n", n);`를 추가한다.
버퍼 16에서는 보통 16,16,0이다. 버퍼를 7로 변경하고 다시 확인한다.
짧은 read만으로 EOF라고 판단하지 않는다. 진단 메시지는 stderr에 출력한다.

## 섹션 5. file_seek: 지정 위치 읽기와 수정 (25분)

사용법: `./file_seek FILE BYTE_OFFSET TEXT`

기존의 위치 이동 가능한 파일을 O_RDWR로 연다. OFFSET은 0 이상의 바이트 위치다.
해당 위치에서 최대 16바이트를 읽어 표준 출력에 표시한 뒤,
같은 위치로 다시 이동해서 TEXT를 덮어쓴다. 안내 메시지는 stderr에 표시한다.

```sh
./file_write truncate "$RUN/note.txt" ABCDEFGHIJ
./file_seek "$RUN/note.txt" 3 XY
printf '\n'  # 수정 전 일부 내용 DEFGHIJ 뒤 줄바꿈
./file_read "$RUN/note.txt"; printf '\n'  # ABCXYFGHIJ
wc -c < "$RUN/note.txt"  # 10
./file_seek "$RUN/note.txt" 0 ZZ
printf '\n'
./file_read "$RUN/note.txt"; printf '\n'  # ZZCXYFGHIJ
./file_seek "$RUN/note.txt" 10 '!'
printf '\n'
./file_read "$RUN/note.txt"; printf '\n'  # ZZCXYFGHIJ!
```

문자 삽입이 아니라 바이트 덮어쓰기다. read 후에는 오프셋이 이동하므로 수정 전에 다시 lseek한다.
O_APPEND를 사용하면 write는 끝으로 이동하므로 위치 수정에 쓰지 않는다.

수정 과제: 복사본에서 두 lseek 호출을 모두 `lseek(fd, -2, SEEK_END)`로 바꾼다.
새로 준비한 ABCDEFGHIJ에 XY를 기록해 ABCDEFGHXY가 되는지 확인한다.
이 변형에서는 명령줄 OFFSET을 사용하지 않으므로 명령은 `./file_seek_end FILE TEXT`로 변경하고,
OFFSET 변환 코드를 제거하며 argc 검사와 TEXT 인덱스도 함께 수정한다.

선택 관찰:

```sh
./file_write truncate "$RUN/hole.dat" ABCDEFGHIJ
./file_seek "$RUN/hole.dat" 12 Z
wc -c < "$RUN/hole.dat"  # 13
od -An -tx1 "$RUN/hole.dat"  # J 다음 00 00 5a
./file_seek "$RUN/note.txt" -1 X; echo $?  # 2
./file_seek "$RUN/missing.txt" 0 X; echo $?  # 1
```

lseek만으로 크기가 늘지는 않는다. 끝 너머에 write하면 크기가 늘며 사이 영역은 읽을 때 0이다.
한글의 문자 중간 바이트를 덮어쓰면 텍스트가 깨질 수 있으므로 기본 실습은 ASCII를 사용한다.

## 퀴즈와 제출 (10분)

C 프로그램 4개와 REPORT.md를 제출한다. 정상 실행 외에 실패·빈 파일 결과도 기록한다.
선택 과제는 시간이 남을 때 수행한다.

선택 도구: Ubuntu에서 `strace -e trace=openat,read,write,lseek,close ./file_seek "$RUN/note.txt" 3 XY`로 호출을 관찰한다.
strace가 없으면 `sudo apt install strace`로 설치한다. 시작 시 공유 라이브러리 접근은 구별한다.
