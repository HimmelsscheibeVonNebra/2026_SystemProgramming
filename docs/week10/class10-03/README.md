# 10주차 3차시 실습 — 부모가 자식을 제어하기 (labs/class10-03)

교안 `output/week10/week10-class03-v01.pptx`의 예제 2개입니다. 1부와 2부에 하나씩 대응합니다.

| 파일 | 부 | 하는 일 |
|---|---|---|
| `forkpool.c` | 1부 | 시그널 없이 fork와 waitpid만 씁니다. 자식 여러 개에게 일을 나눠 주고(fork 전 변수), 동시에 도는 수를 MAXPAR로 제한합니다. `waitpid(-1)`로 먼저 끝난 자식부터 거두고, 종료 코드로 판정해 실패한 작업은 한 번 더 시킵니다. 작업 3은 첫 시도에서 `exit 1`로 실패하도록 만들어 두었습니다. |
| `sigctl.c` | 2부 | 부모가 `kill`로 SIGUSR1(보고 요청), SIGSTOP(멈춤), SIGCONT(재개), SIGTERM(정리 종료)이나 SIGKILL(강제 종료)을 보냅니다. 자식의 변화는 `waitpid(WUNTRACED \| WCONTINUED)`로 확인합니다. 자식의 핸들러는 깃발만 세웁니다. |

## 빌드와 검사

```bash
make            # gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -g
make test       # python3 test_examples.py — Linux에서 16개 검사
make clean
```

macOS에서도 경고 없이 빌드되지만, 자동 검사와 교안의 시그널 번호(SIGSTOP 19, SIGUSR1 10)는 Linux 기준입니다.

## 실행 예

```bash
./forkpool 6 2                                   # 두 자리로 여섯 작업 + 재시도 1
for p in 1 2 3 6; do printf 'MAXPAR=%d ' $p; ./forkpool 6 $p | tail -1; done
./sigctl                                         # 보고 → 멈춤 → 재개 → 보고 → SIGTERM(정리 후 exit 0)
./sigctl kill                                    # 마지막에 SIGKILL — 정리 없이 signal 9
./sigctl late                                    # 핸들러 설치 전 SIGUSR1 → 자식이 signal 10으로 죽음
./sigctl > log.txt & sleep 0.65; ps -o pid,ppid,stat,cmd -C sigctl   # 멈춘 자식은 STAT T
```

## 실습 과제

1. **forkpool 시간 제한**: 작업이 1초를 넘으면 부모가 `kill(pid, SIGKILL)`로 끝내고 실패로 처리합니다. 방법은 둘 중 하나를 고릅니다.
   - `waitpid(-1, &st, WNOHANG)` 폴링과 작업별 시작 시각 기록
   - `alarm`과 SIGALRM
2. **재시도 정책**: 시그널로 죽은 작업(`WIFSIGNALED`)은 재시도하지 않게 바꿉니다. 확인하려면 `./forkpool 6 1`을 실행하는 동안 다른 창에서 `kill -9 <작업 PID>`를 보냅니다.
3. **sigctl 속도 전환**: SIGUSR2를 "속도 바꾸기"로 약속하고 틱 간격을 100 ms와 50 ms 사이에서 바꿉니다. 필요한 것은 깃발 `want_speed`, 핸들러 `on_usr2`이고, 핸들러는 fork 전에 설치합니다. 부모가 실행 도중에 SIGUSR2를 보내게 합니다.

제출: 소스, Makefile, 실행 기록 `lab.txt`, 과제마다 "관찰한 것 한 줄".
