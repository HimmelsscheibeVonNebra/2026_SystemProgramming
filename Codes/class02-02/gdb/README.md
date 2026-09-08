# GDB 기본 실습

서비스에 설치하지 않는 별도 예제입니다. 이 폴더에서 일반 사용자로 실행합니다. Ubuntu PC 또는 VM을 사용하세요.

```bash
sudo apt update
sudo apt install gdb
gcc -std=c17 -Wall -Wextra -g -O0 helloworld.c -o helloworld
gdb ./helloworld
```

GDB 안에서 다음을 순서대로 입력합니다.

```text
break helloworld.c:8
run
list
print count
next
print count
continue
print count
next
print count
quit
```

제공 소스의 8행은 count++입니다. 처음 중단점에서는 증가 전이므로 count가 0이고, next 뒤에는 1입니다. continue로 다음 반복의 중단점에 도달하면 1이고, next 뒤에는 2입니다. 소스에 줄을 추가했다면 list로 count++의 새 줄 번호를 확인하세요. 종료 확인 질문이 나오면 y를 입력합니다.

next는 현재 표시한 줄을 실행하고 다음 줄에서 멈춥니다. continue는 다음 중단점까지 실행하므로 sleep을 지나며 약 2초 대기할 수 있습니다. quit은 GDB가 실행한 이 예제도 종료합니다. 기존 systemd 서비스와는 별도 프로세스입니다.

제출: 중단점 위치, next 전후의 count, next와 continue의 차이를 기록하세요.
