# 교수자 안내 · 시그널: 종료 상태·처리·관찰

**V01** · 2026-10-26

실습은 프로그램이 스스로 보고한 값(종료 방식·raw status·플래그)과 밖의 창(셸 `$?`, `/proc/PID/status`, `kill -l`)을 맞춰 보는 방식입니다. 값은 환경마다 다르므로 숫자 일치가 아니라 두 창의 교차 확인과 관계 설명을 봅니다. 교안의 실행 화면은 Ubuntu 24.04 실습 서버 "playground"(x86-64, 커널 6.8)에서 2026-10-23에 캡처한 실제 화면입니다(shots/ 폴더의 k9_*.txt·png).

## 핵심 확인 사항

- exit_how의 서버 실측(raw status): exit 0x0300, abort 0x0086(core dumped), segv 0x008b(core dumped), term 0x000f. wait 모드: STOP 0x137f, CONT 0xffff, USR1 종료 0x000a. raw status의 하위 바이트가 시그널, 상위 바이트가 종료 코드라는 규칙을 이 네 값에서 직접 읽게 합니다.
- 같은 죽음을 셸은 `$?=128+시그널`로 보고합니다(k9_keys·k9_status): Ctrl-C→130, Ctrl-\→131(core dumped), Ctrl-Z→148(128+SIGTSTP 20), sleep에 kill -TERM→143, kill -KILL→137. 143과 raw status 0x000f가 같은 시그널 15의 두 표현이라는 점이 이 차시의 핵심 관계입니다.
- Ctrl-Z 뒤 `$?`가 148인 이유: bash가 정지도 "시그널 20으로 멈췄다"로 기록하기 때문입니다. `kill %1`은 정지 잡업에 SIGTERM을 보내고, Terminated 표시는 다음 프롬프트의 작업 상태 갱신 시점에 나타납니다(k9_keys).
- 코어(k9_core): 서버의 core_pattern은 apport 파이프(`|/usr/share/apport/apport ...`)이고 `ulimit -c` 기본값은 0입니다. `(ulimit -c unlimited; ./exit_how abort)`로 낸 코어는 `/var/lib/apport/coredump`에 307200바이트 파일로 남았습니다. "core dumped" 표시는 ulimit와 무관하게 waitpid가 알려 주는 것이고, 실제 파일은 core_pattern과 ulimit가 결정합니다 — 이 둘의 구분을 확인하게 합니다.
- catch_int(k9_catch): `sigaction(SIGKILL): Invalid argument`가 먼저 찍히고, Ctrl-C 세 번에 `[handler] SIGINT`→`main: SIGINT #n`이 한 쌍으로 나옵니다. Ctrl-\는 무시되고 SIGTERM에는 정리 메시지 후 exit 0입니다.
- 백그라운드 catch_int를 sigmask_show로 보면(k9_catchbg) SigIgn 0x0000000000000004(QUIT), SigCgt 0x0000000000004002(INT|TERM)입니다. 학생이 등록한 세 sigaction(SIGINT·TERM·QUIT)이 비트에 그대로 보입니다.
- bash의 마스크(k9_mask): SigBlk CHLD, SigIgn QUIT TERM TSTP TTIN TTOU, SigCgt 0x000000004b813efb(HUP INT ILL TRAP ABRT BUS FPE USR1 SEGV USR2 PIPE ALRM CHLD XCPU XFSZ VTALRM WINCH SYS). "왜 셸이 이렇게 많이 잡는가"를 작업 제어 논의로 연결합니다. 정답을 요구하지 말고 "자식 상태 변화(CHLD)를 페어링해 받고, 키 조작(INT·TSTP)을 직접 처리해야 한다" 수준이면 충분합니다.
- 상속(k9_inherit): sleep 30의 SigIgn·SigCgt는 0입니다. nohup sleep은 SigIgn에 HUP 한 비트만 있습니다. `(trap '' INT; ...; exec ./sigmask_show)`는 SigIgn INT로 exec를 건너 무시가 상속됩니다. nohup은 터미널 끊김(HUP)에 살아남게 하려는 도구라는 것을 마스크에서 직접 보게 합니다.
- timeout_read(k9_timeout): default 2 → "Alarm clock", exit=142. norestart → `[handler] SIGALRM` 후 `read: -1 EINTR after 2.00s`, exit=1. restart → `read: 6 bytes after 4.00s (alarms=1)`, exit=0. 세 줄의 걸린 시간(2.00/2.00/4.00)을 비교하게 하면 EINTR과 SA_RESTART의 차이가 숫자로 드러납니다.

## 로컬(macOS) 실측과 서버 기대값의 차이

2026-10-25 macOS arm64(clang, 기본 cc)에서 다섯 예제를 -Werror로 빌드했고 test_examples.py(28회 실행)를 통과했습니다. 차이는 다음과 같습니다.

- /proc가 없어 sigmask_show는 `/proc/self/status: No such file or directory`를 내고 상태 1로 끝납니다. 실습 3은 서버에서만 수행하게 안내합니다. block_pending·sig_names는 이 차이를 문서화된 동작(unsupported·리눅스 기준표 주석)으로 처리합니다.
- macOS의 strsignal과 kill -l은 다른 번호 체계입니다. sig_names에서 이름 열(리눅스 기준)과 strsignal 열이 어긋나는 것이 그 증거입니다: 10행이 macOS에서는 "Bus error"(SIGBUS), 17·18·19행이 CHLD·CONT·STOP인데 macOS 실측은 "Suspended (signal)"·"Suspended"·"Continued"(STOP·TSTP·CONT)로 나왔습니다. 서버에서는 두 열이 일치합니다. "같은 번호, 다른 시그널"을 로컬에서 먼저 보고 서버에서 확인하는 순서도 좋습니다.
- abort·segv의 raw status가 macOS에서는 0x0006·0x000b로 나왔습니다(코어 플래그 0x80 없음). 서버 기대값은 0x0086·0x008b입니다. abort 메시지도 macOS "Abort trap: 6" vs 서버 "Aborted"로 다릅니다.
- exit_how wait의 CONT 보고는 macOS waitpid가 WCONTINUED를 지원하지 않아 나오지 않았습니다(STOP 0x117f → 바로 KILL 보고). 서버에서는 continued/raw 0xffff가 나옵니다(k9_wait). test_examples.py는 이 차이를 플랫폼 분기로 반영했습니다.
- timeout_read의 세 모드 결과(EINTR 2.00s·재개 후 6바이트 4.00s)는 macOS에서도 서버와 같게 나왔습니다. default 모드의 exit는 셸이 계산한 142로 동일합니다.

## 평가 기준

숫자의 일치보다 다음을 설명했는지 봅니다: raw status 비트 배치와 `$?=128+시그널`의 관계, core dumped 표시와 실제 코어 파일의 구분, SIGKILL·SIGSTOP 처리 변경 불가의 이유, SigIgn·SigCgt 비트와 sigaction 호출의 대응, 무시의 상속(no·nohup·trap), EINTR과 SA_RESTART의 차이. 관찰 대상을 kill·wait로 정리했는지도 습관 지표로 봅니다.

## 진행 문제 해결

- Ctrl-\가 아무 일도 안 함: catch_int가 SIGQUIT를 SIG_IGN으로 등록한 정상 동작입니다. 죽여야 하면 kill -TERM 또는 Ctrl-C 세 번입니다.
- 코어 파일을 못 찾겠음: core_pattern이 apport 파이프인 서버에서는 coredump 디렉터리를 봅니다(README 실습 1). ulimit -c 0이면 파일이 안 남습니다.
- macOS에서 /proc 명령 실패: 정상입니다. sigmask_show 관찰은 서버에서 수행합니다.
- `pgrep -n exit_how`가 다른 것을 고름: 가장 최근 프로세스를 고르는 옵션입니다. wait 모드 인스턴스가 여러 개면 먼저 정리합니다.
- timeout_read restart가 4초 걸림: 정상입니다. read가 재개되어 파이프의 hello(6바이트)를 받은 것입니다.
- catch_int에서 Ctrl-C를 눌러도 종료 안 됨: 세 번 눌러야 합니다. SIGINT 카운트가 3에 도달하면 exit 0입니다.


## 변경 이력

| 버전 | 날짜 | 내용 |
|---|---|---|
| V01 | 2026-10-26 | 9주차 1차시 실습 자료를 저장소에 추가. |
