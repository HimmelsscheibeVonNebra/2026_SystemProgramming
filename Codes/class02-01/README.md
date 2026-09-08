# 첫 수업 실습: 실행하고 비교하기

이 자료의 목표는 파일·권한·프로세스·네트워크의 변화를 직접 만들고 관찰하는 것입니다. 모든 명령은 Ubuntu 22.04의 Bash에서 실행합니다. Docker 명령만 Windows PowerShell에서 실행합니다.

## 준비

배포 자료의 `labs/class02-01` 폴더 전체를 다음 위치에 복사합니다.

- Windows Docker: 기존에 연결한 Windows `work` 폴더 안으로 복사. 컨테이너 경로는 `/work/class02-01`.
- 실제 Ubuntu: `~/week02/class02-01`.

일반 사용자 Bash에서 해당 폴더로 이동합니다. 아래 두 cd 중 자신의 환경에 맞는 것 하나만 실행합니다.

```bash
cd /work/class02-01
# 실제 Ubuntu에서는 대신: cd ~/week02/class02-01
lab_dir=$(bash setup.sh)
cd "$lab_dir"
pwd
find . -maxdepth 2 -type f
whoami
id
```

setup.sh는 현재 폴더 아래에 새 run-XXXXXX 폴더를 만들며 기존 실습 파일을 덮어쓰지 않습니다. 경로는 직접 출력한 pwd로 확인하세요. `data/items.txt`, `web/index.html`, `logs/auth-demo.log` 세 파일이 있어야 합니다. 이후 같은 Bash에서 실습합니다. 새 터미널이면 pwd에서 확인한 경로로 다시 이동하세요.

## 실습 1. 파일 복사·검색·비교 (5분)

```bash
cat data/items.txt
wc -l data/items.txt
grep -c '^alpha$' data/items.txt
cp data/items.txt data/copy.txt
printf 'gamma\n' >> data/copy.txt
diff -u data/items.txt data/copy.txt
echo $?
find data -type f
```

예측: 원본은 3줄, alpha는 2개입니다. diff에서 `+gamma`를 찾으세요. 차이가 있으므로 종료값은 1입니다. `>`와 `>>` 차이를 복사본으로 시험하고 REPORT.md에 적으세요. 원본을 바꿨다면 새 run 폴더를 생성하여 다시 시작할 수 있습니다.

## 실습 2. 권한과 예상한 오류 (5분)

Windows 바인드 마운트에서는 Unix 권한을 그대로 적용하지 못할 수 있습니다. Docker 학생은 먼저 아래 명령으로 컨테이너의 파일시스템으로 이동합니다. 실제 Ubuntu 학생은 현재 폴더에서 진행합니다.

```bash
# Docker 학생만 실행
permission_dir=$(mktemp -d /tmp/lsp-permission-XXXXXX)
cd "$permission_dir"
```

```bash
printf 'lab secret\n' > private.txt
chmod 600 private.txt
ls -l private.txt
cat private.txt
chmod 000 private.txt
cat private.txt
chmod 600 private.txt
cat private.txt
```

일반 사용자라면 000 뒤에 Permission denied, 복구 뒤에는 lab secret이 나와야 합니다. sudo나 root로 수행하면 비교가 성립하지 않을 수 있습니다. Docker 학생은 `cd -`로 원래 run 폴더로 복귀합니다. 권한 실습 파일은 /tmp의 개인 폴더에만 남습니다.

## 실습 3. APT 목록 갱신과 설치 구분 (5분 + 다운로드)

실제 Ubuntu:

```bash
dpkg-query -W bash > bash-before.txt
sudo apt update
apt list --upgradable
apt -s upgrade
dpkg-query -W bash > bash-after.txt
diff -u bash-before.txt bash-after.txt
echo $?
```

Docker에서는 별도 PowerShell에서 `docker exec -u 0 -it lsp-week02 bash`로 root Bash를 엽니다. `cd /work/class02-01/run-실제이름`으로 자신의 폴더로 이동한 뒤 위 명령에서 **sudo만 생략**합니다. 끝나면 `exit`로 나가고 기존 student Bash로 돌아옵니다.

다른 자동 업데이트가 없다면 bash의 설치 버전이 같고 diff 종료값은 0입니다. update의 오류를 먼저 확인하세요. `apt -s upgrade`는 변경 예정 내역을 보여 주는 시뮬레이션입니다. 실제 `sudo apt upgrade`는 슬라이드의 패키지 갱신 절차대로 변경 내역을 확인하며 수행합니다.

## 실습 4. 내 프로세스 시작·관찰·종료 (5분)

```bash
sleep 300 &
lab_pid=$!
printf 'PID=%s\n' "$lab_pid"
ps -o pid,ppid,user,stat,args -p "$lab_pid"
jobs -l
kill "$lab_pid"
wait "$lab_pid"
ps -p "$lab_pid"
```

같은 Bash에서 실행해야 변수와 jobs가 유지됩니다. wait의 종료값이 0이 아닌 것은 신호로 종료했기 때문일 수 있습니다. 마지막 ps에 sleep이 없어야 합니다. 본인이 생성한 PID만 종료하세요. 300초가 지나 자연 종료했으면 다시 시작합니다.

## 실습 5. 로컬 웹 서버와 포트 (10분)

도구 준비: 실제 Ubuntu는 `sudo apt update`와 `sudo apt install python3 curl`을 실행합니다. 기존 Docker 이미지에는 python3를 별도로 설치해야 합니다. PowerShell에서 `docker exec -u 0 -it lsp-week02 bash`를 연 뒤 root Bash에서 `apt update`, `apt install python3 curl`, `exit`를 차례로 실행합니다. 이후 기존 student Bash로 돌아옵니다.

run 폴더에서 `python3 --version`과 `curl --version`을 확인한 뒤:

```bash
ip -br a
ss -lnt 'sport = :8000'
```

8000번 포트가 이미 사용 중이면 멈추고 모든 8000을 다른 빈 포트로 바꿉니다. 다른 프로그램을 종료하지 마세요.

```bash
python3 -m http.server 8000 --bind 127.0.0.1 \
  --directory web > web.log 2>&1 &
web_pid=$!
sleep 1
ss -lntp 'sport = :8000'
curl --noproxy '*' http://127.0.0.1:8000/
kill "$web_pid"
wait "$web_pid"
ss -lnt 'sport = :8000'
```

중간 ss에는 127.0.0.1:8000의 LISTEN과 python3 프로세스가 보여야 합니다. curl의 예상 출력은 `Hello Linux lab!`입니다. 마지막 ss에서 해당 행이 사라져야 합니다. 오류는 `cat web.log`로 확인합니다. 시작이 늦다면 curl을 잠시 뒤 재시도하세요. 서버와 curl 모두 같은 Ubuntu/컨테이너 안에서 실행합니다. Windows 브라우저나 포트 공개 옵션은 사용하지 않습니다.

## 실습 6. 모의 로그의 사실과 추측 (5분)

아래 파일은 교육용 가상 로그입니다. 192.0.2.10만 학생이 사용한 주소라고 가정합니다.

```bash
cat logs/auth-demo.log
grep -c 'Failed password' logs/auth-demo.log
grep -c 'Accepted' logs/auth-demo.log
grep '198.51.100.23' logs/auth-demo.log
grep 'Accepted password' logs/auth-demo.log
```

실패 2건과 성공 2건을 찾으세요. 먼저 확인할 성공 기록은 무엇인가요? 실제 침해를 단정하기 전에 확인할 질문을 두 개 작성하세요. 실제 공격이나 SSH 접속을 수행하지 않습니다.

## 실습 7. Hello World 서비스 등록 (20분)

`service/README.md`로 이어서 진행합니다. C 프로그램을 직접 실행한 뒤 systemd에 등록하고 실행 결과를 로그에서 확인합니다. systemd 실습은 실제 Ubuntu 또는 Ubuntu VM에서 진행합니다. 시간에 따라 실습 5~6은 선택 실습으로 돌리고 서비스 등록까지 마무리하세요.

## 제출

상위 class01의 REPORT.md 사본에 실행 명령, 관찰 결과, 해석을 작성하세요. 교수자용 INSTRUCTOR.md는 수업 운영자가 따로 보관하면 됩니다. 예시 출력과 다른 환경값 자체는 오답이 아닙니다.
