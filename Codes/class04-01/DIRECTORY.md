# 디렉토리 C 예제

Week04-class01의 디렉토리 부분을 함수별로 실행하는 실습이다. 각 프로그램은 `main`에서 인자 검사 → API 호출 → 반환값 검사 순서로 읽을 수 있다.

| 소스 | 실행 | 핵심 API | 연결 슬라이드 |
|---|---|---|---|
| [dir_create.c](dir_create.c) | `./dir_create DIRECTORY` | `mkdir` | 27 |
| [dir_list.c](dir_list.c) | `./dir_list DIRECTORY` | `opendir`, `readdir`, `closedir` | 23–25 |
| [dir_chdir.c](dir_chdir.c) | `./dir_chdir DIRECTORY` | `getcwd`, `chdir` | 28 |
| [dir_remove.c](dir_remove.c) | `./dir_remove DIRECTORY` | `rmdir` | 27 |

공통 종료 상태는 성공 `0`, 실행 중 오류 `1`, 인자 개수 오류 `2`이다. C 함수의 실패 반환값 `-1`과 프로그램 종료 상태 `1`은 구분한다.

## 빌드

이 폴더에서 실행한다. C 컴파일러와 make가 필요하다.

```sh
make
```

기존 `mystat`, `mini_ls`도 함께 빌드한다. 디렉토리 예제만 빌드하려면 다음처럼 지정한다.

```sh
make dir_create dir_list dir_chdir dir_remove
```

## 실습 1 · 생성하고 다시 생성하기

새 임시 폴더에서 시작한다. 이후 실습도 같은 터미널에서 이어서 실행한다. 이전 교안의 `RUN`과 구별하기 위해 `DIR_LAB`을 사용한다.

```sh
DIR_LAB=$(mktemp -d)
printf '실습 폴더: %s\n' "$DIR_LAB"
./dir_create "$DIR_LAB/demo"
ls -ld "$DIR_LAB/demo"
./dir_create "$DIR_LAB/demo"
echo $?
```

첫 생성은 `created: .../demo`를 출력한다. 같은 이름으로 다시 만들면 오류와 종료 상태 `1`을 보인다. 기존 디렉토리는 그대로 남는다.

**설명:** `mkdir(path, 0755)`의 `0755`는 요청 권한이다. 기본 ACL이 없는 환경에서는 요청 권한에서 umask 비트를 제거한 권한으로 생성된다. 부모 디렉토리를 자동으로 만들지는 않는다. [mkdir(2)](https://man7.org/linux/man-pages/man2/mkdir.2.html).

**질문:** `./dir_create "$DIR_LAB/missing/child"`는 성공할까? → `missing`이 없으므로 실패한다.

## 실습 2 · 이름 목록 읽기

```sh
./dir_list "$DIR_LAB/demo"
touch "$DIR_LAB/demo/.hidden"
printf 'hello\n' > "$DIR_LAB/demo/space name.txt"
./dir_create "$DIR_LAB/demo/sub"
./dir_list "$DIR_LAB/demo"
```

첫 목록에는 `.`과 `..`가 나온다. 두 번째 목록에는 다음 이름들이 나온다. 순서는 달라질 수 있다.

```text
.
..
.hidden
space name.txt
sub
```

**코드 읽기:** `readdir` 직전에 `errno=0`으로 초기화하고, NULL 반환 시 `errno`를 검사한다. `d_name`은 전체 경로가 아닌 이름이다. 반환된 포인터는 직접 `free`하지 않는다. 마지막에 `closedir`를 호출한다. [readdir(3)](https://man7.org/linux/man-pages/man3/readdir.3.html).

**기존 mini_ls와 비교:** `dir_list`는 이름만 출력하며 `.`과 `..`도 보여 준다. `mini_ls`는 이 두 이름을 생략하고 각 항목의 종류·inode·크기도 조회한다. 따라서 빈 폴더의 `dir_list` 출력 두 줄과 `mini_ls`의 출력 없음은 모두 정상이다. 두 예제 모두 재귀 탐색이나 이름 정렬을 하지 않는다. 개행이 포함된 파일 이름을 이스케이프하는 기능은 이 기초 예제에 포함하지 않는다.

## 실습 3 · 작업 디렉토리 변경

```sh
pwd
./dir_chdir "$DIR_LAB/demo"
pwd
```

프로그램은 `before: ...`와 `after: .../demo`를 출력한다. 프로그램 실행 앞뒤에 입력한 셸의 `pwd` 출력은 같다.

**설명:** `chdir`는 실행 중인 프로그램의 작업 디렉토리를 바꾼다. 부모 셸의 위치까지 바꾸지 않는다. `getcwd`는 변경 전후의 절대 경로를 보여 준다. 이 예제의 4096바이트 버퍼는 수업용 고정 크기이며 모든 경로 길이를 보장하지 않는다. 버퍼가 부족한 경우를 포함해 `getcwd` 실패를 검사한다. [chdir(2)](https://man7.org/linux/man-pages/man2/chdir.2.html), [getcwd(3)](https://man7.org/linux/man-pages/man3/getcwd.3.html).

**질문:** 프로그램이 종료된 후 `cat ./space\ name.txt`를 입력하면 어느 디렉토리에서 찾을까? → 부모 셸의 현재 위치. 방금 자식 프로그램이 이동한 위치가 아니다.

## 실습 4 · 비어 있지 않은 디렉토리와 빈 디렉토리 삭제

먼저 삭제 실패를 관찰한다.

```sh
./dir_remove "$DIR_LAB/demo"
echo $?
./dir_list "$DIR_LAB/demo"
```

`demo`에 항목이 남아 있으므로 실패한다. 종료 상태는 `1`이며 내부 항목은 보존된다.

이 실습에서 만든 두 파일과 하위 디렉토리를 제거한 뒤 다시 실행한다.

```sh
rm "$DIR_LAB/demo/.hidden" "$DIR_LAB/demo/space name.txt"
./dir_remove "$DIR_LAB/demo/sub"
./dir_remove "$DIR_LAB/demo"
echo $?
./dir_remove "$DIR_LAB"
```

이번에는 빈 디렉토리를 지우므로 `removed: ...`가 출력되고 종료 상태는 `0`이다. 마지막 명령은 비어 있는 실습 임시 폴더도 정리한다.

**설명:** `rmdir`는 빈 디렉토리만 제거한다. 일반 파일을 지우거나 내부 파일을 재귀적으로 삭제하는 함수가 아니다. [rmdir(2)](https://man7.org/linux/man-pages/man2/rmdir.2.html).

## 오류 입력과 확인 질문

실습 폴더를 정리하기 전 아래 입력도 비교할 수 있다. 실패한 명령 바로 다음에 `echo $?`로 종료 상태를 확인한다.

| 입력 | 기대 결과 |
|---|---|
| 인자 없이 `./dir_list` | 사용법 출력, 종료 상태 2 |
| `./dir_list "$DIR_LAB/missing"` | 없는 경로 오류, 종료 상태 1 |
| `./dir_chdir "$DIR_LAB/demo/space name.txt"` | 일반 파일로 이동 불가, 종료 상태 1 |
| `./dir_remove "$DIR_LAB/demo/space name.txt"` | 일반 파일은 삭제하지 않음, 종료 상태 1 |

- `readdir`의 NULL은 왜 곧바로 오류라고 판단할 수 없는가?
- `dir_chdir` 안에서 이동했는데 부모 셸의 `pwd`가 그대로인 이유는?
- 숨김 파일 하나가 남아 있는 디렉토리는 비어 있는가?
- 디렉토리의 이름 목록만 얻는 작업과 각 파일의 속성을 얻는 작업은 어느 코드에 있는가?

## 자동 확인

Python 3 표준 라이브러리로 새 임시 폴더를 만들어 검사한다.

```sh
make test-directory
```

생성·umask·중복 이름, 빈 폴더·숨김 파일·공백 이름·깨진 링크의 목록, 작업 위치 변경, 삭제 실패 시 내용 보존, 인자 오류를 검사한다. 일반 사용자로 실행하면 접근 권한 오류도 검사한다. root에서는 권한 검사를 건너뛴다. 권한 계산 검사는 기본 ACL이 없는 임시 폴더를 전제로 한다.

로컬 검증 환경은 macOS의 POSIX API이다. Linux API 설명은 위 Linux man-pages와 대조했으며, 같은 테스트를 Ubuntu에서도 실행할 수 있다.
