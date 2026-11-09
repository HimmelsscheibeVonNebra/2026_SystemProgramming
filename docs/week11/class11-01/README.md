# 11주차 1차시 실습 — 메모리 매핑 (labs/class11-01)

교안 `output/week11/week11-class01-v02.pptx`의 예제 3개입니다. 각 부(1부·2부·3부)에 하나씩 대응합니다.

| 파일 | 부 | 하는 일 |
|---|---|---|
| `mapcat.c` | 1부 | 파일을 `mmap(PROT_READ, MAP_PRIVATE)`으로 매핑해 표준 출력으로 내보낸다. `-p`는 Enter를 기다려 `/proc/PID/maps`를 관찰하게 한다. |
| `mapmode.c` | 2부 | `private` / `shared`: 첫 줄을 대문자로 바꾸고 `pread`로 파일을 다시 읽는다. `anon`: fork 뒤 전역 변수와 익명 공유 매핑을 비교한다. `race`: 여러 자식이 공유 카운터를 동시에 증가시킨다. |
| `countnl.c` | 3부 | 같은 파일의 개행을 `read`(64 KiB 버퍼)와 `mmap`으로 세고 경과 시간과 minor/major fault를 출력한다. `drop`은 `posix_fadvise(DONTNEED)`로 그 파일의 페이지 캐시를 비운다. |

## 빌드와 검사

```bash
make            # gcc -Wall -Wextra -O2 -g
make test       # python3 test_examples.py — Linux에서 21개 검사
make clean
```

macOS에서도 빌드됩니다. 다만 `countnl drop`은 `posix_fadvise`가 없어 안내만 하고 끝나고, 자동 검사는 Linux 기준입니다.

## 실행 예

```bash
printf 'hello mmap\nsecond line\n' > hello.txt
./mapcat hello.txt
sleep 60 | ./mapcat -p hello.txt > /dev/null &      # 다른 명령으로 /proc/$!/maps 확인
cp hello.txt p.txt; ./mapmode private p.txt; cat p.txt
cp hello.txt s.txt; ./mapmode shared s.txt;  cat s.txt
./mapmode anon
./mapmode race 4 1000000
seq 1 30000000 > big.txt                             # 약 247 MB
./countnl drop big.txt; ./countnl read big.txt; ./countnl read big.txt
./countnl drop big.txt; ./countnl mmap big.txt; ./countnl mmap big.txt seq
```

## 실습 과제

1. **mapcat 범위 출력** — `./mapcat FILE OFFSET LEN`. offset을 페이지 경계로 내려(`off & ~(pagesz - 1)`) 매핑하고 차이만큼 건너뛰어 출력한다. 범위가 파일 끝을 넘으면 거절한다. 페이지 크기는 `sysconf(_SC_PAGESIZE)`로 얻는다.
2. **mapmode struct 모드** — `./mapmode struct`. `struct msg { int count; char text[32]; }`를 익명 공유 매핑에 두고 자식이 채운 뒤 `_exit`, 부모가 `waitpid` 뒤 출력한다.
3. **countnl 크기별 비교표** — 1 MB · 50 MB · 200 MB 파일 각각에서 drop → read → read, drop → mmap → mmap을 실행해 시간 · minflt · majflt 표를 만들고, 어느 크기와 상태에서 차이가 나는지 한 줄로 결론을 쓴다.

제출: 소스, Makefile, 실행 기록 `lab.txt`, 과제마다 "관찰한 것 한 줄".

## 측정 환경 메모

교안의 화면은 Ubuntu 24.04 컨테이너(Docker, aarch64, glibc 2.39)에서 2026-11-09에 캡처했습니다. Docker의 overlayfs 위 파일에서는 `posix_fadvise(DONTNEED)`가 캐시를 비우지 못했으므로, 실습 폴더는 ext4 볼륨에 두었습니다. 또 Docker Desktop은 디스크가 Mac 호스트 캐시 위에 있어 cold와 warm의 시간 차이가 실제 디스크보다 작게 나옵니다. 실습 서버에서 다시 재 보기를 권합니다.
