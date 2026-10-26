# Week09-class01 · 시그널 기본 실습 기록지

**V01** · 2026-10-26

- 이름 / 학번:
- 운영체제·아키텍처 (Linux 관찰은 어느 서버에서?):
- `python3 test_examples.py` 결과:

## 1. 프로그램은 어떻게 끝나나 (exit_how)

| 모드 | 프로그램의 보고 | raw status | 같은 끝남을 셸에서 보면 $?는 |
|---|---|---|---|
| exit | exited, status=3 | | |
| abort | killed by signal 6 | | |
| segv | killed by signal 11 | | |
| term | killed by signal 15 | | |
| wait + STOP | stopped by signal | | — |
| wait + CONT | continued | | — |
| wait + KILL | killed by signal 9 | | |

- raw status에서 시그널 번호가 놓이는 자리 (abort 0x0086 예):
- exit 모드에서 3이 놓이는 자리 (0x0300 예):
- WIFEXITED·WIFSIGNALED가 구분하는 비트:
- "core dumped" 표시가 붙은 모드와, 실제 코어 파일이 생긴 곳(core_pattern·ulimit -c):

## 2. 셸 키와 $?

| 입력 | 셸이 보여 준 것 | echo $? | 128+n으로 읽으면 시그널은? |
|---|---|---|---|
| Ctrl-C | | | |
| Ctrl-\ | | | |
| Ctrl-Z | | | |
| kill %1 (Stopped 잡업) | | — | |

## 3. 시그널을 잡다 (catch_int)

| 시그널 | 예측 | 실제 |
|---|---|---|
| Ctrl-C 1·2회 | | |
| Ctrl-C 3회 | | |
| Ctrl-\ (SIGQUIT) | | |
| kill -TERM | | |
| sigaction(SIGKILL, SIG_IGN) | | |

- 백그라운드 실행 뒤 sigmask_show로 본 SigIgn / SigCgt 값:
- 그 값이 코드의 세 sigaction 호출과 대응하는 방식:
- SIGKILL·SIGSTOP의 처리를 바꿀 수 없는 이유:

## 4. 밖에서 보는 마스크 (sigmask_show, 서버)

| 대상 | SigPnd | SigBlk | SigIgn | SigCgt |
|---|---|---|---|---|
| sigmask_show 자신 | | | | |
| bash ($$) | | | | |
| sleep 30 | | | | |
| nohup sleep 30 | | | | |
| trap '' INT + exec | | | | |

- bash의 SigCgt에 INT·CHLD·ALRM이 있는 이유(작업 제어 셸의 일):
- nohup이 SigIgn에 남긴 것과 그 이유:
- 무시(Ignore)와 블록(Block)의 차이:

## 5. 시간 제한 (timeout_read)

| 모드 | read의 돌아옴 | 걸린 시간 | exit |
|---|---|---|---|
| default 2 | | | |
| norestart 2 | | | |
| restart 2 | | | |

- default 모드에서 "Alarm clock"이 나온 이유:
- EINTR이 말해 주는 것:
- SA_RESTART가 해 주는 일과, 그래도 EINTR이 남는 경우:

## 6. 번호와 이름 (sig_names)

- kill -l에는 없고 sig_names에만 있는 정보:
- 서버와 macOS(또는 로컬)에서 같은 번호가 다른 이름인 예:
- 기본 동작이 Ignore인 시그널 둘과 그 추측 이유:

## 정리 (서술형)

- SIGKILL에 sigaction이 실패하는 이유:
- 셸의 $? 130과 raw status 0x0002의 관계:
- nohup이 SIGHUP을 무시로 만드는 이유:
- EINTR와 SA_RESTART의 차이:
- 프로그램의 자기 보고와 /proc·셸 관찰이 일치했던 예 하나:


## 변경 이력

| 버전 | 날짜 | 내용 |
|---|---|---|
| V01 | 2026-10-26 | 9주차 1차시 실습 자료를 저장소에 추가. |
