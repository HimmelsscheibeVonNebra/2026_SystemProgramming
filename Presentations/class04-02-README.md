# Week04-class02 · 리눅스 파일 속성

저장소에서는 교안의 `labs/`가 루트의 `Codes/`에 해당합니다. 명령은 저장소 루트에서 시작합니다.

38장 교안과 120분 수업 자료입니다. class04-01의 디자인을 이어 사용하며, 파일 속성을 조회하고 작업 전후의 변화를 비교합니다.

## 자료 구성

| 파일 | 용도 |
|---|---|
| [class04-02.odp](class04-02.odp) | LibreOffice Impress 편집본 |
| [class04-02.pptx](class04-02.pptx) | PowerPoint 편집본 |
| [class04-02.pdf](class04-02.pdf) | 학생 배포·인쇄용, 38쪽 |
| [class04-02.md](class04-02.md) | 슬라이드 내용과 발표자 노트, 참고 문서 |
| [class04-02-script.md](class04-02-script.md) | 설명 멘트·시연·질문을 포함한 120분 대본 |
| [labs/class04-02/README.md](../Codes/class04-02/README.md) | C 실습 5개와 실행 안내 |
| [labs/class04-02/REPORT.md](../Codes/class04-02/REPORT.md) | 학생용 예측·관찰·해석 기록지 |
| [labs/class04-02/INSTRUCTOR.md](../Codes/class04-02/INSTRUCTOR.md) | 교수자용 해설과 퀴즈 답안 |

ZIP을 풀면 `class04-02/` 폴더 아래에 교안과 실습이 함께 있습니다. ZIP 안에서는 이 안내 파일의 이름이 `README.md`입니다.

## 추가한 그림

교안 4–7쪽에 CS 537 원문 이미지 4개를 추가했습니다. 디렉토리 트리, 절대 경로, 상대 경로, 디렉토리·inode·데이터 블록을 차례로 설명합니다. 원본 이미지와 출처·이용 안내는 [assets/cs537/README.md](assets/cs537/README.md)에 있습니다. 도입부 설명 시간을 조정해 전체 수업은 120분을 유지합니다.

## 학습 목표

- stat·lstat·fstat의 조회 대상을 구별하고 실패를 처리한다.
- struct stat에서 종류, 권한, 소유권, 크기, 시간, 링크 수를 해석한다.
- chmod와 umask의 적용 시점을 구분하고 권한 변경을 확인한다.
- 논리적 크기와 실제 할당량, atime·mtime·ctime을 구별한다.
- 마지막 이름을 unlink한 뒤에도 열린 FD로 접근할 수 있는 이유를 설명한다.

## 수업 흐름

| 시간 | 슬라이드 | 주제 |
|---|---|---|
| 00–10분 | 1–7 | 파일 내용·속성·이름, 디렉토리와 inode 관계 그림 |
| 10–40분 | 8–16 | 속성 조회, 링크와 inode, 조회 실패 |
| 40–50분 | — | 휴식 |
| 50–80분 | 17–25 | rwx, chmod, umask, 소유권 |
| 80–105분 | 26–32 | 희소 파일, 크기와 할당량, 시간 속성 |
| 105–115분 | 33–35 | unlink 이후 fstat과 read |
| 115–120분 | 36–38 | 확인 문제, 제출, 참고 문서 |

## 실습 시작

Ubuntu의 일반 사용자 환경에서 `cc`, `make`, `python3`를 준비합니다. 다음 명령은 자료의 최상위 폴더에서 실행합니다.

```sh
cd Codes/class04-02
make
make check
```

예제는 `file_attr`, `mode_demo`, `sparse_demo`, `time_attrs`, `fd_unlink_demo`입니다. 순서대로 실습 README의 명령을 실행하고 REPORT.md를 작성합니다. 각 프로그램의 소스와 Makefile을 포함했으므로 수업 환경에서 직접 컴파일합니다.

자료 제작 시 macOS arm64에서 C 예제 5개의 컴파일과 29회 실행 검사를 통과했습니다. Linux 실행은 제작 환경에서 확인하지 않았습니다. Ubuntu에서 `make check`로 확인하며, GNU stat·du 명령은 실습 README의 Linux용 옵션을 사용합니다.

## 편집과 참고

교안 글꼴은 `Noto Sans CJK KR`, 코드 글꼴은 `Noto Sans Mono CJK KR`입니다. 편집 환경에 글꼴이 없으면 줄바꿈이 달라질 수 있으며, 배포용 PDF는 제작 시의 배치를 유지합니다.

API 출처는 슬라이드 발표자 노트와 Markdown 교안에 연결했습니다. 주요 문서는 Linux man-pages의 [stat(2)](https://man7.org/linux/man-pages/man2/stat.2.html), [inode(7)](https://man7.org/linux/man-pages/man7/inode.7.html), [chmod(2)](https://man7.org/linux/man-pages/man2/chmod.2.html), [umask(2)](https://man7.org/linux/man-pages/man2/umask.2.html), [utimensat(2)](https://man7.org/linux/man-pages/man2/utimensat.2.html), [unlink(2)](https://man7.org/linux/man-pages/man2/unlink.2.html)입니다.
