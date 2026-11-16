# Week10-class01 실습 · 프로세스 생성과 실행: fork · exec · wait

예제 7개는 [bluekds/SystemProgramming의 Lecture7](https://github.com/bluekds/SystemProgramming/tree/main/Lecture7)(SPIN Lab 김덕수 교수, 시스템 프로그래밍 7강)을 바꾸지 않고 그대로 가져왔고, Makefile과 test_examples.py, 이 안내만 추가했습니다. 먼저 영상 세 편을 보고 오면 예제의 의도가 보입니다: [7강-1 프로세스 생성/실행](https://youtu.be/Q8LltC5L4EA)(25분), [7강-2 프로세스 종료](https://youtu.be/ypSjwkG5SKQ)(10분), [7강-3 자식 프로세스 동기화](https://youtu.be/hbeROUdkxsc)(16분).

준비 도구는 `cc`, `make`, `python3`이고, 명령은 이 README가 있는 폴더에서 실행합니다.

```sh
make
make check        # python3 test_examples.py
```

| 예제 | 영상 | 학습 내용 |
|---|---|---|
| fork.c | 7강-1 | fork 한 번에 두 번 반환. 부모·자식의 PID·PPID와 "End of fork"가 두 번 찍히는 이유 |
| execlp.c | 7강-1 | PATH에서 ls를 찾아 프로그램을 교체. "After" 줄이 찍히지 않는 이유 |
| execv.c | 7강-1 | 경로(/bin/ls)와 인자 배열로 같은 일을 하는 변형 |
| forkNexec.c | 7강-1 | 자식만 ls로 바꾸고 부모는 계속 — 셸의 명령 실행 구조(wait만 빠짐) |
| atexit.c | 7강-2 | 종료 처리기 등록과 역순 호출 |
| wait.c | 7강-3 | 자식 exit(2) → 부모 wait → status 512(0x0200)의 해석 |
| waitpid.c | 7강-3 | WNOHANG으로 1초마다 "끝났니?" 찔러 보기 |

Makefile은 C11과 POSIX API를 쓰고 경고를 오류로 처리합니다. `_DARWIN_C_SOURCE`는 macOS, `_DEFAULT_SOURCE`는 Linux에서 추가 선언을 켭니다. 일곱 예제는 Ubuntu 24.04(gcc 13)와 macOS(clang) 양쪽에서 경고 없이 빌드됩니다.

`test_examples.py`는 예제를 **파이프에 연결해** 실행합니다. 그래서 exec 직전의 `printf` 출력("Before exec function", "--> Child Process")이 **없는 것**을 정상으로 검사합니다 — stdout이 파이프면 완전 버퍼링이라 그 줄이 버퍼에 남아 있다가 exec와 함께 사라지기 때문입니다. 터미널에서 직접 실행하면(줄 버퍼링) 그 줄이 보입니다. 이 차이가 실습 2의 관찰 대상입니다.

`strace`와 `ps -o cmd`는 Linux에서만 됩니다. macOS에서는 `ps -o pid,ppid,stat,command`를 쓰고, strace 관찰(실습 3)은 실습 서버나 Ubuntu 컨테이너에서 합니다. 출력 값(PID, 폴더의 파일 목록, "still wait" 횟수)은 실행마다 다릅니다. 숫자를 맞히는 것이 아니라 프로그램의 자기 보고와 밖의 관찰(`$$`, `ps`, `strace`, `$?`)이 같은 사실을 가리키는지, 그 관계를 설명할 수 있는지가 기록 대상입니다.

## 실습 1 · fork: 한 번 호출, 두 번 반환 (교안 1부, 영상 7강-1)

예측: `./fork`를 실행하면 "End of fork"는 몇 번 찍힐까요? 자식이 출력하는 "My Parent's PID"는 항상 부모의 PID일까요?

```sh
echo $$                       # 셸의 PID
./fork
./fork                        # 여러 번 실행해 PPID 값을 비교한다
./fork | cat                  # 파이프에서도 같은 줄 수인지
```

관찰할 점:

- 부모가 찍은 "My Parent's PID"가 `$$`와 같은지, 부모가 찍은 "My Child's PID"가 자식이 찍은 "My PID"와 같은지 확인하고 사슬(셸 → 부모 → 자식)을 적습니다.
- 자식의 "My Parent's PID"가 1로 나오는 실행이 있으면 기록합니다. 부모가 먼저 끝나 자식이 PID 1(init)에게 입양된 뒤 `getppid()`가 불린 것입니다(교안 16·17쪽). 나오지 않아도 "나올 수 있는 이유"를 설명합니다.
- "End of fork"가 두 번인 이유를 fork 다음 줄부터 두 프로세스가 각자 실행한다는 말로 설명합니다.
- 부모 줄과 자식 줄의 순서가 실행마다 바뀌는지 봅니다. 바뀌지 않더라도 순서가 보장되지 않는 이유를 적습니다.

## 실습 2 · exec: 교체, 버퍼, 실패 (교안 1부, 영상 7강-1)

예측: `./execlp`의 출력에 "After exec function"이 나올까요? `./execlp | head -3`에는 "Before"가 나올까요?

```sh
./execlp
./execv
./execlp | head -3            # "Before" 줄이 있는가?
./execlp > out.txt; cat out.txt
PATH=/nonexistent ./execlp; echo "exit=$?"
```

관찰할 점:

- "After" 줄이 없는 이유를 "프로세스의 메모리를 ls의 바이너리로 덮어썼다"는 말로 설명합니다. `echo $?`가 누구의 종료 상태인지 적습니다(ls의 것).
- 파이프·파일로 보냈을 때 "Before" 줄이 사라진 이유를 stdout의 버퍼링 방식(터미널 줄 버퍼 / 파이프 완전 버퍼)과 exec가 사용자 공간을 통째로 교체한다는 사실로 설명합니다.
- PATH를 비웠을 때 `execlp: No such file or directory`와 exit=1이 나오고 이번에는 "Before"가 보이는 이유(exec 실패 → 프로세스 생존 → exit(1)의 flush)를 적습니다. stderr(perror)가 "Before"보다 먼저 찍혔다면 그 이유(stderr는 버퍼 없음)도 적습니다.
- execlp와 execv의 호출 형태 차이(인자 나열 vs 배열, PATH 검색 vs 경로)를 한 줄로 정리합니다.

## 실습 3 · fork + exec: 셸의 방식과 strace (교안 1부, 영상 7강-1)

예측: `./forkNexec`를 실행하면 프롬프트와 ls 출력 중 무엇이 먼저 나올까요?

```sh
./forkNexec                   # 프롬프트가 먼저 돌아오면 Enter를 한 번 더
./forkNexec | head -4         # "--> Child Process" 줄이 있는가?

# Linux(실습 서버·컨테이너)에서
strace -f -e trace=%process -o st.txt ./forkNexec > /dev/null; cat st.txt
strace -f -e trace=%process -o st2.txt ./wait > /dev/null; cat st2.txt
```

관찰할 점:

- 프롬프트가 ls 출력보다 먼저 돌아왔다면 그 이유(부모가 wait하지 않고 끝나 셸이 프롬프트를 찍고, 고아가 된 ls가 그 뒤에 출력)를 적습니다.
- strace 기록에서 `clone(... SIGCHLD) = 자식PID`, `execve("/usr/local/sbin/ls") = -1 ENOENT` … `execve("/usr/bin/ls") = 0`처럼 **execve가 몇 번 시도되었는지** 세고, 그것이 execlp의 "p"(PATH 검색)와 어떻게 연결되는지 설명합니다. `echo $PATH`와 대조합니다.
- 부모(첫 PID)와 자식의 `exit_group` 순서를 적고, 실습 3의 첫 관찰과 연결합니다.
- `./wait`의 strace에서 `wait4(-1, ...)`, `exit_group(2)`, `[{WIFEXITED(s) && WEXITSTATUS(s) == 2}]`, `--- SIGCHLD {... si_status=2}` 네 줄을 찾아 순서대로 적습니다.

## 실습 4 · wait · waitpid: 회수와 좀비 (교안 3부, 영상 7강-3)

예측: `./wait`의 "Status:" 줄은 얼마일까요? `./waitpid`의 "Parent still wait..."는 몇 번 찍힐까요?

```sh
./wait; echo "exit=$?"
./waitpid & sleep 1.2; ps -o pid,ppid,stat,cmd; wait      # macOS: ...,command
(sleep 1 & exec sleep 4) & sleep 2; ps -o pid,ppid,stat,cmd; wait
```

관찰할 점:

- `Status: 512, 200`에서 512 = 2 << 8임을 확인하고, `WEXITSTATUS`가 어느 바이트를 꺼내는지 적습니다. `exit=$?`가 0인 이유(셸이 받은 것은 wait.c 부모의 return 0)를 설명합니다.
- waitpid의 ps 출력에서 셸 → 부모 → 자식의 PID·PPID 사슬과 STAT(둘 다 S)를 적고, 부모와 자식이 각각 무엇 때문에 잠들어 있는지(sleep(1), sleep(3)) 설명합니다. "still wait" 횟수를 영상(4회)과 비교하고 왜 다를 수 있는지 적습니다.
- 세 번째 명령의 ps에서 `Z`·`<defunct>` 줄을 찾아 PID·PPID를 적고, 왜 좀비가 생겼는지(서브셸이 `exec sleep 4`로 교체돼 wait할 코드가 없음), 4초 뒤 누가 치우는지(PID 1) 설명합니다.
- wait와 waitpid의 차이(누구를, WNOHANG)를 한 줄로 정리합니다.

## 실습 5 · atexit와 exit의 단계 (교안 2부, 영상 7강-2)

예측: `./atexit`는 "Cleanup 1"과 "Cleanup 2" 중 무엇을 먼저 찍을까요?

```sh
./atexit; echo "exit=$?"
# 변형: atexit.c의 exit(0)을 차례로 바꿔 보고 되돌린다
#   return 0;   /  _exit(0);  (unistd.h 추가)  /  abort();
```

관찰할 점:

- 호출 순서(2 → 1)를 등록 순서와 비교해 "역순"임을 적습니다.
- 변형 세 가지의 출력과 `$?`를 표로 적고, exit의 네 단계(처리기 → flush → 임시 파일 → _exit) 중 어디를 건너뛰었는지 설명합니다.
- 선택 과제: forkNexec.c의 자식 가지에 있는 `exit(1)`·`exit(0)`을 `_exit`로 바꾸면 `./forkNexec | head -4`의 출력이 달라지는지 확인하고, 교안 28쪽(버퍼 사본)의 규칙으로 설명합니다. 바꾼 파일은 되돌립니다.

## 종료 상태와 제출

- 0: 정상 완료 (execlp·execv·forkNexec는 ls의 종료 상태)
- 1: fork·exec 실패 (`perror` 메시지와 함께)
- 2·3: wait.c·waitpid.c의 **자식**이 부모에게 전달하는 값 — 셸의 `$?`로는 보이지 않는다(부모가 받는다)

제출물은 C 소스 7개(원본 그대로), Makefile, 작성한 REPORT.md, 실행 로그(터미널 복사)입니다.

## 실행 확인 범위

2026-11-02 macOS arm64(clang)와 Ubuntu 24.04 컨테이너(Docker, aarch64, gcc 13.3)에서 일곱 예제를 `-Werror`로 빌드하고 `python3 test_examples.py`(8회 실행)를 통과했습니다. 교안의 실행 화면은 같은 날 Ubuntu 24.04 컨테이너에서 iTerm으로 캡처한 실제 화면입니다(실습 서버의 ssh 포트가 이날 닫혀 있어 컨테이너로 대체). 서버에서는 PID 값과 PATH의 디렉터리 순서(execve 시도 횟수)가 다를 수 있습니다.

## 참고

[fork(2)](https://man7.org/linux/man-pages/man2/fork.2.html), [execve(2)](https://man7.org/linux/man-pages/man2/execve.2.html), [exec(3)](https://man7.org/linux/man-pages/man3/exec.3.html), [wait(2)](https://man7.org/linux/man-pages/man2/wait.2.html), [exit(3)](https://man7.org/linux/man-pages/man3/exit.3.html), [_exit(2)](https://man7.org/linux/man-pages/man2/_exit.2.html), [atexit(3)](https://man7.org/linux/man-pages/man3/atexit.3.html), [strace(1)](https://man7.org/linux/man-pages/man1/strace.1.html), [ps(1)](https://man7.org/linux/man-pages/man1/ps.1.html), [stdio(3)](https://man7.org/linux/man-pages/man3/stdio.3.html). 예제 출처: [bluekds/SystemProgramming](https://github.com/bluekds/SystemProgramming) · 강의 홈페이지 [sites.google.com/view/hpclab/courses/system-programming](https://sites.google.com/view/hpclab/courses/system-programming).
