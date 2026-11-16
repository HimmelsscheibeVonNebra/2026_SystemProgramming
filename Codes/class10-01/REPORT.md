# Week10-class01 · 프로세스 생성과 실행 실습 기록지

- 이름 / 학번:
- 운영체제·아키텍처 (Linux 관찰은 어디서? 실습 서버 / 컨테이너):
- 영상 시청: 7강-1 ☐ · 7강-2 ☐ · 7강-3 ☐
- `make check` 결과:

## 1. fork (fork.c)

| 실행 | $$ | 부모 PID | 부모가 찍은 PPID | 부모가 찍은 자식 PID | 자식 PID | 자식이 찍은 PPID |
|---|---|---|---|---|---|---|
| 1회 | | | | | | |
| 2회 | | | | | | |
| 3회 | | | | | | |

- "End of fork"가 찍힌 횟수와 그 이유:
- 자식이 찍은 PPID가 1인 실행이 있었나? 있었다면(없었다면 "있을 수 있는") 이유:
- 부모 줄과 자식 줄의 순서가 바뀐 적이 있나? 순서가 보장되지 않는 이유:

## 2. exec (execlp.c · execv.c)

| 명령 | "Before" 보임? | "After" 보임? | 그 뒤의 출력 | $? |
|---|---|---|---|---|
| ./execlp | | | | |
| ./execlp \| head -3 | | | | |
| ./execlp > out.txt | | | | |
| PATH=/nonexistent ./execlp | | | | |

- "After"가 찍히지 않는 이유(exec가 하는 일):
- 파이프·파일에서 "Before"가 사라진 이유(버퍼링 방식 + exec):
- 실패했을 때 "Before"가 다시 보이는 이유:
- execlp와 execv의 호출 형태 차이 한 줄:

## 3. fork + exec (forkNexec.c) 와 strace

- `./forkNexec`에서 프롬프트와 ls 출력 중 먼저 나온 것과 그 이유:
- `./forkNexec | head -4`에서 "--> Child Process" 줄이 있었나? 이유:

| strace 항목 (forkNexec) | 값 |
|---|---|
| clone(...)의 반환값(자식 PID) | |
| execve 시도 횟수와 시도한 경로들 | |
| 성공한 execve의 경로 | |
| exit_group 순서 (부모 먼저 / 자식 먼저) | |

| strace 항목 (wait) | 값 |
|---|---|
| wait4의 첫 인자 | |
| 자식의 exit_group 인자 | |
| wait4가 돌아올 때의 status 식 | |
| SIGCHLD의 si_pid · si_status | |

- execve 시도 횟수와 `echo $PATH`의 관계:

## 4. wait · waitpid · 좀비

| 항목 | 값 |
|---|---|
| ./wait의 Status (10진 / 16진) | |
| WEXITSTATUS | |
| ./wait 뒤 $? 와 그 이유 | |
| ./waitpid의 "still wait" 횟수 | |
| ps 사슬: 셸 PID → 부모 PID → 자식 PID (STAT) | |
| 좀비의 PID · PPID · STAT · CMD | |

- 512가 2 << 8인 것을 비트로 설명:
- "still wait" 횟수가 영상(4회)과 다르다면 그 이유:
- 좀비가 생긴 이유와 4초 뒤 누가 치우는지:
- wait와 waitpid의 차이 한 줄:

## 5. atexit와 exit의 단계

| atexit.c의 마지막 줄 | 출력 | $? | 건너뛴 단계 |
|---|---|---|---|
| exit(0) | | | — |
| return 0 | | | |
| _exit(0) | | | |
| abort() | | | |

- 호출 순서가 2 → 1인 이유:
- (선택) forkNexec.c의 exit를 _exit로 바꿨을 때 `| head -4` 출력의 변화와 이유:

## 6. 오늘의 계약 한 줄씩

- fork:
- exec:
- wait / waitpid:
- exit / _exit:
- 좀비와 고아:
