# 교수자 안내 · 프로세스 생성과 실행: fork · exec · wait

예제 7개는 bluekds/SystemProgramming Lecture7(SPIN Lab 김덕수 교수)의 원본 그대로이고, 영상 7강-1·2·3의 순서(생성/실행 → 종료 → 동기화)가 교안의 1·2·3부입니다. 실습은 프로그램의 자기 보고(printf)와 밖의 창(`$$`, `ps`, `strace`, `$?`)을 맞춰 보는 방식입니다. 값은 환경마다 다르므로 숫자 일치가 아니라 관계 설명을 봅니다. 교안의 실행 화면은 2026-11-02에 Ubuntu 24.04 컨테이너(Docker, aarch64, gcc 13.3, 커널 7.0 linuxkit)에서 캡처한 실제 화면입니다(shots/ 폴더의 k10_*.txt·png). 이날 실습 서버(203.252.136.17:60022)의 ssh 포트가 닫혀 있어 컨테이너로 대체했습니다.

## 핵심 확인 사항 (컨테이너 실측)

- fork(k10_fork): 셸 `$$`=206, 첫 실행 부모 220(PPID 206, 자식 221), **자식의 PPID가 1** — 부모가 printf·return을 자식의 getppid()보다 먼저 끝내 자식이 PID 1(docker-init)에게 입양된 뒤였습니다. 둘째 실행은 부모 222, 자식 223, 자식의 PPID 222. 같은 프로그램에서 두 결과를 얻은 것이 "순서는 스케줄러가 정한다"의 실증입니다. 실습 서버(ssh 세션)에서도 고아는 PID 1에게 갑니다(7주차 확인 사실). "End of fork"는 매번 두 번.
- exec(k10_exec): `./execlp`와 `./execv` 모두 "Before" 한 줄 뒤 ls -a 목록, "After" 없음. `$?`는 ls의 0.
- exec의 경계(k10_execpipe): `./execlp | head -3`은 `. .. Makefile`만 — "Before"가 파이프의 완전 버퍼에 남은 채 exec에 버려졌습니다. `PATH=/nonexistent ./execlp`는 `--> Before exec function` / `execlp: No such file or directory` / `exit=1`. 이번에는 Before가 보이는 이유(exec 실패 → exit(1)의 flush)와, 터미널이라 줄 버퍼여서 Before가 perror보다 먼저 보인 점을 짚습니다. 파이프로 실행하면 perror(stderr, 버퍼 없음)가 먼저 나옵니다 — test_examples.py의 실패 케이스가 그 순서입니다.
- forkNexec(k10_forkexec): 부모 232가 PID를 찍고 끝나 **프롬프트가 ls 출력보다 먼저** 돌아왔고, 고아가 된 ls의 목록이 프롬프트 뒤에 찍혔습니다. `| head -4`에서는 "--> Child Process"가 사라집니다(파이프 버퍼). 영상의 깔끔한 출력과 다른 점이 wait의 동기입니다.
- strace(k10_strace_exec): `270 execve("./forkNexec")`, `270 clone(... SIGCHLD) = 271`, `270 exit_group(0)`, 자식 271의 `execve`가 `/usr/local/sbin/ls`, `/usr/local/bin/ls`, `/usr/sbin/ls`에서 ENOENT 뒤 `/usr/bin/ls`에서 0 — execlp의 PATH 검색이 그대로 보입니다(컨테이너 lab 사용자의 PATH 순서). 부모가 먼저 exit_group. 서버에서는 PATH가 달라 시도 횟수가 다를 수 있습니다.
- strace wait(k10_strace_wait): `248 clone = 249`, `248 wait4(-1, <unfinished>)`, `249 exit_group(2)`, `wait4 resumed [{WIFEXITED(s) && WEXITSTATUS(s) == 2}] = 249`, `--- SIGCHLD {si_pid=249, si_status=2} ---`. wait의 정체가 wait4이고 SIGCHLD가 회수와 별개로 배달된다는 두 가지를 읽게 합니다.
- wait(k10_wait): `Status: 512, 200`, `Child process Exit Status:2`, `$?`=0. 512 = 2 << 8. `$?`가 0인 이유(셸이 받은 것은 부모의 return 0)를 꼭 묻습니다.
- waitpid(k10_waitpid): "Parent still wait..." **3회**(영상은 4회), ps 사슬 bash 206 → 255(S) → 257(S), 자식 종료 뒤 `Child Exit Status : 3`, 셸의 `[1]+ Done`. 횟수 차이는 네 번째 질문(3.0초)과 자식 종료(3.0초)의 경쟁 때문입니다.
- 좀비(k10_zombie): `(sleep 1 & exec sleep 4) &` 2초 뒤 ps에 `262  260  Z  [sleep] <defunct>` — PPID 260은 exec로 sleep 4가 된 서브셸. 4초 뒤 sleep 4가 끝나면 PID 1이 회수합니다. C 코드 없이 오늘의 exec 지식으로 좀비를 만든 것이 포인트입니다.
- atexit(k10_atexit): `Cleanup 2 is called.` → `Cleanup 1 is called.`, `$?`=0.

## 로컬(macOS) 실측과 Linux의 차이

2026-11-02 macOS arm64(clang)에서 일곱 예제를 -Werror로 빌드했고 test_examples.py(8회)를 통과했습니다.

- `strace`가 없습니다. 실습 3의 strace는 Linux에서만 합니다. `ps -o cmd`는 macOS에 없으므로 `command`를 씁니다.
- fork·exec·wait·waitpid·atexit의 출력은 macOS에서도 같습니다(PID 값만 다름). 파이프에서 "Before"·"--> Child Process"가 사라지는 동작, PATH를 비웠을 때의 실패(exit=1)도 같습니다. 고아는 launchd(PID 1)에게 갑니다.
- 좀비 관찰 명령은 macOS에서도 됩니다(`ps -o pid,ppid,stat,command`에 `Z`와 `<defunct>`).

## 영상과 다르게 가르치는 점

- 영상 7강-2는 _exit 뒤 커널이 "자식 프로세스에게 SIGHUP을 보낸다"고 설명하지만, 일반 프로세스의 자식은 PID 1에 입양될 뿐입니다. SIGHUP은 세션 리더(제어 터미널의 셸)가 끝날 때 전경 프로세스 그룹에 가는 것입니다(6주차 세션 규칙). 교안 25쪽은 "부모에게 SIGCHLD, 자식들은 입양"으로 적었습니다.
- 영상 7강-3의 WNOWAIT는 Linux에서 waitid(2)의 옵션이라 waitpid 표(37쪽)에서는 뺐습니다.
- 영상은 wait.c·waitpid.c의 status를 "앞 8비트 exit 값, 뒤 시그널"로 설명합니다. 교안 33쪽은 같은 내용에 9주차 exit_how의 실측(0x0086·0x137f·0xffff)을 더해 "매크로로만 읽는다"를 강조합니다.

## 평가 기준

숫자의 일치보다 다음을 설명했는지 봅니다: fork의 두 반환값과 PID 사슬, PPID가 1이 될 수 있는 이유, exec가 교체이며 성공 시 돌아오지 않는다는 것, 파이프에서 Before가 사라지는 이유(버퍼 + exec), execlp의 PATH 검색(execve 횟수), 프롬프트가 먼저 돌아오는 이유(wait 없음), 512의 비트 배치와 `$?`가 0인 이유, 좀비의 생성과 회수 주체, atexit의 역순과 exit의 단계. 관찰 대상을 `ps`·`strace`로 확인했는지도 습관 지표로 봅니다.

## 진행 문제 해결

- `./forkNexec` 뒤 프롬프트가 깨진 것처럼 보임: 정상입니다(고아 ls의 출력). Enter를 치면 프롬프트가 다시 나옵니다.
- `./execlp | head -3`에 Before가 없다고 버그로 보고함: 정상 동작입니다(README의 버퍼 설명). 터미널에서 실행하면 보입니다.
- strace에서 `clone3`가 보임: glibc 버전에 따라 clone 대신 clone3를 쓸 수 있습니다. 같은 fork입니다.
- strace의 execve 횟수가 교안(4회)과 다름: PATH 순서가 다르기 때문입니다. `echo $PATH`와 대조하게 합니다.
- waitpid의 "still wait"가 4회: 정상입니다(폴링 시점의 경쟁). 영상도 4회였습니다.
- 좀비 명령에서 Z가 안 보임: `sleep 2` 전에 ps를 쳤거나 4초가 지난 것입니다. 명령을 한 줄로 그대로 다시 실행합니다.
- `make` 경고: Makefile의 `-D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE`가 빠지면 fork·getpid 선언 경고가 납니다. 제공한 Makefile을 씁니다.
