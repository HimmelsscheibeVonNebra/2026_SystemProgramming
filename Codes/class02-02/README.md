# class02-02: while 반복 서비스와 Makefile

1차시의 Hello World를 2초마다 출력하도록 바꿉니다. `class02-02` 폴더에서 실습합니다. C 코드에서는 `while (1)`, `printf`, `fflush`, `sleep`만 사용하며, 시스템 신호 처리는 이후 수업에서 다룹니다.

## 1. 반복 실행 (10분)

`helloworld.c`를 읽고 실행 결과를 예상합니다.

```bash
gcc -std=c17 -Wall -Wextra -g helloworld.c -o helloworld
./helloworld
```

2초마다 Hello World가 출력됩니다. 직접 실행한 프로그램은 Ctrl+C로 끝냅니다. `while (1)`은 조건이 항상 참인 반복문입니다. `sleep(2)`는 반복 사이에 대기하고, `fflush(stdout)`는 출력한 내용이 서비스 로그에도 바로 보이도록 합니다. 별도 종료 메시지는 출력하지 않습니다.

## 2. Makefile로 빌드 (10분)

```bash
make clean
make
make
touch helloworld.c
make
make run
```

`make run`은 Ctrl+C로 끝냅니다. 두 번째 make는 변경이 없으면 컴파일하지 않습니다. 소스 수정 시각을 바꾼 뒤에는 다시 컴파일합니다.

Makefile의 타깃, 의존 파일, 실행 명령을 찾아보세요. 명령 앞 들여쓰기는 탭입니다. `$@`는 타깃이고 `$<`는 첫 의존 파일입니다. `.PHONY`는 파일 이름이 아닌 작업을 표시합니다. `make clean`은 로컬 실행 파일만 제거합니다.

## 3. 서비스 교체와 로그 확인 (15분)

서비스 등록은 systemd가 실행되는 실제 Ubuntu 또는 Ubuntu VM에서 수행합니다. 1차시 서비스를 등록하지 않았다면 첫 stop 명령은 생략합니다.

```bash
sudo systemctl stop helloworld.service
make
sudo make install
sudo systemctl daemon-reload
sudo systemctl enable --now helloworld.service
systemctl status helloworld.service --no-pager
sudo journalctl -u helloworld.service -f
```

계속 실행되는 프로그램이므로 `Type=simple`을 사용합니다. 상태는 `active (running)`입니다. `journalctl -f`에서 Ctrl+C를 누르면 로그 보기만 끝나고 서비스는 계속 실행합니다.

```bash
sudo systemctl stop helloworld.service
systemctl is-active helloworld.service
sudo systemctl start helloworld.service
systemctl is-active helloworld.service
```

중지하면 inactive, 시작하면 active입니다. inactive일 때 `is-active`의 종료값은 0이 아닙니다. 이번 실습에는 자동 재시작 설정이나 장애 주입을 사용하지 않습니다.

## 4. 코드 변경과 재설치 (10분)

출력 문자열을 바꾸고 `sleep(2)`를 `sleep(5)`로 수정합니다.

```bash
sudo systemctl stop helloworld.service
make
sudo make install
sudo systemctl start helloworld.service
sudo journalctl -u helloworld.service -f
```

새 출력과 5초 간격을 확인하고 Ctrl+C로 로그 보기를 끝냅니다. 소스 변경은 빌드, 설치, 서비스 시작을 거쳐 반영합니다. unit 설정도 바꾸었다면 시작 전에 `sudo systemctl daemon-reload`를 수행합니다.

## 5. 제출과 정리 (5분)

- `while (1)`이 계속 실행되는 이유를 설명합니다.
- 두 번의 make 출력과 소스 수정 후 출력을 비교합니다.
- 서비스의 active/inactive 상태와 반복 로그를 기록합니다.
- 변경한 메시지와 출력 간격을 제출합니다.
- 직접 실행 중 Ctrl+C와 로그 보기 중 Ctrl+C의 차이를 설명합니다.

```bash
sudo systemctl disable --now helloworld.service
sudo make uninstall
sudo systemctl daemon-reload
make clean
```

Docker 학생은 1~2단계까지 진행하고 서비스 실습은 Ubuntu 환경에서 이어갑니다.
