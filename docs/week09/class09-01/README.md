# Week09-class01 실습 · 시그널: 종료 상태·처리·관찰

**V01** · 2026-10-26

교안: [9주차 1차시 V04 PPTX](../../../output/week09/week09-class01-v04.pptx) (74장). 실습: [소스 폴더](../../../output/week09/labs/class09-01/). 교안의 `labs/class09-01/`은 이 소스 폴더에 해당합니다. 기록지: [REPORT.md](REPORT.md), 교수자 안내: [INSTRUCTOR.md](INSTRUCTOR.md).

프로그램이 어떻게 끝났는지(종료 코드·시그널·코어) 스스로 보고하게 하고, 같은 사실을 셸의 `$?`·`/proc/PID/status`·`kill -l`로 밖에서 다시 확인합니다. 준비 도구는 `cc`, `make`, `python3`이고 저장소 루트에서 아래 명령으로 실습 폴더에 들어간 뒤 실행합니다.

```sh
cd output/week09/labs/class09-01
make
python3 test_examples.py
```

| 예제 | 학습 내용 |
|---|---|
| exit_how.c | exit·abort·segv·term·wait 다섯 끝남을 fork·waitpid로 보고, raw status와 셸 `$?`의 대응 |
| catch_int.c | sigaction으로 시그널별 처리를 정하고, SIGKILL 거절(EINVAL)과 Ctrl-\ 무시까지 확인 |
| timeout_read.c | alarm으로 read에 시간 제한을 걸고, EINTR과 SA_RESTART의 차이를 관찰 |
| sigmask_show.c | /proc/PID/status의 SigPnd·SigBlk·SigIgn·SigCgt를 시그널 이름으로 풀어 보기 |
| sig_names.c | 시그널 번호↔이름↔기본 동작 참조 표, kill -l과 strsignal과 대조 |

Makefile은 C11과 POSIX API를 쓰고 경고를 오류로 처리합니다. `_DARWIN_C_SOURCE`는 macOS, `_DEFAULT_SOURCE`는 Linux에서 추가 선언을 켭니다.

`/proc` 관찰은 Linux에서만 됩니다. macOS에서는 sigmask_show가 "No such file or directory"를 내고 상태 1로 끝나는데, 이것도 기록할 관찰입니다. 실습 1–3의 /proc 명령은 Ubuntu 실습 서버에서 수행합니다.

출력 값은 실행마다 다릅니다(PID, 걸린 시간, 코어 파일 이름). 숫자를 맞히는 것이 아니라 프로그램의 자기 보고와 밖의 관찰이 같은 사실을 가리키는지, 그 관계를 설명할 수 있는지가 기록 대상입니다.

## 실습 1 · 프로그램은 어떻게 끝나나 (교안 1부)

예측: `sleep 100`을 Ctrl-C로 끊으면 `echo $?`는 몇일까요? Ctrl-\는요?

```sh
sleep 100                     # Ctrl-C, 그다음 echo $?
sleep 100                     # Ctrl-\ ("Quit (core dumped)"), echo $?
sleep 100                     # Ctrl-Z, jobs, kill %1, true로 알림 확인

./exit_how exit
./exit_how abort
./exit_how segv
./exit_how term
echo $?
./exit_how wait & sleep 0.3
c=$(pgrep -n exit_how)        # 다른 터미널에서
kill -STOP $c; sleep 0.3      # stopped 보고와 raw status
kill -CONT $c; sleep 0.3      # continued 보고
kill -KILL $c                 # killed by signal 9 보고

# 코어는 어디로? (서버, Ubuntu)
cat /proc/sys/kernel/core_pattern
ulimit -c
(ulimit -c unlimited; ./exit_how abort)
ls -l /var/lib/apport/coredump | tail -2
```

관찰할 점:

- 네 모드의 raw status(0x0300·0x0086·0x008b·0x000f)에서 시그널 번호와 종료 코드가 각각 어느 비트에 있는지 설명합니다.
- 같은 죽음을 셸은 `$?=128+시그널`로 보여 줍니다. term 다음 `$?`를 적고 raw status 0x000f와의 관계를 설명합니다.
- abort·segv 보고의 "core dumped"가 `ulimit -c 0`에서도 붙는 이유(waitpid가 알려 주는 것)와, 실제 코어 파일이 생긴 곳을 구분합니다.
- Ctrl-Z 다음 `$?`(예: 148)가 무슨 시그널 번호인지, SIGTSTP와의 관계를 설명합니다.
- `kill %1`은 SIGTERM을 보내는 작업 정리입니다. Terminated 표시가 프롬프트에 늦게 나타나는 시점을 관찰합니다.

## 실습 2 · 시그널을 잡다 (교안 2부)

예측: SIGKILL은 sigaction으로 무시하게 만들 수 있을까요?

```sh
./catch_int                   # Ctrl-C 세 번, 사이에 Ctrl-\도 눌러 본다
echo $?

./catch_int & sleep 0.3
./sigmask_show $!             # SigIgn·SigCgt에 내가 등록한 게 보이나?
kill -TERM $!; wait $!; echo "exit=$?"
```

관찰할 점:

- `sigaction(SIGKILL): Invalid argument` 줄을 적고, 커널이 SIGKILL·SIGSTOP 처리 변경을 거절하는 이유를 설명합니다.
- Ctrl-\를 눌러도 아무 일 없는 이유(SIGQUIT를 SIG_IGN으로 등록)를 적습니다.
- SigIgn의 QUIT 비트와 SigCgt의 INT·TERM 비트(예: SigIgn 0x4, SigCgt 0x4002)를 코드의 sigaction 호출과 대응합니다.
- 핸들러의 `[handler] SIGINT`(write) 줄과 main의 `SIGINT #n`(printf) 줄이 섞여 나오는 순서를 기록합니다.
- kill -TERM 뒤에도 `exit=0`인 이유(정리 후 정상 종료)를 적습니다.

## 실습 3 · 밖에서 보는 시그널 마스크 (교안 3부, 서버)

예측: nohup으로 실행한 프로세스의 SigIgn에는 무엇이 찍혀 있을까요?

```sh
./sigmask_show                # 자기 자신: 아직 아무것도 없다
./sigmask_show $$             # bash의 SigBlk·SigIgn·SigCgt
sleep 30 & ./sigmask_show $! | tail -2
nohup sleep 30 > /dev/null 2>&1 & ./sigmask_show $! | tail -2
(trap '' INT; trap 'echo hi' TERM; exec ./sigmask_show) | tail -2
kill %1 %2
```

관찰할 점:

- bash가 CHLD를 블록하고(SigBlk) 많은 시그널을 잡는(SigCgt) 이유를 작업 제어 셸의 역할과 연결해 설명합니다.
- sleep 30의 SigIgn·SigCgt가 0인데, nohup의 것은 HUP 1비트만 있다는 것을 확인하고 그 이유를 설명합니다.
- `trap '' INT`로 무시한 INT가 exec 뒤에도 무시로 남는 것을 기록합니다(무시는 상속됩니다).
- 무시(Ignore)와 블록(Block)의 차이를 한 문장으로 정리합니다.

## 실습 4 · 시간 제한과 EINTR (교안 4부)

예측: SA_RESTART를 켜면 alarm이 울린 뒤 read는 어떻게 될까요?

```sh
(sleep 4; echo hello) | ./timeout_read default 2; echo "exit=$?"
(sleep 4; echo hello) | ./timeout_read norestart 2; echo "exit=$?"
(sleep 4; echo hello) | ./timeout_read restart 2; echo "exit=$?"
```

관찰할 점:

- default 모드의 "Alarm clock"과 exit=142(128+14)를 설명합니다. 핸들러가 없으니 기본 동작(종료)이 일어났습니다.
- norestart 모드에서 read가 -1 EINTR로 돌아온 시각(2.00s)과 뒤이어 셸로 돌아온 exit=1을 적습니다.
- restart 모드에서 read가 4.00s에 6바이트를 읽은 이유(핸들러 끝나고 read가 재개)를 설명합니다.
- 세 모드에서 걸린 시간을 비교해 표로 정리합니다.

## 실습 5 · 번호와 이름 (교안 5부)

예측: `kill -l 10`은 서버와 macOS에서 같은 이름을 보여 줄까요?

```sh
kill -l                       # 셸이 아는 번호→이름
./sig_names                   # 프로그램의 표: 이름 + strsignal + 기본 동작
./sig_names 9
./sig_names 11
./sig_names 17
./sig_names 19
./sig_names 32                # 범위 밖 → usage(2)
```

관찰할 점:

- kill -l에는 없는 기본 동작(Term/Core/Stop/Cont/Ignore) 열에서, 잡으면 안 되는 시그널(KILL·STOP)과 무시되는 시그널(CHLD·URG·WINCH)을 찾습니다.
- 서버와 macOS에서 같은 번호가 다른 이름을 가리키는 예를 하나씩 적습니다(예: 10). 이름과 기본 동작 표는 x86-64 리눅스 기준이라는 주석이 프로그램에 있습니다.
- 32 이후 RT(실시간) 범위는 이 과목에서 다루지 않는다는 안내를 확인합니다.

## 종료 상태와 제출

- 0: 정상 완료 (macOS의 sigmask_show 실패(1)·block_pending의 unsupported는 문서화된 동작)
- 1: 조회·실패 (없는 /proc 열기, timeout_read의 EINTR 시간 초과 등 의도적인 실패 포함)
- 2: 잘못된 실행 인자

제출물은 C 소스 5개, Makefile, 작성한 REPORT.md, 실행 로그입니다. 선택 과제는 catch_int의 SIGINT 카운트를 `/proc/PID/status`의 SigCgt·SigPnd와 함께 보여 주는 `./sigmask_show PID` 관찰 로그를 REPORT에 붙이는 것입니다.

## 실행 확인 범위

2026-10-25 macOS arm64(clang)에서 다섯 예제를 경고 없이 컴파일하고 `python3 test_examples.py`(28회 실행)를 통과했습니다. 서버의 실행 화면은 Ubuntu 24.04(x86-64, 커널 6.8) 실습 서버 "playground"에서 2026-10-23에 캡처한 실제 결과이고, sig_names는 캡처 이후 추가된 참조 도구라 교안 화면에는 없습니다.

## 참고

[sigaction(2)](https://man7.org/linux/man-pages/man2/sigaction.2.html), [sigprocmask(2)](https://man7.org/linux/man-pages/man2/sigprocmask.2.html), [alarm(2)](https://man7.org/linux/man-pages/man2/alarm.2.html), [pause(2)](https://man7.org/linux/man-pages/man2/pause.2.html), [wait(2)](https://man7.org/linux/man-pages/man2/wait.2.html), [strsignal(3)](https://man7.org/linux/man-pages/man3/strsignal.3.html), [signal(7)](https://man7.org/linux/man-pages/man7/signal.7.html), [proc_pid_status(5)](https://man7.org/linux/man-pages/man5/proc_pid_status.5.html), [core(5)](https://man7.org/linux/man-pages/man5/core.5.html), [kill(1)](https://man7.org/linux/man-pages/man1/kill.1.html).


## 변경 이력

| 버전 | 날짜 | 내용 |
|---|---|---|
| V01 | 2026-10-26 | 9주차 1차시 실습 자료를 저장소에 추가. |
