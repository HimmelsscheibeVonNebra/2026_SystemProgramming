# Week04-class01 · 리눅스 파일과 디렉토리

발표자료는 [`Presentations/class04-01.pdf`](../../Presentations/class04-01.pdf)에 있습니다. 발표자료의 `labs/class04-01/` 경로는 이 저장소의 `Codes/class04-01/`에 해당합니다.

저장소 루트에서 `cd Codes/class04-01` 후 `make`로 `mystat`과 `mini_ls`를 빌드합니다. Linux에서 실행하는 수업 예제입니다.

- `./mystat PATH`: `lstat`으로 종류, inode, 링크 수, 권한, 소유자, 크기를 표시합니다.
- `./mini_ls DIRECTORY`: 한 디렉토리의 항목을 `종류 inode 크기 이름` 순서로 출력합니다. 마지막 심볼릭 링크는 따라가지 않습니다.

`mini_ls`는 `.`과 `..`만 생략하므로 숨김 파일도 출력합니다. 정렬과 재귀 탐색은 구현하지 않습니다. 개별 항목의 조회 실패는 stderr에 기록하고 계속하며 최종 종료 상태는 1입니다. 디렉토리 자체를 열지 못하면 바로 실패합니다. 파일 이름의 개행 등 특수문자 escaping은 확장 과제입니다.

## 실습

강의 15쪽에서 `RUN=$(mktemp -d)`로 만든 폴더를 이후 실습에서도 같은 셸에서 사용합니다. 강의의 `rm` 명령은 이 폴더에 만든 예제 파일에만 적용합니다.

1. 일반 파일과 디렉토리의 종류·크기를 비교합니다.
2. 하드 링크와 심볼릭 링크를 만들고 inode·링크 수를 비교합니다.
3. 원래 이름을 삭제한 뒤 두 링크의 차이를 관찰합니다.
4. 빈 디렉토리, 숨김 파일, 공백이 있는 이름, 없는 경로를 검사합니다.
5. 실행 직후 `echo $?`로 종료 상태를 확인합니다.

예제 코드는 로컬 macOS의 POSIX 환경에서 경고를 활성화해 빌드하고, 링크·디렉토리·오류 입력으로 검증했습니다. Linux API 동작은 Linux man-pages와 대조했습니다. 이 작업 환경에서는 Docker 데몬이 실행되지 않아 Linux 컨테이너 실행은 검증하지 않았습니다. `strace` 명령은 수업의 Ubuntu 환경에서 실행합니다.

참고: [stat(2)](https://man7.org/linux/man-pages/man2/stat.2.html), [readdir(3)](https://man7.org/linux/man-pages/man3/readdir.3.html), [link(2)](https://man7.org/linux/man-pages/man2/link.2.html), [symlink(7)](https://man7.org/linux/man-pages/man7/symlink.7.html)
