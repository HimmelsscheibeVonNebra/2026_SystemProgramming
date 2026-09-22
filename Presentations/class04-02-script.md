# Week04-class02 수업 진행 대본

**리눅스 파일 속성 · 교안 38장 · 120분(휴식 10분 포함)**

설명 멘트, 터미널 시연, 학생 예측과 실행, 확인 질문을 연결한 대본입니다. 시간은 수업 시작 후 누적 분이며 활동 시간을 포함합니다. 파일 속성의 의미와 작업 전후 변화를 중심으로 진행합니다.

## 수업 전 준비

- Ubuntu 일반 사용자 환경에서 cc, make, python3를 준비합니다.
- labs/class04-02에서 make와 make check를 실행합니다.
- 기본 ACL이 없는 본인 실습 디렉토리를 사용합니다.
- file_attr.c, mode_demo.c, sparse_demo.c, time_attrs.c, fd_unlink_demo.c를 엽니다.
- 실습 1의 RUN을 이후에도 쓰므로 같은 터미널을 유지합니다.
- 명령은 한 줄씩 실행합니다. 의도적인 실패가 있으므로 전체를 set -e로 일괄 실행하지 않습니다.

```sh
cd Codes/class04-02
make
make check
```

## 시간표

| 누적 시간 | 슬라이드 | 내용 |
|---|---|---|
| 00–10분 | 1–7 | 내용·속성·이름과 디렉토리 관계 그림 |
| 10–40분 | 8–16 | 속성 조회와 링크, 실습 1 |
| 40–50분 | — | 휴식 |
| 50–80분 | 17–25 | 권한과 소유권, 실습 2 |
| 80–105분 | 26–32 | 크기와 시간, 실습 3·4 |
| 105–115분 | 33–35 | 이름 삭제 후 속성, 실습 5 |
| 115–120분 | 36–38 | 퀴즈와 제출 |

## 슬라이드 1 · 리눅스 파일 속성

**00–01분 · 1분**

> 오늘은 파일 속성을 집중해서 살펴보겠습니다. 파일 내용을 읽고 쓰는 것과, 파일의 종류·권한·소유권·크기·시간을 확인하는 것은 서로 다른 작업입니다. 같은 파일에 여러 이름이 있을 때 어떤 속성을 공유하는지도 실험하겠습니다.

**진행:** 지난 시간의 mystat 예제를 잠깐 보여 주고 오늘의 file_attr가 출력하는 항목을 소개한다.

**확인 질문:** 내용이 없는 0바이트 파일에도 권한이 있을까요?

**예상 답변:** 파일이 존재하면 내용이 비어 있어도 종류·권한·소유자 등의 속성이 있다.

교수자 참고: 파일과 디렉토리 개요를 복습한 뒤 속성 조회와 변경 결과를 깊이 살펴본다. 학생은 각 값의 의미를 설명하고, 파일 내용 변경과 속성 변경을 구분한다.

## 슬라이드 2 · 수업 구성

**01–02분 · 1분**

> 조회 방법을 익힌 뒤 권한을 바꿔 보고, 크기와 시간을 비교하겠습니다. 마지막에는 파일 이름을 지운 뒤 열린 FD의 속성을 조회합니다. 40분까지 진행하고 10분 쉽니다.

**진행:** 학생이 labs/class04-02에서 make를 실행했는지 확인한다. REPORT.md를 열도록 한다.

**확인 질문:** 실습에서 모든 학생의 inode 번호가 같아야 하나요?

**예상 답변:** 아니다. 번호 자체보다 작업 전후와 두 이름 사이의 관계를 비교한다.

교수자 참고: 40분 뒤 휴식 10분을 포함해 총 120분이다. 전반부는 stat 계열의 대상과 반환값, 후반부는 권한·크기·시각·링크 수의 변화를 비교한다.

## 슬라이드 3 · 파일 내용과 파일 속성

**02–03분 · 1분**

> read와 write는 내용을 다루고 stat 계열은 속성을 다룹니다. 파일 이름은 디렉토리의 정보입니다. 내용이 똑같은 두 파일도 서로 다른 inode와 소유자를 가질 수 있습니다. 오늘은 이 세 가지를 구별하며 결과를 읽습니다.

**진행:** 이전 stat 필드를 짧게 떠올린 뒤 내용·속성·이름의 차이를 확인하고 다음 그림으로 연결한다.

**확인 질문:** 파일을 복사했을 때 내용이 같으면 같은 파일이라고 해도 될까요?

**예상 답변:** 바이트 내용은 같을 수 있지만 별도 inode를 가진 다른 객체일 수 있다.

교수자 참고: 빈 파일도 종류·권한·소유자 등의 속성을 가진다. 파일 이름은 struct stat의 필드가 아니다. inode 번호는 같은 파일시스템 안에서 해석하며 파일 비교 시 st_dev와 st_ino를 함께 본다.

원문: [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html), [stat.2](https://man7.org/linux/man-pages/man2/stat.2.html)

## 슬라이드 4 · 디렉토리 트리

**03–04분 · 1분**

> 디렉토리는 이름을 찾는 출발점입니다. 루트에서 usr, local을 차례로 선택하면 /usr/local이라는 경로가 됩니다. 디렉토리도 inode를 가지며 그 내용에는 이름과 inode 번호의 연결이 들어 있습니다.

**진행:** 그림의 루트와 usr, local을 순서대로 가리킨다. 그림은 예시 트리임을 알린다.

**확인 질문:** /usr/local에서 중간의 usr은 어떤 종류의 파일이어야 하나요?

**예상 답변:** 다음 이름을 검색할 수 있는 디렉토리여야 한다.

교수자 참고: 그림의 루트 /와 usr, local을 가리키며 /usr/local을 읽는다. 디렉토리도 inode를 가진다. 디렉토리의 데이터에는 이름과 inode 번호의 연결이 들어간다. 그림은 원문의 역사적인 디렉토리 예시이며 모든 Linux 설치의 실제 디렉토리 구조를 뜻하지 않는다.

원문: [s25](https://pages.cs.wisc.edu/~bart/537/lecturenotes/s25.html), [s27.unixdir.gif](https://pages.cs.wisc.edu/~bart/537/lecturenotes/figures/s27.unixdir.gif), [directory](https://docs.kernel.org/filesystems/ext4/directory.html)

## 슬라이드 5 · 절대 경로의 탐색

**04–05분 · 1분**

> 절대 경로는 루트부터 해석합니다. 빨간 선을 따라 usr, bart, git을 차례로 찾고 마지막 gui.py에 연결된 inode를 찾습니다. 전체 경로 문자열이 파일 inode의 이름 필드에 저장되는 구조라고 생각하지 않도록 합니다.

**진행:** 그림 아래의 경로를 읽으며 각 이름에 해당하는 빨간 노드를 짚는다.

**확인 질문:** 현재 작업 디렉토리를 바꿔도 이 절대 경로의 시작점은 어디인가요?

**예상 답변:** 프로세스가 보는 루트 디렉토리다.

교수자 참고: 그림의 빨간 경로를 따라 /usr/bart/git/gui.py를 읽는다. 각 디렉토리의 이름 엔트리에서 다음 inode를 찾는 과정으로 연결해 설명한다. 절대 경로의 시작점은 프로세스가 보는 루트이며 이 수업의 그림은 마운트·심볼릭 링크 등의 세부 동작을 생략했다.

원문: [s25](https://pages.cs.wisc.edu/~bart/537/lecturenotes/s25.html), [AbsolutePathNameExample.JPG](https://pages.cs.wisc.edu/~bart/537/lecturenotes/figures/AbsolutePathNameExample.JPG), [path_resolution.7](https://man7.org/linux/man-pages/man7/path_resolution.7.html)

## 슬라이드 6 · 상대 경로와 현재 디렉토리

**05–06분 · 1분**

> 상대 경로는 현재 작업 디렉토리를 기준으로 합니다. 지금은 /usr/bart에서 prog.c를 열었으므로 /usr/bart/prog.c를 찾습니다. 같은 prog.c 문자열도 현재 디렉토리가 바뀌면 다른 파일을 가리킬 수 있습니다.

**진행:** bart의 빨간 테두리와 prog.c로 연결되는 빨간 선을 가리킨다.

**확인 질문:** 현재 디렉토리가 /usr/bart/git이라면 prog.c가 같은 파일을 뜻하나요?

**예상 답변:** 그렇지 않다. 그 디렉토리에서 prog.c를 새로 검색한다.

교수자 참고: 그림의 빨간 테두리 bart는 현재 작업 디렉토리다. 여기에서 prog.c라는 이름을 검색한다. cwd를 바꾸면 같은 상대 경로 문자열도 다른 파일을 가리킬 수 있다. 일반 open의 상대 경로를 설명하며 openat 계열은 지정한 디렉토리 FD를 기준으로 할 수 있다.

원문: [s25](https://pages.cs.wisc.edu/~bart/537/lecturenotes/s25.html), [RelativePathNameExample.JPG](https://pages.cs.wisc.edu/~bart/537/lecturenotes/figures/RelativePathNameExample.JPG), [path_resolution.7](https://man7.org/linux/man-pages/man7/path_resolution.7.html)

## 슬라이드 7 · 디렉토리·inode·데이터 블록

**06–10분 · 4분**

> 이 그림은 이름 검색과 내용 읽기를 연결합니다. 루트 inode에서 루트의 데이터 블록으로 가서 a를 찾습니다. a의 inode와 데이터에서 b를 찾고, b의 데이터에서 마지막 파일 이름을 찾습니다. 마지막 inode가 가리키는 데이터에는 실제 프로그램 내용이 있습니다. 원본 표의 c와 오른쪽 c.c는 표기가 다르므로 여기서는 c.c 엔트리로 읽겠습니다. 1부터 8까지는 캐시 등을 생략한 설명용 순서입니다.

**진행:** 첫 2분 동안 1–8을 따라간다. 다음 1분은 디렉토리 데이터의 이름·inode 표와 일반 파일 데이터의 소스 코드 내용을 비교한다. 마지막 1분에 inode의 속성을 stat으로 읽는 다음 부분에 연결한다.

**확인 질문:** 디렉토리 데이터와 일반 파일 데이터에 담긴 내용은 어떻게 다른가요?

**예상 답변:** 디렉토리는 이름과 inode 연결을 담고, 그림의 일반 파일은 프로그램 내용 바이트를 담는다. 실제 I/O 횟수는 캐시와 파일시스템 등에 따라 달라진다.

교수자 참고: open("/a/b/c.c", O_RDONLY)와 이어지는 read를 그림으로 추적한다. 1 루트 inode, 2 루트 디렉토리 데이터에서 a 검색, 3 a의 inode, 4 a의 데이터에서 b 검색, 5 b의 inode, 6 b의 데이터에서 c.c 검색, 7 일반 파일 inode, 8 실제 파일 데이터 읽기 순서다. 원본의 /a/b 디렉토리 표에는 c라고 적혀 있으나 본문과 오른쪽 라벨은 c.c이다. 이 표기 불일치를 밝히고 c.c 엔트리로 읽어 설명한다. 각 디렉토리가 한 블록에 들어가고 필요한 정보가 캐시되지 않았다는 단순화 모델이므로 실제 Linux에서 항상 8번 물리 디스크 I/O를 한다는 뜻은 아니다. 디렉토리 데이터는 이름과 inode 연결을, 일반 파일 데이터는 내용 바이트를 담는다는 차이를 강조한다. 그림은 원본 그대로 삽입했다.

원문: [s25](https://pages.cs.wisc.edu/~bart/537/lecturenotes/s25.html), [dir%2Binode.jpg](https://pages.cs.wisc.edu/~bart/537/lecturenotes/figures/dir%2Binode.jpg), [directory](https://docs.kernel.org/filesystems/ext4/directory.html), [path_resolution.7](https://man7.org/linux/man-pages/man7/path_resolution.7.html)

## 슬라이드 8 · 1부 파일 속성 조회

**10–11분 · 1분**

> 첫 부분은 조회 API입니다. 어떤 이름이나 FD를 넘겼는지, 심볼릭 링크를 따라가는지부터 확인합니다. 반환값 검사 후에 구조체 필드를 읽습니다.

**진행:** stat, lstat, fstat 이름을 칠판에 적고 인자가 경로인지 FD인지 표시한다.

**확인 질문:** fstat은 무엇을 인자로 받나요?

**예상 답변:** 열린 파일 디스크립터다.

## 슬라이드 9 · ls -l의 각 열과 파일 속성

**11–14분 · 3분**

> ls의 한 줄은 여러 파일 속성을 모아 보여 줍니다. 첫 글자는 종류이고 다음 아홉 글자는 권한입니다. 숫자 2는 하드 링크 수, 4는 여기서 일반 파일의 바이트 크기입니다. 화면의 이름을 inode 안에 저장된 필드라고 혼동하지 마세요.

**진행:** 예시 한 줄의 열을 차례로 가리킨다. ls -ln과 ls -ld .를 비교하고 숫자 ID 및 디렉토리 자체 조회를 확인한다.

**확인 질문:** 디렉토리의 크기 4096이 보이면 파일이 4096개라는 뜻일까요?

**예상 답변:** 아니다. 디렉토리 크기를 내부 파일 개수로 해석할 수 없다.

교수자 참고: 화면은 설명용 예시다. 계정 이름과 시각은 환경에 따라 달라진다. ls -n은 숫자 ID를 표시하고, ls -ld는 디렉토리 자체의 속성을 보여준다. 크기는 일반 파일을 기준으로 설명한다.

원문: [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html), [stat.2](https://man7.org/linux/man-pages/man2/stat.2.html)

## 슬라이드 10 · stat, lstat, fstat의 조회 대상

**14–18분 · 4분**

> stat은 링크 대상을, lstat은 마지막 요소인 링크 자체를 조회합니다. fstat은 이름 대신 열린 FD로 객체를 정합니다. 세 함수 모두 성공은 0, 실패는 -1입니다. lstat이 경로의 모든 링크를 무시하는 것은 아닙니다. 경로 중간의 링크는 여전히 해석합니다.

**진행:** 각 API에 일반 파일, 심볼릭 링크, 깨진 링크를 주는 경우를 학생이 예측하도록 한다. 결과는 실습 1에서 확인한다.

**확인 질문:** lstat("dir-link/file")은 중간 dir-link도 무시하나요?

**예상 답변:** 아니다. 마지막 요소가 링크일 때 링크 자체를 조회한다.

교수자 참고: 세 함수는 성공 0, 실패 -1을 반환한다. 실패하면 errno를 확인한다. lstat도 경로 중간의 심볼릭 링크는 해석한다. 파일 내용의 읽기 권한과 경로 디렉토리 검색 권한을 구분한다.

원문: [stat.2](https://man7.org/linux/man-pages/man2/stat.2.html)

## 슬라이드 11 · struct stat의 주요 필드

**18–22분 · 4분**

> 필드 이름을 그대로 외우기보다 역할별로 묶어 봅시다. dev와 ino는 파일을 식별하고, mode는 종류와 권한을, uid와 gid는 소유권을 나타냅니다. 크기 관련 필드는 뒤에서 단위를 따로 비교합니다. 시간은 접근·내용 수정·상태 변경을 구별합니다.

**진행:** REPORT에 다섯 범주를 적게 하고 각 필드를 배치한다. st_dev와 st_rdev의 차이는 장치 파일을 질문할 때만 보충한다.

**확인 질문:** st_ino만으로 서로 다른 파일시스템의 파일을 비교할 수 있나요?

**예상 답변:** inode 번호는 파일시스템마다 겹칠 수 있으므로 st_dev도 함께 비교한다.

교수자 참고: Linux의 POSIX 필드명을 사용한다. st_atime 등의 이름은 초 부분에 접근하는 호환 이름으로 볼 수 있다. st_dev는 장치 파일 자체가 나타내는 st_rdev와 다르다. 모든 필드가 하나의 원자적 시점 스냅샷이라는 보장은 없다.

원문: [stat.3type](https://man7.org/linux/man-pages/man3/stat.3type.html), [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html)

## 슬라이드 12 · file_attr: 조회 후 필드 출력

**22–25분 · 3분**

> file_attr는 기본적으로 lstat을 사용합니다. --follow를 주면 stat으로 바꿉니다. 실패한 뒤 구조체를 출력하지 않고 바로 오류로 끝납니다. inode와 크기의 자료형을 printf에 맞추려고 uintmax_t와 intmax_t로 변환한 부분도 확인합시다.

**진행:** file_attr.c의 인자 검사와 rc에 해당하는 반환값 검사 부분을 찾게 한다. 예제를 컴파일하고 단일 파일을 조회한다.

**확인 질문:** stat 실패 뒤에도 이전에 넣어 둔 st의 값을 사용해도 되나요?

**예상 답변:** 이번 조회 결과가 아니므로 사용하면 안 된다.

교수자 참고: 완성 소스는 인자 검사, 종류·권한 해석, 나노초 시각 출력도 포함한다. 실패한 경우에는 st를 읽지 않는다. --follow는 이 예제에서 정의한 옵션이다.

원문: [stat.2](https://man7.org/linux/man-pages/man2/stat.2.html), [stat.3type](https://man7.org/linux/man-pages/man3/stat.3type.html)

## 슬라이드 13 · 파일 종류와 권한 비트 분리

**25–28분 · 3분**

> st_mode에는 권한뿐 아니라 파일 종류도 들어 있습니다. 종류는 S_ISREG 같은 매크로로 판정합니다. 권한만 보려면 마스크를 씌웁니다. 0777은 기본 rwx, 07777은 특수 비트까지 포함합니다.

**진행:** mode 전체, mode & 0777, mode & 07777을 구별해 기록하게 한다. 실행 파일 확장자와 파일 종류 비트도 구분한다.

**확인 질문:** mode == 0644라는 비교가 보통 틀리는 이유는 무엇인가요?

**예상 답변:** mode에는 파일 종류 비트 등도 들어 있으므로 권한 부분만 추출해야 한다.

교수자 참고: st_mode 전체를 0644와 직접 비교하면 종류 비트 때문에 잘못된 판정이 된다. S_ISREG 같은 매크로를 사용한다. 완성 예제는 문자·블록 장치와 소켓도 구별한다.

원문: [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html)

## 슬라이드 14 · 링크와 inode의 관계

**28–33분 · 5분**

> 하드 링크는 같은 파일의 다른 이름입니다. 따라서 같은 inode와 내용을 공유하고 링크 수가 늘어납니다. 심볼릭 링크는 대상 경로를 담는 별도 객체입니다. 심볼릭 링크를 만들었다고 대상 파일의 하드 링크 수가 늘어나는 것은 아닙니다.

**진행:** 칠판에 이름 두 개와 한 inode, 별도의 symlink 객체를 구별해 적는다. note.txt의 문자열 길이와 실제 파일 내용 길이를 각각 세게 한다.

**확인 질문:** soft.txt가 문자열 note.txt를 가리키고 대상 내용이 ABC와 개행이면 lstat 크기는 얼마일까요?

**예상 답변:** 저장된 경로 문자열 길이인 8바이트다. stat으로 대상을 보면 내용 크기 4바이트다.

교수자 참고: 하드 링크를 추가하면 같은 파일의 st_nlink가 증가한다. 심볼릭 링크 생성은 대상의 하드 링크 수를 늘리지 않는다. inode 번호는 파일이 제거된 뒤 재사용될 수 있으므로 영구 식별자로 저장하지 않는다.

원문: [link.2](https://man7.org/linux/man-pages/man2/link.2.html), [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html), [stat.2](https://man7.org/linux/man-pages/man2/stat.2.html)

## 슬라이드 15 · 실습 1: 링크의 속성 비교

**33–37분 · 4분**

> 같은 임시 폴더 안에서 원본, 하드 링크, 심볼릭 링크를 만들겠습니다. 두 개의 숫자를 따로 적지 말고 무엇이 같은지와 다른지를 표시해 보세요. 깨진 링크에서는 조회 함수가 어느 대상을 찾으려 했는지가 중요합니다.

**진행:** 슬라이드 명령을 같은 터미널에서 실행한다. 2분 동안 네 결과를 기록하고, 1분 동안 깨진 링크를 두 방식으로 조회한다. 남은 시간에 링크 수를 설명한다.

**확인 질문:** --follow로 깨진 링크 조회가 실패하면 lstat 조회도 반드시 실패하나요?

**예상 답변:** 아니다. 링크 객체 자체는 여전히 있으므로 기본 lstat 조회는 가능하다.

```sh
RUN=$(mktemp -d)
printf 'ABC\n' > "$RUN/note.txt"
ln "$RUN/note.txt" "$RUN/hard.txt"
ln -s note.txt "$RUN/soft.txt"

./file_attr "$RUN/note.txt"
./file_attr "$RUN/hard.txt"
./file_attr "$RUN/soft.txt"
./file_attr --follow "$RUN/soft.txt"
ln -s missing "$RUN/broken.txt"
./file_attr --follow "$RUN/broken.txt"
```

교수자 참고: soft.txt의 lstat 크기는 경로 문자열 note.txt의 길이인 8바이트다. 대상 내용은 ABC와 개행으로 4바이트다. 마지막 실패 뒤 기본 조회 ./file_attr "$RUN/broken.txt"도 실행한다. RUN은 이후 실습에서도 유지한다.

원문: [stat.2](https://man7.org/linux/man-pages/man2/stat.2.html), [link.2](https://man7.org/linux/man-pages/man2/link.2.html)

## 슬라이드 16 · 조회 실패와 조회 이후의 변화

**37–40분 · 3분**

> 조회 실패의 원인은 경로가 없거나, 중간 요소가 디렉토리가 아니거나, 경로 검색 권한이 없는 경우 등입니다. 한 번 성공한 조회가 다음 open의 성공을 보장하지는 않습니다. 실제로 하려는 호출의 반환값을 검사해야 합니다.

**진행:** 없는 경로와 일반 파일 뒤에 /child를 붙인 경로를 비교한다. access 후 open 사이에 이름이 바뀔 수 있는 상황을 설명하고 휴식으로 연결한다.

**확인 질문:** stat이 성공하면 그 파일의 내용을 반드시 읽을 수 있나요?

**예상 답변:** 아니다. 내용 읽기 권한과 경로 검색 권한은 다르며 그 사이 상태도 바뀔 수 있다.

교수자 참고: stat 전 access를 호출해도 뒤의 open 성공은 보장하지 않는다. 열린 객체를 다룰 때는 fstat으로 그 FD의 속성을 조회한다. fstat은 파일 내용이나 속성을 고정하거나 잠그지는 않는다.

원문: [stat.2](https://man7.org/linux/man-pages/man2/stat.2.html), [path_resolution.7](https://man7.org/linux/man-pages/man7/path_resolution.7.html), [access.2](https://man7.org/linux/man-pages/man2/access.2.html)

## 휴식 · 40–50분

같은 터미널을 유지하고 휴식 후 권한 실습으로 이어갑니다.

## 슬라이드 17 · 2부 권한과 소유권

**50–51분 · 1분**

> 이제 속성을 바꾸겠습니다. 먼저 rwx 비트를 읽고, 생성 시 적용되는 umask와 이미 있는 파일의 권한을 바꾸는 chmod를 구분합니다.

**진행:** 앞 실습에서 만든 RUN 변수가 같은 터미널에 남아 있는지 확인한다.

**확인 질문:** umask를 바꾸면 기존 note.txt의 권한도 바뀔까요?

**예상 답변:** 아니다. 새 파일을 만들 때 영향을 준다.

## 슬라이드 18 · 기본 권한과 8진수 표현

**51–55분 · 4분**

> 권한은 세 사용자 범주에 각각 rwx를 부여합니다. 4, 2, 1을 더해 한 자리를 만듭니다. C에서는 0640처럼 앞에 0을 붙여 8진수를 표현합니다. 숫자 모양이 비슷한 640은 전혀 다른 상수입니다.

**진행:** 0640, 0755, 0600을 학생이 rwx 문자열로 바꾼다. 2분 동안 풀이하고 각 범주의 의미를 말하게 한다.

**확인 질문:** 소유자 권한이 없지만 기타 사용자에게 r이 있으면 소유자가 기타 권한을 대신 쓰나요?

**예상 답변:** 기본 권한 판정은 해당 범주를 적용하며 더 허용적인 다른 범주로 자동 대체하지 않는다.

교수자 참고: 프로세스가 파일 소유자와 일치하면 소유자 권한 비트를 적용한다. 소유자 권한이 없다고 기타 사용자 권한으로 다시 시도하지 않는다. 보조 그룹, ACL과 capability 등도 실제 접근 판단에 영향을 준다.

원문: [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html), [chmod.2](https://man7.org/linux/man-pages/man2/chmod.2.html)

## 슬라이드 19 · 권한 비트로 rwx 문자열 만들기

**55–59분 · 4분**

> 아홉 개 비트를 하나씩 검사하면 권한 문자열을 만들 수 있습니다. 비트가 켜져 있으면 해당 r, w, x를, 꺼져 있으면 하이픈을 넣습니다. 문자열 끝의 널 문자를 잊지 않아야 합니다. 특수 권한 비트는 완성 코드에서 실행 문자 자리를 s나 t 등으로 바꿉니다.

**진행:** 반복문의 i=0,1,2를 직접 추적한다. 0640을 입력으로 손으로 아홉 문자를 적고 프로그램 결과와 비교한다.

**확인 질문:** 출력 문자가 9개인데 배열은 왜 10칸인가요?

**예상 답변:** C 문자열의 끝을 표시할 널 문자 자리가 필요하다.

교수자 참고: 발췌는 기본 rwx 부분이다. 완성 file_attr는 set-user-ID·set-group-ID·sticky bit를 s/S/t/T로 표시한다. 예: set-user-ID가 있고 실행 비트가 없으면 S, 있으면 s다.

원문: [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html)

## 슬라이드 20 · 일반 파일과 디렉토리의 권한

**59–63분 · 4분**

> 디렉토리의 x는 파일 실행이라는 뜻과 다릅니다. 알고 있는 이름을 검색하고 경로를 통과하는 권한입니다. r은 이름 목록을 읽는 권한입니다. 이름을 만들거나 지우는 것은 디렉토리의 변경이므로 보통 w와 x가 함께 필요합니다.

**진행:** 일반 파일 내용 쓰기와 부모 디렉토리 엔트리 변경을 두 줄로 나누어 적는다. 읽기 전용 파일도 부모 권한에 따라 이름을 지울 수 있음을 설명한다.

**확인 질문:** 파일에 w 권한이 없으면 그 이름을 절대로 지울 수 없나요?

**예상 답변:** 아니다. 이름 삭제는 부모 디렉토리 권한과 추가 조건에 영향을 받는다.

교수자 참고: 파일 내용 쓰기와 파일 이름 삭제는 다른 동작이다. sticky bit, ACL, capability, 읽기 전용 파일시스템 등 추가 조건이 있을 수 있다. 실습은 root가 아닌 일반 사용자와 자신이 만든 임시 디렉토리에서 진행한다.

원문: [path_resolution.7](https://man7.org/linux/man-pages/man7/path_resolution.7.html), [unlink.2](https://man7.org/linux/man-pages/man2/unlink.2.html)

## 슬라이드 21 · chmod와 fchmod

**63–67분 · 4분**

> chmod는 경로, fchmod는 열린 FD를 대상으로 권한을 설정합니다. 지정한 숫자에 해당하는 권한으로 바꾸므로 비트를 자동 추가하는 호출이라고 생각하면 안 됩니다. 파일 내용이 그대로여도 inode 상태가 바뀌어 ctime이 갱신됩니다.

**진행:** chmod(path, 0640)과 fchmod(fd, 0600)을 비교한다. 완성 mode_demo에서 fchmod 후 fstat으로 다시 확인하는 부분을 찾는다.

**확인 질문:** chmod에 0640을 넘길 때 umask 0077이 다시 적용되나요?

**예상 답변:** 아니다. umask는 생성 권한에 적용되며 chmod는 기존 권한을 설정한다.

교수자 참고: chmod는 마지막 심볼릭 링크를 따라간다. Linux의 일반 심볼릭 링크 권한을 링크 자체의 접근 제어로 해석하지 않는다. umask는 chmod가 지정한 권한에 다시 적용되지 않는다.

원문: [chmod.2](https://man7.org/linux/man-pages/man2/chmod.2.html), [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html)

## 슬라이드 22 · umask와 생성 권한

**67–71분 · 4분**

> umask는 요청한 생성 권한에서 특정 비트를 제거합니다. 0666과 0022이면 0644, 0666과 0077이면 0600입니다. 뺄셈처럼 보일 때가 있어도 실제로는 비트 마스크 연산입니다. 원래 요청에 없는 실행 권한을 더하지 않습니다.

**진행:** 0666 & ~0027을 개인별로 계산하게 하고 0640을 확인한다. 0600과 mask 0002로 단순 뺄셈이 틀리는 사례도 비교한다.

**확인 질문:** 0666에서 0027을 단순히 빼서 계산해도 되나요?

**예상 답변:** 항상 맞지 않는다. 비트별 제거인 AND 연산으로 계산한다.

교수자 참고: 위 블록은 계산 설명이며 완성 C 프로그램이 아니다. Linux에서 부모 디렉토리에 기본 ACL이 있으면 생성 권한 결정에 그 ACL이 사용되므로 단순 공식을 그대로 적용할 수 없다. 이 실습은 기본 ACL 없는 환경을 가정한다.

원문: [umask.2](https://man7.org/linux/man-pages/man2/umask.2.html), [open.2](https://man7.org/linux/man-pages/man2/open.2.html)

## 슬라이드 23 · 생성 후 fstat으로 권한 확인

**71–74분 · 3분**

> umask 함수는 이전 값을 돌려줍니다. 임시로 바꾸었다면 성공과 실패 모두에서 복원합니다. open 실패의 errno도 복원 전에 보관해 둡니다. O_EXCL은 이미 있는 이름을 덮어쓰지 않고 실패하게 합니다. 실제 생성 권한은 fstat으로 확인합니다.

**진행:** old, saved, fd 변수의 역할을 각각 찾는다. open 실패 경로에서도 mask를 복원하는지 학생이 추적한다.

**확인 질문:** 이 프로그램이 umask를 바꾸면 부모 셸의 umask도 바뀌나요?

**예상 답변:** 자식 프로세스의 변경이므로 부모 셸에는 반영되지 않는다.

교수자 참고: 발췌다. 완성 mode_demo는 close 실패도 검사한다. umask는 프로세스 범위이므로 다중 스레드에서는 임시 변경에도 주의가 필요하다. 자식 프로그램의 umask 변경은 부모 셸의 mask를 바꾸지 않는다.

원문: [umask.2](https://man7.org/linux/man-pages/man2/umask.2.html), [open.2](https://man7.org/linux/man-pages/man2/open.2.html), [stat.2](https://man7.org/linux/man-pages/man2/stat.2.html)

## 슬라이드 24 · 소유 사용자와 그룹 변경

**74–76분 · 2분**

> 소유권은 UID와 GID로 저장합니다. chown은 이 값을 변경하지만 일반 사용자가 임의 사용자에게 소유권을 넘길 수 있다고 가정하면 안 됩니다. 그룹 변경도 제한됩니다. 오늘은 기본 조회를 하고 권한 조건을 이해하는 데 집중합니다.

**진행:** chown(path, (uid_t)-1, gid)의 의미를 읽고 유지하는 항목을 표시한다. 특수 권한 변경은 시연하지 않는다.

**확인 질문:** uid 인자 -1은 소유자를 UID -1로 바꾸라는 의미인가요?

**예상 답변:** 해당 소유 사용자 값을 유지하라는 특별한 인자다.

교수자 참고: Linux에서 파일 소유자는 자신이 속한 그룹으로 파일 그룹을 바꿀 수 있다. chown은 심볼릭 링크 대상을, lchown은 링크 자체를 변경한다. 소유권 변경이 set-ID 비트를 지울 수 있다. 수업에서는 소유권을 조회하고, 임의 계정으로의 변경을 요구하지 않는다.

원문: [chown.2](https://man7.org/linux/man-pages/man2/chown.2.html)

## 슬라이드 25 · 실습 2: 생성 권한과 chmod

**76–80분 · 4분**

> mode_demo는 생성 직후 권한을 먼저 출력하고 첫 파일을 fchmod로 0640으로 바꿉니다. 따라서 마지막 file_attr 결과는 생성 직후 0644와 다를 수 있습니다. 두 출력 시점을 구분해 기록하세요. 하드 링크에서는 원본을 통해 바꾼 권한이 다른 이름에서도 보입니다.

**진행:** 2분간 mode_demo와 세 경로를 조회한다. 1분간 원본 권한 변경 후 hard.txt를 조회한다. 1분간 두 종류의 변경을 설명한다.

**확인 질문:** mode_demo 후 mask022.txt를 조회했는데 0640이면 umask 계산이 틀린 건가요?

**예상 답변:** 생성 직후는 0644이고 이후 fchmod가 0640으로 바꾼 것이다.

```sh
mkdir "$RUN/modes"
./mode_demo "$RUN/modes"
./file_attr "$RUN/modes/mask022.txt"
./file_attr "$RUN/modes/mask077.txt"
./file_attr "$RUN/modes/groupdir"

chmod 600 "$RUN/note.txt"
./file_attr "$RUN/note.txt"
./file_attr "$RUN/hard.txt"
```

교수자 참고: mode_demo 출력에는 첫 파일의 생성 직후 0644와 fchmod 후 0640이 각각 나온다. 마지막 file_attr 조회는 0640이어야 한다. groupdir는 0777 요청과 mask 0027로 0750이 예상된다. 두 번째 실행은 기존 이름 때문에 실패하는 것이 정상이다.

원문: [umask.2](https://man7.org/linux/man-pages/man2/umask.2.html), [chmod.2](https://man7.org/linux/man-pages/man2/chmod.2.html), [link.2](https://man7.org/linux/man-pages/man2/link.2.html)

## 슬라이드 26 · 3부 파일 크기와 시간 속성

**80–81분 · 1분**

> 파일 크기에도 서로 다른 값이 있습니다. 프로그램이 보는 논리적 길이와 파일시스템의 실제 할당량을 구분하겠습니다. 시간 역시 하나의 날짜로 묶지 않고 세 종류로 나누어 보겠습니다.

**진행:** file_attr 출력에서 size, blocks, blksize와 세 시각 줄을 가리킨다.

**확인 질문:** size와 blocks는 단위가 같나요?

**예상 답변:** size는 바이트, Linux st_blocks는 512바이트 단위다.

## 슬라이드 27 · st_size, st_blocks, st_blksize

**81–85분 · 4분**

> 일반 파일의 st_size는 마지막 바이트 위치까지의 논리적 길이입니다. st_blocks는 할당 공간에 대한 정보이고 Linux에서는 512바이트 단위입니다. st_blksize는 효율적인 입출력을 위한 권장 크기입니다. 이름이 비슷해도 st_blocks에 st_blksize를 곱하면 안 됩니다.

**진행:** 세 값의 의미와 단위를 REPORT에 적게 한다. 희소 파일에서 앞의 빈 구간을 모두 디스크에 저장할 필요가 있는지 예측한다.

**확인 질문:** st_blocks가 8, st_blksize가 4096이면 할당량은 32768바이트인가요?

**예상 답변:** Linux st_blocks 단위를 적용하면 8×512이다. st_blksize를 곱하지 않는다.

교수자 참고: st_blksize를 st_blocks의 단위로 곱하지 않는다. st_blocks × 512와 st_size는 서로 다른 질문에 답한다. 디렉토리 크기는 내부 파일 크기의 합이나 파일 개수가 아니다. 압축, 희소 파일, 파일시스템 정책 등에 따라 할당량이 달라진다.

원문: [stat.3type](https://man7.org/linux/man-pages/man3/stat.3type.html), [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html)

## 슬라이드 28 · 실습 3: 희소 파일의 크기

**85–90분 · 5분**

> 파일의 1MiB 위치로 이동해서 한 바이트를 씁니다. 크기는 1MiB가 아니라 1MiB 더하기 1입니다. 중간 구간을 읽으면 0 값이 나옵니다. 파일시스템이 희소 파일을 지원하면 실제 할당 공간은 논리적 길이보다 작을 수 있습니다.

**진행:** 1분간 size를 예측하고 2분간 프로그램과 stat·du를 실행한다. 1분간 파일 앞부분과 마지막 바이트를 확인하고 1분간 할당량의 환경 차이를 설명한다.

**확인 질문:** lseek만 실행하고 write를 하지 않아도 파일 크기가 늘어날까요?

**예상 답변:** lseek는 위치만 옮기므로 뒤에서 실제 쓰기를 해야 크기가 늘어난다.

```sh
./sparse_demo "$RUN/sparse.bin"
./file_attr "$RUN/sparse.bin"

# Ubuntu GNU coreutils 명령
stat -c 'size=%s blocks=%b unit=%B' \
  "$RUN/sparse.bin"
du -B1 "$RUN/sparse.bin"

# 코드 핵심: 1 MiB 위치에서 한 바이트 쓰기
# lseek(fd, 1024 * 1024, SEEK_SET);
# write(fd, "X", 1);
```

교수자 참고: 완성 sparse_demo는 open, lseek, write, fstat, close 반환값을 검사한다. lseek만으로는 파일 크기가 늘지 않으며 뒤에서 실제 쓰기를 해야 한다. 할당량이 특정 숫자나 반드시 작은 값이어야 한다고 채점하지 않는다.

원문: [lseek.2](https://man7.org/linux/man-pages/man2/lseek.2.html), [stat.3type](https://man7.org/linux/man-pages/man3/stat.3type.html)

## 슬라이드 29 · atime, mtime, ctime

**90–94분 · 4분**

> atime은 접근, mtime은 내용 수정, ctime은 inode 상태 변경과 관련됩니다. ctime의 c를 creation으로 외우면 잘못 해석하게 됩니다. 읽을 때마다 atime이 반드시 증가하는 것도 아닙니다. 마운트 정책은 갱신 횟수를 줄일 수 있습니다.

**진행:** 읽기, 쓰기, chmod를 세 시각과 연결하게 한다. ctime과 생성 시각을 구분해 학생이 한 문장으로 설명하게 한다.

**확인 질문:** 파일을 chmod만 했는데 ctime이 바뀌면 새로 생성된 파일인가요?

**예상 답변:** 아니다. 같은 파일의 상태가 변경된 것이다.

교수자 참고: ctime은 생성 시각이 아니다. 내용 쓰기는 일반적으로 mtime과 ctime에 영향을 준다. atime은 noatime·relatime 등의 조건 때문에 읽을 때마다 증가한다고 가정하지 않는다. 파일시스템마다 시각 저장 해상도도 다르다.

원문: [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html), [utimensat.2](https://man7.org/linux/man-pages/man2/utimensat.2.html)

## 슬라이드 30 · 작업과 시간 속성의 변화

**94–98분 · 4분**

> 내용 쓰기는 일반적으로 mtime과 ctime을 갱신합니다. 권한만 바꾸면 mtime은 유지되고 ctime이 갱신됩니다. futimens로 과거 atime과 mtime을 지정할 수 있어도 ctime을 같은 방식으로 임의 지정하지는 않습니다. 빠른 두 작업이 같은 숫자로 보일 수 있다는 점도 기억하세요.

**진행:** 변경 전후에 유지될 필드를 학생이 먼저 적는다. 초 단위 표시와 파일시스템 시각 해상도의 한계를 설명한다.

**확인 질문:** 연속 chmod 후 ctime 숫자가 같으면 chmod가 실패한 걸까요?

**예상 답변:** 반환값과 권한을 확인해야 한다. 저장·표시 해상도 때문에 시각 값이 같을 수 있다.

교수자 참고: 짧은 간격의 두 작업은 시각 해상도 때문에 같은 숫자로 보일 수 있다. 표시값이 같다고 작업이 일어나지 않았다고 판단하지 않는다. 이번 실습은 과거의 고정 atime·mtime을 지정한 뒤 권한을 바꾸어 의미를 구분한다.

원문: [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html), [chmod.2](https://man7.org/linux/man-pages/man2/chmod.2.html), [utimensat.2](https://man7.org/linux/man-pages/man2/utimensat.2.html)

## 슬라이드 31 · futimens로 atime과 mtime 지정

**98–101분 · 3분**

> timespec 배열의 첫 항목은 atime, 두 번째는 mtime입니다. 원하는 초와 나노초를 넣거나 UTIME_NOW, UTIME_OMIT을 사용할 수 있습니다. 실습에서는 자신이 만든 파일의 두 시각을 과거로 지정하겠습니다. ctime은 배열의 세 번째 항목이 아닙니다.

**진행:** ts[0]과 ts[1]이 어떤 시각인지 표시한다. mtime만 바꾸려면 atime 쪽을 어떻게 설정할지 묻는다.

**확인 질문:** atime을 그대로 두고 mtime만 바꾸려면 어떻게 하나요?

**예상 답변:** atime 항목의 tv_nsec에 UTIME_OMIT을 사용한다.

교수자 참고: 지금 시각으로 두 값을 갱신하는 경우와 임의 값을 지정하는 경우의 권한 조건은 다르다. 실습은 자신이 새로 만든 파일에 수행한다. utimensat에 AT_SYMLINK_NOFOLLOW를 주면 마지막 링크 자체를 대상으로 삼을 수 있다.

원문: [utimensat.2](https://man7.org/linux/man-pages/man2/utimensat.2.html)

## 슬라이드 32 · 실습 4: 시간 속성의 의미

**101–105분 · 4분**

> time_attrs는 두 단계로 출력합니다. 첫 단계에서 atime과 mtime을 고정하고, 두 번째 단계에서는 권한만 바꿉니다. 마지막 두 시각이 그대로인지 확인하세요. ctime은 과거 파일 생성 시각을 복원한 값이 아니라 현재 일어난 상태 변경에 관련됩니다.

**진행:** 2분간 프로그램과 file_attr를 실행하고, 1분간 같은 필드와 다른 필드를 표시한다. 1분간 ctime이 같게 보이는 경우도 설명한다.

**확인 질문:** 파일 내용을 읽지 않고 fstat만 했는데 atime이 바뀌어야 하나요?

**예상 답변:** 파일 데이터 읽기를 하지 않으므로 그 이유로 atime 갱신을 기대하지 않는다.

```sh
./time_attrs "$RUN/times.txt"
./file_attr "$RUN/times.txt"

# Ubuntu에서 이름과 시간을 함께 확인
stat -c 'atime=%X mtime=%Y ctime=%Z' \
  "$RUN/times.txt"

# 프로그램의 두 출력 단계
# after_futimens: atime과 mtime을 과거로 설정
# after_fchmod:   권한만 0640으로 변경
```

교수자 참고: 연속 출력의 ctime이 같은 숫자여도 틀린 결과가 아니다. time_attrs는 content를 읽지 않으며 futimens와 fchmod 후 fstat만 수행한다. 시각의 의미와 변하지 않아야 할 값이 핵심이다.

원문: [utimensat.2](https://man7.org/linux/man-pages/man2/utimensat.2.html), [chmod.2](https://man7.org/linux/man-pages/man2/chmod.2.html), [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html)

## 슬라이드 33 · 4부 이름과 열린 파일의 속성

**105–106분 · 1분**

> 마지막은 이름과 열린 파일의 관계입니다. 이름이 사라졌다는 사실과 열린 객체를 더는 사용할 수 없다는 사실을 구분하겠습니다.

**진행:** fd_unlink_demo의 mkstemp, unlink, fstat을 찾는다.

**확인 질문:** unlink 다음에 fstat을 시도하면 무조건 실패하나요?

**예상 답변:** 이미 열린 FD가 유효하면 파일 속성 조회가 가능하다.

## 슬라이드 34 · unlink 이후의 fstat

**106–110분 · 4분**

> unlink는 이름 하나를 제거합니다. 하드 링크가 하나뿐이면 링크 수가 0이 되지만 열린 FD가 객체를 계속 참조할 수 있습니다. 그래서 경로 조회는 실패하면서 fstat과 read는 성공하는 결과가 가능합니다. 이름의 수와 열린 참조의 수는 같은 값이 아닙니다.

**진행:** before·after 결과의 inode, size, links를 학생이 예측한다. 파일을 두 이름으로 열어 두었을 때와 하드 링크 두 개의 차이는 질문이 있을 때 보충한다.

**확인 질문:** st_nlink는 그 파일을 열어 둔 FD의 개수인가요?

**예상 답변:** 아니다. 하드 링크, 즉 파일을 가리키는 이름의 수다.

교수자 참고: 실습은 파일을 만든 뒤 다른 하드 링크를 추가하지 않는다. unlink 이후에는 경로 조회가 ENOENT로 실패하지만 열린 FD를 통한 fstat과 읽기는 가능하다. 최종 공간 회수에는 열린 참조 등 파일 수명 조건이 모두 해소되어야 한다.

원문: [unlink.2](https://man7.org/linux/man-pages/man2/unlink.2.html), [stat.2](https://man7.org/linux/man-pages/man2/stat.2.html)

## 슬라이드 35 · 실습 5: 이름 삭제 후 속성 조회

**110–115분 · 5분**

> 이 프로그램은 자기 임시 파일만 만들어 지우므로 인자를 받을 필요가 없습니다. before와 after에서 inode와 크기를 비교하고 링크 수가 1에서 0으로 바뀌는지 확인합니다. 마지막 read에서 hello가 나온 이유를 이름과 FD의 관계로 설명해 보세요.

**진행:** 1분간 예측, 2분간 실행과 로그 기록, 2분간 짝 설명을 진행한다. 프로그램이 출력한 임시 경로가 종료 후 남지 않는지도 확인한다.

**확인 질문:** 이름이 없는데 왜 read=hello가 나오나요?

**예상 답변:** 열린 FD가 같은 파일을 참조하고 있어 내용을 읽을 수 있다.

```sh
./fd_unlink_demo

# 프로그램 내부의 실행 순서
# mkstemp: 전용 임시 파일 생성
# write:   "hello" 5바이트 기록
# fstat:   inode, links=1, size=5 출력
# unlink:  방금 만든 이름 제거
# fstat:   같은 inode, links=0, size=5
# read:    열린 FD로 "hello" 읽기
# close:   열린 참조 정리
```

교수자 참고: fd_unlink_demo는 명령 인자를 받지 않고 mkstemp로 자기 임시 파일만 생성·삭제한다. 정상 실행 뒤 파일이 남지 않는지도 자동 검사한다. Linux 로컬 파일시스템을 수업 기준으로 하며 NFS 등 특수 환경은 별도 해석이 필요하다.

원문: [unlink.2](https://man7.org/linux/man-pages/man2/unlink.2.html), [stat.2](https://man7.org/linux/man-pages/man2/stat.2.html), [open.2](https://man7.org/linux/man-pages/man2/open.2.html)

## 슬라이드 36 · 확인 퀴즈

**115–118분 · 3분**

> 다섯 확인 문제로 오늘 내용을 정리하겠습니다. 값만 답하지 말고 어떤 대상을 조회했고 어떤 비트를 보았는지 함께 설명해 주세요.

**진행:** 1분간 개인 답안을 적고 2분간 정답의 근거를 확인한다.

**확인 질문:** 정답의 핵심을 한 문장씩 말해 보세요.

**예상 답변:** 링크 자체와 대상 구별, 종류·권한 비트 분리, 생성 권한 0640, ctime은 상태 변경, 열린 FD가 수명 유지다.

교수자 참고: 정답: 1. stat은 없는 대상을 따라가 실패하고 lstat은 링크 자체를 조회한다. 2. 종류와 특수 비트도 포함되므로 목적에 맞는 마스크가 필요하다. 3. 0640. 4. 아니다. 상태 변경 시각이다. 5. 열린 FD가 참조를 유지한다.

## 슬라이드 37 · 실습 정리와 제출

**118–119분 · 1분**

> 제출할 것은 소스와 실행 로그, 기록지입니다. inode 숫자나 할당 블록 수를 옆 사람과 같게 만들 필요는 없습니다. 작업 전후의 관계를 설명하는 것이 중요합니다. 선택 과제는 작은 ls에 오늘 배운 속성 열을 붙이는 것입니다.

**진행:** REPORT 양식과 제출 파일을 보여 준다. 마감과 실제 배점은 수업 운영에 맞게 별도로 안내한다.

**확인 질문:** 희소 파일의 blocks 값이 예시와 다르면 무조건 오답인가요?

**예상 답변:** 파일시스템에 따라 달라질 수 있으므로 단위와 크기의 의미를 정확히 설명했는지 본다.

교수자 참고: 선택 과제는 이전 mini_ls에 rwx 문자열과 UID/GID, 크기, mtime을 출력하는 기능을 추가하는 것이다. 재귀 탐색이나 시스템 정보 조회로 범위를 넓히지 않고 파일 속성 해석에 집중한다.

## 슬라이드 38 · 참고 자료

**119–120분 · 1분**

> 새 API를 읽을 때는 함수 이름과 예제만 보지 말고, 대상과 반환값, 권한 조건을 함께 확인하세요. 관련 원문은 교안과 발표자 노트에 연결했습니다.

**진행:** man 2 stat, man 7 inode, man 2 chmod처럼 절 번호를 포함한 조회를 안내한다.

**확인 질문:** 같은 이름의 명령과 C 함수는 어떻게 구분해 문서를 보나요?

**예상 답변:** 매뉴얼 절 번호와 함수 원형, 헤더를 함께 확인한다.

교수자 참고: 각 슬라이드 발표자 노트와 교안 원고에 관련 원문 링크가 있다. 수업은 Ubuntu 일반 사용자와 자신이 만든 임시 파일을 기준으로 한다. 반환값, 자료형, 적용 대상, 실패 조건을 함께 확인한다. 디렉토리 관련 원본 그림 4개는 Barton P. Miller의 CS 537 Section 25: Directories에서 가져왔다. 원본 URL과 이용 안내는 assets/cs537/README.md에 기록했다.

원문: [stat.2](https://man7.org/linux/man-pages/man2/stat.2.html), [stat.3type](https://man7.org/linux/man-pages/man3/stat.3type.html), [inode.7](https://man7.org/linux/man-pages/man7/inode.7.html), [chmod.2](https://man7.org/linux/man-pages/man2/chmod.2.html), [chown.2](https://man7.org/linux/man-pages/man2/chown.2.html), [umask.2](https://man7.org/linux/man-pages/man2/umask.2.html), [utimensat.2](https://man7.org/linux/man-pages/man2/utimensat.2.html), [link.2](https://man7.org/linux/man-pages/man2/link.2.html), [unlink.2](https://man7.org/linux/man-pages/man2/unlink.2.html), [open.2](https://man7.org/linux/man-pages/man2/open.2.html), [lseek.2](https://man7.org/linux/man-pages/man2/lseek.2.html), [path_resolution.7](https://man7.org/linux/man-pages/man7/path_resolution.7.html), [access.2](https://man7.org/linux/man-pages/man2/access.2.html), [s25](https://pages.cs.wisc.edu/~bart/537/lecturenotes/s25.html)
