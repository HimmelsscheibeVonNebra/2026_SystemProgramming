# 교수자용 해설

기본 파일·권한·프로세스·로그 실습은 25분, APT와 웹 서버까지 40분 정도를 배정합니다. 환경 구축 시간은 별도입니다. 학생 배포본에서 이 파일을 제외할 수 있습니다.

- 준비: 실행할 때마다 새 run 폴더와 세 입력 파일. 학생이 반드시 일반 사용자로 실행하는지 확인합니다.
- 파일: wc 3, grep 2, diff +gamma. 종료값 1은 diff에서 차이를 뜻합니다. >는 기존 내용 대체, >>는 추가입니다.
- 권한: 600은 rw-------, 000은 ---------. 일반 사용자는 000 파일 읽기에 실패하나 소유자로서 chmod는 가능합니다. root 권한의 우회 및 Windows 마운트 권한 차이를 설명하고 /tmp 실습으로 대조합니다.
- APT: update는 목록 갱신, -s upgrade는 예정 변경 시뮬레이션. 둘 다 bash 설치 파일을 갱신하지 않습니다. 백그라운드 자동 업데이트가 동시에 수행되면 버전이 달라질 수 있습니다. 실제 upgrade는 별도 기존 슬라이드에서 진행합니다.
- 프로세스: PID는 환경별로 다름, 소유자는 실행자. sleep 대기 상태는 보통 S이며 고정 답으로 요구하지 않습니다. SIGTERM 후 Bash wait는 보통 143, ps는 헤더만 나오고 보통 종료값 1입니다.
- 웹: 시작 전과 종료 후 8000 LISTEN 없음. 시작 중 127.0.0.1:8000과 python3 PID, curl 본문 Hello Linux lab!. 컨테이너의 루프백은 Windows 호스트의 루프백과 다릅니다. ss -ntp state established는 짧은 HTTP 요청이 끝나면 해당 연결이 없을 수 있습니다.
- 로그: 실패 2, 성공 2. 198.51.100.23의 guest 실패 2건 후 student 성공 1건. 실패와 성공 계정이 다름에 주의합니다. 성공 출발지의 VPN/NAT/관리자 작업 여부, 계정 소유자의 실제 접속 여부, 시간대 일치, 관련 프로세스·로그를 확인합니다. 합성 로그만으로 실제 침해를 판정하지 않습니다.

평가: 실행 증거 40%, 출력 해석 40%, 관찰/추측 구분 20%를 예시 배점으로 사용할 수 있습니다. 공식 강의계획서 평가 비율을 바꾸는 제안은 아닙니다.

출처: https://docs.python.org/3/library/http.server.html , https://manpages.ubuntu.com/manpages/jammy/man8/apt.8.html , https://manpages.ubuntu.com/manpages/jammy/man8/ss.8.html , https://www.rfc-editor.org/rfc/rfc5737
