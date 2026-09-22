# Week04-class02 실습 · 파일 속성

교안: [ODP](../../Presentations/class04-02.odp) · [PDF](../../Presentations/class04-02.pdf). 저장소 루트에서 `cd Codes/class04-02`로 이동합니다.

Ubuntu 일반 사용자 환경에서 파일 속성을 조회하고, 작업 전후의 변화를 비교합니다. 준비 도구는 `cc`, `make`, `python3`입니다. 명령은 이 README가 있는 폴더에서 실행합니다.

```sh
make
make check
```

| 예제 | 학습 내용 |
|---|---|
| file_attr.c | stat/lstat, 파일 종류, rwx, 소유권, 크기, 나노초 시간 속성 |
| mode_demo.c | umask에 따른 생성 권한, fchmod 후 권한 |
| sparse_demo.c | lseek 후 쓰기, 논리적 크기와 할당 공간 |
| time_attrs.c | futimens로 atime/mtime 지정, fchmod와 ctime |
| fd_unlink_demo.c | 이름 삭제 후 열린 FD로 속성 조회와 읽기 |

Makefile은 C11과 POSIX API를 사용하며 경고를 오류로 처리합니다. `_FILE_OFFSET_BITS=64`는 32비트 Linux의 파일 크기 표현을 위한 설정입니다. `_DEFAULT_SOURCE`와 `_DARWIN_C_SOURCE`는 각각 Linux와 macOS의 구조체·상수 선언을 위한 설정입니다.

## 실습 1 · 조회 대상과 링크

다음 명령은 같은 터미널에서 실행합니다. `RUN`은 이후 실습에서도 사용합니다.

```sh
RUN=$(mktemp -d)
printf 'ABC\n' > "$RUN/note.txt"
ln "$RUN/note.txt" "$RUN/hard.txt"
ln -s note.txt "$RUN/soft.txt"
./file_attr "$RUN/note.txt"
./file_attr "$RUN/hard.txt"
./file_attr "$RUN/soft.txt"
./file_attr --follow "$RUN/soft.txt"
ln -s missing "$RUN/broken.txt"
./file_attr "$RUN/broken.txt"
./file_attr --follow "$RUN/broken.txt"
echo $?
./file_attr "$RUN/missing"
echo $?
./file_attr "$RUN/note.txt/child"
echo $?
```

관찰할 점:

- 원본과 하드 링크의 `device`, `inode`는 같고 `links=2`입니다.
- 기본 조회는 `lstat`입니다. `soft.txt`의 종류는 symlink, 크기는 저장한 문자열 `note.txt`의 길이인 8입니다.
- `--follow`는 `stat`을 사용합니다. 대상의 종류 regular와 크기 4가 나옵니다.
- 깨진 링크도 링크 자체의 속성은 읽을 수 있습니다. 대상을 따라가는 조회는 실패합니다.
- inode 값, UID/GID, 시각은 환경마다 다릅니다. 관계와 의미를 기록합니다.

## 실습 2 · 생성 권한과 변경 권한

기본 ACL이 없는 디렉토리를 기준으로 비교합니다.

```sh
mkdir "$RUN/modes"
./mode_demo "$RUN/modes"
./file_attr "$RUN/modes/mask022.txt"
./file_attr "$RUN/modes/mask077.txt"
./file_attr "$RUN/modes/groupdir"
chmod 600 "$RUN/note.txt"
./file_attr "$RUN/note.txt"
./file_attr "$RUN/hard.txt"
```

mode_demo의 예상 출력:

```text
mask022.txt mode=0644
after_fchmod mode=0640
mask077.txt mode=0600
groupdir mode=0750
```

첫 파일은 `0666 & ~0022 = 0644`로 생성한 **직후에** fchmod로 0640으로 바꿉니다. 따라서 나중에 file_attr로 조회하면 0640입니다. 디렉토리는 0777과 mask 0027로 0750이 됩니다. 기본 ACL이 있으면 생성 권한의 결정 과정이 달라질 수 있습니다.

기존 파일을 덮어쓰지 않도록 O_EXCL을 사용합니다. 같은 폴더에 mode_demo를 두 번째 실행하면 기존 이름 때문에 종료 상태 1로 실패합니다. 이미 만든 파일은 남으며 자동으로 되돌리지 않습니다. 다시 실험할 때는 새 임시 폴더를 준비합니다.

내용 쓰기 권한 없이도 속성 조회가 가능한지 확인합니다.

```sh
chmod 000 "$RUN/note.txt"
./file_attr "$RUN/note.txt"
chmod 600 "$RUN/note.txt"
```

속성 조회에 필요한 경로 검색 권한과 파일 내용 읽기 권한을 구별합니다. 소유권 변경은 권한 조건을 설명하는 주제이며 root 실행이나 임의 계정으로의 chown은 실습에 필요하지 않습니다.

## 실습 3 · 크기와 할당 공간

```sh
./sparse_demo "$RUN/sparse.bin"
./file_attr "$RUN/sparse.bin"
stat -c 'size=%s blocks=%b unit=%B' "$RUN/sparse.bin"
du -B1 "$RUN/sparse.bin"
```

위 stat·du 옵션은 Ubuntu GNU coreutils용입니다. C 예제는 1 MiB 위치로 이동해서 `X` 한 바이트를 씁니다. 논리적 크기는 **1,048,577바이트**입니다. lseek만으로는 크기가 늘지 않습니다.

Linux에서 `st_blocks`의 단위는 512바이트이며 `st_blksize`는 권장 입출력 블록 크기입니다. 실제 할당량은 파일시스템에 따라 달라지므로 고정된 숫자로 채점하지 않습니다.

앞부분과 끝 바이트를 확인하는 선택 명령:

```sh
od -An -tx1 -N16 "$RUN/sparse.bin"
tail -c 1 "$RUN/sparse.bin" | od -An -tx1
```

앞 16바이트는 00, 마지막 바이트는 ASCII X의 58이 예상됩니다.

## 실습 4 · 시간 속성

```sh
./time_attrs "$RUN/times.txt"
./file_attr "$RUN/times.txt"
stat -c 'atime=%X mtime=%Y ctime=%Z' "$RUN/times.txt"
```

새 파일을 만들고 atime=1704067200(UTC 2024-01-01), mtime=1704153600(UTC 2024-01-02)으로 지정합니다. 이후 권한을 0640으로 변경합니다.

- after_futimens와 after_fchmod의 atime·mtime은 유지됩니다.
- ctime은 inode 상태 변경 시각이며 생성 시각이 아닙니다.
- 연속된 작업의 ctime이 같은 숫자여도 실패라고 단정하지 않습니다. 출력 및 저장 시각 해상도의 영향을 받습니다.
- 읽기 직후 atime 갱신을 항상 기대할 수는 없습니다. noatime, relatime 등 정책의 영향을 받습니다.

## 실습 5 · 이름 삭제 이후

```sh
./fd_unlink_demo
```

예제 자체가 만든 전용 임시 파일만 생성·삭제합니다. 명령 인자는 없습니다.

```text
path=/tmp/lsp-file-attrs-XXXXXX
before device=... inode=... links=1 size=5
after device=... inode=... links=0 size=5
path_lookup=ENOENT
read=hello
```

임시 이름과 device·inode는 실제 환경의 값입니다. 같은 inode와 크기를 유지하면서 링크 수가 0이 되는지 확인합니다. 열린 FD가 참조를 유지하므로 이름으로는 찾을 수 없어도 fstat과 read가 가능합니다. 정상 종료 후 예제가 만든 임시 파일은 남지 않습니다.

## 종료 상태와 제출

- 0: 정상 완료
- 1: 조회·생성·변경·입출력 등의 실패
- 2: 잘못된 실행 인자

실패 직후 `echo $?`를 실행합니다. 의도적인 실패가 있으므로 모든 명령을 `set -e`로 일괄 실행하지 않습니다.

제출물은 C 소스 5개, Makefile, 작성한 REPORT.md, 정상·실패 실행 로그입니다. 선택 과제는 이전 mini_ls에 rwx 문자열, UID/GID, 크기, mtime 열을 추가하는 것입니다.

## 실행 확인 범위

2026-09-22 macOS arm64에서 다섯 예제를 경고 없이 컴파일하고 자동 검사 29회 실행을 통과했습니다. 링크, 깨진 링크, 공백 경로, FIFO, 권한 000의 속성 조회, 디렉토리 검색 권한 오류, 생성 권한, 희소 데이터, 시간 속성, unlink 이후 FD를 검사했습니다. Linux 실행은 이번 제작 환경에서 확인하지 않았으므로 수업용 Ubuntu에서 `make check`로 재확인할 수 있습니다. root에서는 검색 권한 실패 검사를 건너뜁니다.

## 참고

[stat(2)](https://man7.org/linux/man-pages/man2/stat.2.html), [inode(7)](https://man7.org/linux/man-pages/man7/inode.7.html), [chmod(2)](https://man7.org/linux/man-pages/man2/chmod.2.html), [umask(2)](https://man7.org/linux/man-pages/man2/umask.2.html), [utimensat(2)](https://man7.org/linux/man-pages/man2/utimensat.2.html), [unlink(2)](https://man7.org/linux/man-pages/man2/unlink.2.html).
