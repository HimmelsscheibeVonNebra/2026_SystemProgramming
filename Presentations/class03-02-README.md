# Week03-class02 · 견고한 반복문과 파일 복사

저장소에서는 교안의 `labs/`가 루트의 `Codes/`에 해당합니다. 명령은 저장소 루트에서 시작합니다.

기존 class03-01에 이어 진행하는 120분 수업, 슬라이드 33장입니다.

## 구성

1. 지난 시간 복습: 파일 프로그램 4개와 반환값
2. read: 짧은 읽기, EOF, errno, EINTR
3. write_all: 부분 쓰기와 포인터·남은 길이 갱신
4. file_copy: 새 파일 복사, 오류 처리, 결과 비교
5. 분기 테스트, Linux strace 실습, 퀴즈

## 자료

- `class03-02.odp`: 기존 디자인을 이어받은 Impress 편집용 교안
- `class03-02.pdf`: 열람·배포용 교안
- `class03-02.md`: 슬라이드 원고와 교수자 메모
- `labs/class03-02/`: C 예제, 학생 구현 파일, Makefile, 테스트, 안내·기록지·해설

ODP의 발표자 노트와 원고에 함수별 참고 자료를 넣었습니다.
학생 실습은 `labs/class03-02/README.md`부터 진행합니다.
EOF까지 읽기와 write_all의 내부 쓰기 반복문을 연결해 바이트 단위 파일 복사를 완성합니다.
EOF와 오류는 종료 상태를 구분하고, 복사기는 기존 목적지를 보존합니다.

## 실행 확인

macOS에서 C17 및 `-Wall -Wextra -Wpedantic -Werror`로 컴파일하고 함수·통합 테스트를 확인했습니다.
학생용 빈 구현의 실패와 완성 함수 연결 후 성공도 확인했습니다.
Linux의 strace 및 /dev/full 실습은 실행 안내를 포함했으며 이번 제작 환경에서는 실행하지 않았습니다.
EOF·오류·부분 성공의 의미는 Linux man-pages를 기준으로 설명합니다.

## 참고

- [read(2)](https://man7.org/linux/man-pages/man2/read.2.html)
- [write(2)](https://man7.org/linux/man-pages/man2/write.2.html)
- [open(2)](https://man7.org/linux/man-pages/man2/open.2.html)
- [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)
