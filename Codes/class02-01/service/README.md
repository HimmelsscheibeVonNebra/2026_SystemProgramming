# 1차시 마무리: Hello World 서비스 등록

목표: 소스 코드, 실행 파일, 서비스 설정 파일을 구분하고 Hello World를 systemd로 실행합니다. Ubuntu의 일반 사용자 터미널에서 이 폴더로 이동합니다. 기존 수업 Docker에서는 1단계까지만 진행합니다.

## 1. 컴파일과 직접 실행 (5분)

```bash
gcc -std=c17 -Wall -Wextra -g helloworld.c -o helloworld
./helloworld
echo $?
```

`Hello, world!`가 한 번 출력되고 종료값은 0입니다. `gcc`는 실행 파일을 만들고 `./helloworld`는 그 파일을 실행합니다.

## 2. 서비스 등록 (10분)

다음 명령은 수업용 `helloworld` 실행 파일과 서비스를 설치합니다. 같은 이름의 기존 서비스가 있는 PC에서는 별도 실습 VM을 사용하세요.

```bash
ps -p 1 -o comm=
```

결과가 `systemd`인 Ubuntu에서 이어갑니다.

```bash
sudo install -m 0755 helloworld /usr/local/bin/helloworld
sudo install -m 0644 helloworld.service /etc/systemd/system/helloworld.service
sudo systemctl daemon-reload
sudo systemctl start helloworld.service
systemctl status helloworld.service --no-pager
sudo journalctl -u helloworld.service -n 20 --no-pager
```

`Type=oneshot`은 한 번 실행하고 끝나는 작업입니다. `RemainAfterExit`을 설정하지 않았으므로 성공 후 `inactive (dead)`로 표시되는 것이 정상입니다. 이 상태에서 `systemctl status`의 종료값이 0이 아닐 수 있습니다. 로그의 출력과 성공 여부를 함께 확인하세요.

`ExecStart`는 실행 파일의 절대 경로입니다. `DynamicUser=yes`로 임시 비특권 계정에서 실행하며, 표준 출력·오류는 journal에 기록합니다.

## 3. 시작과 부팅 등록 구분 (5분)

```bash
sudo systemctl enable helloworld.service
systemctl is-enabled helloworld.service
sudo systemctl start helloworld.service
sudo journalctl -u helloworld.service -n 20 --no-pager
```

`start`는 지금 실행하고 `enable`은 부팅 시 실행하도록 등록합니다. `daemon-reload`는 변경한 서비스 설정을 읽게 하는 명령입니다. 실행 파일을 다시 컴파일하거나 서비스를 시작하는 명령이 아닙니다.

제출: 컴파일 명령, 서비스 상태, Hello World 로그를 기록하고 "프로그램이 성공했는데 왜 inactive인가?"와 "start와 enable의 차이는?"에 답하세요. 다음 차시에는 이 서비스를 반복 실행하도록 바꿉니다.

실습을 여기서 끝낼 경우에만 정리합니다.

```bash
sudo systemctl disable --now helloworld.service
sudo rm /etc/systemd/system/helloworld.service /usr/local/bin/helloworld
sudo systemctl daemon-reload
```
