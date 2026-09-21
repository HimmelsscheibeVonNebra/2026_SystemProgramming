# Week04-class01 · 리눅스 파일과 디렉토리

120분 수업. 파일 속성·권한·링크·디렉토리 API.

## 1. 리눅스 파일과 디렉토리

## 2. 수업 구성

1부  파일 이름, inode와 경로 · 15분

2부  파일 속성과 접근 권한 · 25분

3부  하드 링크와 심볼릭 링크 · 25분

4부  디렉토리 탐색과 관리 · 30분

확인 퀴즈와 실습 정리 · 15분

120분 수업 · 2부 후 휴식 10분

교수자 메모: 총 120분. 2부 이후 휴식 10분을 포함한다. 파일 입출력에서 다뤘던 바이트 읽기·쓰기와 이번 시간의 파일 속성·이름·디렉토리 관리를 연결한다.

## 3. 1부 파일과 경로

파일 이름, inode, 데이터의 관계를 살펴봅니다.

## 4. 파일 이름, inode와 데이터

디렉토리 엔트리는 파일 이름과 inode 번호를 연결한다.

inode에는 파일 종류, 권한, 소유자 등의 정보가 있다.

일반 파일의 내용은 파일시스템이 관리하는 바이트다.

같은 파일을 여러 이름으로 참조할 수 있다.

교수자 메모: inode 번호는 같은 파일시스템 안에서만 고유하다. 파일을 비교할 때 st_dev와 st_ino를 함께 본다. 파일 이름은 inode의 필드가 아니다. 학생에게 이름 변경이 파일 복사와 어떻게 다른지 먼저 질문한다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man7/inode.7.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/link.2.html)

## 5. Linux의 파일 종류

-  일반 파일: 텍스트, 이미지, 실행 파일

d  디렉토리: 파일 이름과 대상을 연결

l  심볼릭 링크: 다른 대상을 가리키는 경로

c / b  문자 장치와 블록 장치

p / s  FIFO와 소켓

교수자 메모: ls -l의 첫 문자는 파일 종류다. 확장자는 이름의 일부이며 파일 종류 비트를 바꾸지 않는다. 장치·FIFO·소켓은 일반 파일과 다른 동작을 가진다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man7/inode.7.html)

## 6. 절대 경로와 상대 경로

절대 경로: /에서 시작한다.

상대 경로: 현재 작업 디렉토리 등을 기준으로 해석한다.

특수 이름: .은 현재 디렉토리, ..은 부모 디렉토리다.

```
/home/student/work/note.txt

# 현재 위치: /home/student/work
./note.txt
../work/note.txt

pwd
ls -ld . ..
ls -l ./note.txt
```

교수자 메모: 예시의 세 경로는 현재 작업 디렉토리가 주어진 조건에서 같은 파일을 가리킨다. 중간 디렉토리에는 검색 권한이 필요하다. openat 계열에서는 기준 디렉토리 FD를 지정할 수 있다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man7/path_resolution.7.html)

## 7. 2부 파일 속성과 권한

ls와 stat의 정보를 C 프로그램에서 읽습니다.

## 8. ls -l 출력 읽기

종류와 권한: 첫 문자와 다음 9개 문자를 나누어 읽는다.

소유권: 사용자와 그룹별로 권한을 구분한다.

크기: 일반 파일의 크기는 내용의 바이트 수다.

```
$ ls -l note.txt
-rw-r----- 2 student lab 4 Sep 21 10:00 note.txt

# 유형       -
# 권한       rw- r-- ---
# 링크 수    2
# 소유자     student
# 그룹       lab
# 크기       4 bytes
```

출력 예시: 계정, 그룹, 시각은 실행 환경에 따라 달라집니다.

교수자 메모: 출력은 설명을 위한 예시이며 실제 사용자·그룹·시각은 환경에 따라 달라진다. ls -ld 디렉토리는 디렉토리 자체의 정보를 보여준다. 크기를 디렉토리 안의 파일 개수로 해석하지 않는다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man7/inode.7.html)

## 9. stat, lstat, fstat

stat(path, &st): 경로가 가리키는 파일의 속성을 읽는다. 마지막 심볼릭 링크를 따라간다.

lstat(path, &st): 마지막 경로 요소가 심볼릭 링크이면 링크 자체의 속성을 읽는다.

fstat(fd, &st): 이미 열어 둔 파일 디스크립터로 속성을 읽는다.

공통 헤더: <sys/stat.h> · 성공 0, 실패 -1

교수자 메모: 공통 반환값은 성공 0, 실패 -1이다. 실패하면 errno를 확인한다. lstat도 경로 중간의 심볼릭 링크는 해석한다. 조회 전에 파일 내용을 읽을 권한 자체가 필요한 것은 아니지만 경로 디렉토리의 검색 권한은 필요하다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/stat.2.html)

## 10. struct stat의 주요 필드

st_dev, st_ino: 파일시스템 식별 정보와 inode 번호

st_mode: 파일 종류와 접근 권한 비트

st_nlink: 하드 링크 수    st_uid, st_gid: 소유자

st_size: 일반 파일의 바이트 크기

st_atime / st_mtime / st_ctime: 접근 / 내용 수정 / 상태 변경

ctime은 생성 시각이 아니라 inode 상태 변경 시각입니다.

교수자 메모: ctime은 생성 시각이 아니다. atime 갱신은 마운트 옵션 등에 영향을 받는다. 파일 종류에 따라 st_size의 의미가 다르며 심볼릭 링크에서는 저장된 경로 문자열의 길이다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man7/inode.7.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/stat.2.html)

## 11. mystat: 속성 조회와 파일 종류

호출 결과: 조회에 성공한 뒤에만 st의 필드를 사용한다.

종류 판정: S_ISREG, S_ISDIR, S_ISLNK 매크로를 사용한다.

출력 형식: 정수형을 uintmax_t로 변환해 %ju로 출력한다.

```
struct stat st;
if (lstat(argv[1], &st) == -1) {
    perror(argv[1]);
    return 1;
}

if (S_ISREG(st.st_mode)) puts("regular");
else if (S_ISDIR(st.st_mode)) puts("directory");
else if (S_ISLNK(st.st_mode)) puts("symlink");

printf("inode=%ju links=%ju\n",
       (uintmax_t)st.st_ino,
       (uintmax_t)st.st_nlink);
```

완성 예제: labs/class04-01/mystat.c

교수자 메모: 핵심 코드 발췌다. 완성된 mystat.c에는 인자 개수 검사와 필요한 헤더가 있다. lstat을 사용하므로 symlink 자체를 구별할 수 있다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/stat.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man7/inode.7.html)

## 12. 접근 권한 비트

소유자(u), 그룹(g), 기타 사용자(o)에 각각 rwx를 부여한다.

일반 파일의 r은 읽기, w는 쓰기, x는 실행 권한이다.

r = 4, w = 2, x = 1을 더해 8진수로 나타낸다.

0640 = rw-r-----    0755 = rwxr-xr-x

st_mode & 0777로 기본 rwx 비트만 추출한다.

교수자 메모: 여기서는 기본 rwx 비트에 집중한다. set-user-ID, set-group-ID, sticky bit는 별도 비트이며 ACL, capability와 읽기 전용 마운트도 실제 접근에 영향을 준다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/chmod.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man7/path_resolution.7.html)

## 13. 디렉토리의 r, w, x

r: 이름 목록 읽기: 디렉토리 안의 파일 이름을 나열할 수 있다.

x: 이름 검색과 경로 통과: 이미 아는 이름으로 내부 객체를 찾고 경로를 따라갈 수 있다.

w: 디렉토리 엔트리 변경: 이름 생성·삭제에는 일반적으로 디렉토리의 w와 x가 함께 필요하다.

일반 사용자와 기본 권한을 기준으로 관찰합니다.

교수자 메모: 파일 내용 쓰기와 디렉토리의 이름 변경은 구분한다. 파일이 읽기 전용이어도 부모 디렉토리의 권한에 따라 그 이름을 삭제할 수 있다. sticky bit, ACL, capability, 파일 속성 등 추가 조건이 있을 수 있다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man7/path_resolution.7.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/unlink.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/mkdir.2.html)

## 14. chmod와 umask

chmod: 기존 파일의 권한을 바꾼다. 반환값을 확인한다.

umask: 새 객체 생성 시 허용하지 않을 비트를 제거한다.

8진수 표기: C의 0640과 십진수 640을 구분한다.

```
# 기존 파일의 권한 변경
chmod 640 note.txt
ls -l note.txt

# C API: 성공 0, 실패 -1
if (chmod("note.txt", 0640) == -1)
    perror("chmod");

# 새 파일의 생성 권한 예시
0666 & ~0022 = 0644
```

생성 권한 계산은 부모 디렉토리에 기본 ACL이 없는 경우입니다.

교수자 메모: 셸 명령, C 코드, 계산식을 구분해 제시한 개념 예시다. 기본 ACL이 없는 디렉토리에서 생성 권한은 요청 mode에서 umask 비트를 제거한 값이다. umask는 이미 존재하는 파일의 권한을 바꾸지 않는다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/chmod.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/umask.2.html)

## 15. 실습 1: 파일 속성 확인

내용과 크기: ABC와 개행을 합쳐 4바이트인지 확인한다.

종류와 권한: regular / directory를 구별하고 권한 변경 전후를 비교한다.

실패 경로: 없는 경로의 오류 메시지와 비정상 종료 상태를 확인한다.

```
RUN=$(mktemp -d)
printf 'ABC\n' > "$RUN/note.txt"
mkdir "$RUN/sub"

./mystat "$RUN/note.txt"
./mystat "$RUN/sub"
chmod 640 "$RUN/note.txt"
./mystat "$RUN/note.txt"
./mystat "$RUN/missing"
```

이후 실습도 같은 셸의 RUN 디렉토리에서 진행합니다.

교수자 메모: 실습 폴더 변수 RUN은 이후 실습에서도 같은 셸에서 이어서 사용한다. 명령 실행 직후 echo $?로 종료 상태를 확인할 수 있다. 로그에는 inode의 실제 숫자를 고정하지 않는다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/stat.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/chmod.2.html)

## 16. 3부 하드 링크와 심볼릭 링크

같은 파일에 붙는 이름과 경로를 저장하는 링크를 비교합니다.

## 17. 하드 링크

같은 파일의 다른 이름: 여러 디렉토리 엔트리가 같은 inode를 가리킨다.

관찰할 값: 같은 파일시스템에서 st_ino가 같고 st_nlink가 증가한다.

생성 범위: 서로 다른 파일시스템 사이에는 만들 수 없다. 디렉토리는 일반적으로 제한한다.

명령: ln old.txt new.txt    C API: link(oldpath, newpath)

교수자 메모: 두 하드 링크 사이에는 원본과 복사본의 구분이 없다. 하나의 이름을 통해 내용을 수정하면 다른 이름에서도 같은 파일 내용을 읽는다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/link.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man7/inode.7.html)

## 18. 심볼릭 링크

다른 경로를 저장하는 파일: 대상을 찾을 때 저장된 경로를 해석한다. 별도의 inode를 가진다.

대상과 수명이 분리됨: 대상이 없어도 링크는 존재할 수 있다. 디렉토리나 다른 파일시스템도 가리킨다.

상대 경로의 기준: 상대 대상 경로는 링크가 들어 있는 디렉토리를 기준으로 해석한다.

명령: ln -s target.txt shortcut.txt    C API: symlink(target, linkpath)

교수자 메모: 명령 ln -s TARGET LINK_NAME의 두 인자 순서를 강조한다. 상대 symlink는 사용자가 현재 위치를 바꿔도 링크가 놓인 디렉토리를 기준으로 찾는다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man7/symlink.7.html)

## 19. 심볼릭 링크에서 stat과 lstat

stat: 대상 파일을 따라가므로 대상이 없으면 실패한다.

lstat: 링크 자체를 조회한다. S_ISLNK로 판정할 수 있다.

오류 처리: 실제 코드에서는 모든 호출의 반환값을 검사한다.

```
# links.txt는 original.txt를 가리키는 symlink
stat("links.txt", &st);
/* original.txt의 속성 */

lstat("links.txt", &st);
/* links.txt 자체의 속성 */

# original.txt가 없어진 뒤
stat("links.txt", &st);   /* -1, ENOENT */
lstat("links.txt", &st);  /* 링크는 조회 가능 */
```

깨진 링크: 링크 파일은 남아 있으나 대상 경로를 해석할 수 없는 상태

교수자 메모: 다른 경로 접근 오류가 없는 실습 조건을 가정한다. 이 페이지의 코드는 두 호출의 차이를 보여주는 개념 발췌이며 실행 가능한 하나의 프로그램은 아니다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/stat.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man7/symlink.7.html)

## 20. 실습 2: 이름을 삭제한 뒤 비교

삭제 전: note.txt와 hard.txt의 inode, 링크 수를 비교한다.

삭제 후: hard.txt로 내용을 읽을 수 있고 soft.txt는 깨진 링크가 된다.

링크 자체: mystat은 lstat을 사용하므로 soft.txt의 속성은 읽을 수 있다.

```
ln "$RUN/note.txt" "$RUN/hard.txt"
ln -s note.txt "$RUN/soft.txt"

ls -li "$RUN"
./mystat "$RUN/hard.txt"
./mystat "$RUN/soft.txt"

rm "$RUN/note.txt"
cat "$RUN/hard.txt"
cat "$RUN/soft.txt"
./mystat "$RUN/soft.txt"
```

실습 1에서 생성한 RUN 디렉토리 안에서만 실행합니다.

교수자 메모: rm은 이 실습에서 만든 note.txt의 이름만 제거한다. hard.txt의 st_nlink는 2에서 1로 줄어든다. 심볼릭 링크 생성은 대상의 하드 링크 수를 늘리지 않는다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/link.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/unlink.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man7/symlink.7.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/stat.2.html)

## 21. unlink와 열린 파일의 수명

unlink는 디렉토리에서 이름 하나를 제거한다.

다른 하드 링크가 남아 있으면 다른 이름으로 접근할 수 있다.

마지막 이름을 지워도 이미 열린 FD로 파일을 사용할 수 있다.

마지막 하드 링크와 열린 참조가 사라지면 저장 공간을 반환할 수 있다.

교수자 메모: 이름을 찾는 경로와 이미 획득한 FD를 구분한다. 본문은 일반적인 로컬 파일의 개념 모델이다. mmap 등의 참조도 파일의 수명에 영향을 줄 수 있다. C API unlink 성공 0, 실패 -1.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/unlink.2.html)

## 22. 4부 디렉토리 탐색과 관리

디렉토리의 이름 목록을 읽고 각 파일의 속성을 조회합니다.

## 23. 디렉토리 스트림

opendir(path): 디렉토리를 열어 DIR *를 얻는다. 실패하면 NULL을 반환한다.

readdir(dir): 다음 엔트리를 가리키는 struct dirent *를 얻는다.

closedir(dir): 디렉토리 스트림을 닫는다. 성공 0, 실패 -1이다.

헤더: <dirent.h> · readdir의 NULL은 끝 또는 오류입니다.

교수자 메모: DIR *는 디렉토리 순회를 관리하는 라이브러리 객체다. readdir의 d_name은 하위 항목의 이름이며 전체 경로가 아니다. DIR *를 free하지 않고 closedir로 정리한다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man3/opendir.3.html), [Linux man-pages](https://man7.org/linux/man-pages/man3/readdir.3.html), [Linux man-pages](https://man7.org/linux/man-pages/man3/closedir.3.html)

## 24. readdir의 끝과 오류 구분

매번 초기화: readdir를 호출하기 직전에 errno를 0으로 만든다.

NULL 판별: errno가 0이면 끝, 0이 아니면 오류다.

정리: 반복문을 끝낸 뒤에는 closedir를 호출한다.

```
for (;;) {
    errno = 0;
    struct dirent *ent = readdir(dir);
    if (ent == NULL) {
        if (errno != 0) {
            perror("readdir");
            status = 1;
        }
        break;
    }
    /* ent->d_name을 처리한다. */
}
if (closedir(dir) == -1) {
    perror("closedir"); status = 1;
}
```

완성 예제: labs/class04-01/mini_ls.c

교수자 메모: 핵심 반복문 발췌이며 dir는 opendir 성공 후의 포인터, status는 0으로 초기화한 int다. 반복문 안에서 다른 함수가 errno를 변경할 수 있으므로 처음 한 번만 0으로 만드는 방식은 피한다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man3/readdir.3.html), [Linux man-pages](https://man7.org/linux/man-pages/man3/closedir.3.html)

## 25. 디렉토리 엔트리 처리

d_name은 디렉토리 안의 이름이다. 전체 경로가 아니다.

다음 readdir 호출 전에 필요한 이름과 정보를 사용하거나 복사한다.

반환된 포인터를 free하지 않는다.

d_type이 DT_UNKNOWN일 수 있으므로 속성 조회를 준비한다.

출력 순서는 정렬되지 않을 수 있다. .과 ..의 처리 정책을 정한다.

교수자 메모: 이번 mini_ls는 .과 ..만 생략하고 .hidden 같은 숨김 파일 이름은 포함한다. 재귀 순회는 이번 실습 범위 밖이며 확장할 때 symlink 추적 정책과 순환 방지를 추가해야 한다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man3/readdir.3.html)

## 26. 엔트리의 속성 조회

기준 디렉토리: 열린 디렉토리의 FD를 기준으로 이름을 찾는다.

링크 정책: AT_SYMLINK_NOFOLLOW로 마지막 symlink 자체를 조회한다.

개별 실패: 실패한 이름을 기록하고 다음 엔트리를 읽는다.

```
struct stat st;
int dfd = dirfd(dir);
if (dfd == -1) {
    perror("dirfd");
    closedir(dir); /* 정리 후 실패 반환 */
    return 1;
}

if (fstatat(dfd, ent->d_name, &st,
            AT_SYMLINK_NOFOLLOW) == -1) {
    perror(ent->d_name);
    status = 1;
    continue;
}
```

dirfd는 반복 전에, fstatat은 반복 안에서 호출합니다.

교수자 메모: 설명용 두 구간을 함께 제시했다. dirfd는 반복문 전에 얻고 실패 시 closedir로 정리한다. fstatat 호출은 readdir로 이름을 얻은 반복문 안에서 실행한다. 실제 정리 경로는 mini_ls.c를 참고한다. fstatat은 경로 문자열 조합을 줄여주지만 동시 이름 변경 자체를 막지는 않는다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/stat.2.html)

## 27. 디렉토리 생성과 삭제

mkdir(path, mode): 새 디렉토리를 만든다. 생성 권한에는 umask 등의 조건이 적용된다.

rmdir(path): 비어 있는 디렉토리를 삭제한다. 안에 항목이 있으면 실패한다.

결과 확인: 두 함수 모두 성공 0, 실패 -1이다. errno로 실패 원인을 확인한다.

헤더: mkdir는 <sys/stat.h>, rmdir는 <unistd.h>

교수자 메모: 일반적인 실습에서 EEXIST는 이미 존재하는 이름, ENOTEMPTY는 비어 있지 않은 디렉토리다. 파일을 제거하는 unlink와 빈 디렉토리를 제거하는 rmdir를 구별한다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/mkdir.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/rmdir.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/umask.2.html)

## 28. 현재 작업 디렉토리

getcwd: 현재 작업 디렉토리의 절대 경로를 얻는다.

chdir: 현재 프로세스의 작업 디렉토리를 바꾼다.

영향 범위: 이후 상대 경로의 기준이 바뀐다. 부모 셸은 바뀌지 않는다.

```
char cwd[4096];

if (getcwd(cwd, sizeof cwd) == NULL) {
    perror("getcwd");
    return 1;
}
puts(cwd);

if (chdir("sub") == -1) {
    perror("chdir");
    return 1;
}
```

헤더: <unistd.h> · 경로 버퍼가 부족한 경우도 오류로 처리합니다.

교수자 메모: 핵심 코드 발췌다. 고정 크기 버퍼보다 경로가 길면 getcwd는 ERANGE로 실패할 수 있으므로 4096을 모든 환경의 최대값으로 가정하지 않는다. 자식 프로그램의 chdir는 부모 셸의 cwd를 바꾸지 않는다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/chdir.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man3/getcwd.3.html)

## 29. 실습 3: 작은 ls 실행

정상 입력: 하위 이름, 종류, 크기를 출력하는지 확인한다.

경계 입력: 빈 디렉토리, 숨김 파일, 공백이 있는 이름을 확인한다.

실패 입력: 일반 파일과 없는 경로를 인자로 주어 오류를 확인한다.

```
touch "$RUN/.hidden"
printf 'hello\n' > "$RUN/space name.txt"

./mini_ls "$RUN"
./mini_ls "$RUN/sub"
./mini_ls "$RUN/space name.txt"
./mini_ls "$RUN/missing"

# Ubuntu에서 시스템 호출 관찰
strace -e openat,getdents64,newfstatat,close \
  ./mini_ls "$RUN"
```

출력 순서와 inode 번호는 환경에 따라 달라집니다.

교수자 메모: mini_ls는 정렬과 재귀 없이 한 디렉토리의 엔트리를 조회한다. dot와 dot-dot만 건너뛴다. 이미 만든 soft.txt는 깨진 symlink여도 속성 조회가 가능하다. strace의 구체적인 syscall 이름은 libc와 아키텍처에 따라 다를 수 있다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man3/opendir.3.html), [Linux man-pages](https://man7.org/linux/man-pages/man3/readdir.3.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/stat.2.html)

## 30. 확인 퀴즈

하드 링크 두 개는 inode와 파일 내용을 어떻게 공유하는가?

깨진 심볼릭 링크에 stat과 lstat을 호출하면 어떻게 다른가?

디렉토리의 x 권한은 어떤 동작에 필요한가?

readdir가 NULL을 반환했을 때 어떤 값을 더 확인해야 하는가?

파일 이름을 삭제한 뒤에도 열린 FD로 읽을 수 있는 이유는?

교수자 메모: 정답: 1. 같은 파일시스템의 같은 inode와 내용을 공유한다. 2. 대상이 없으면 stat은 ENOENT, lstat은 링크 자체 조회가 가능하다. 3. 이름 검색과 경로 통과. 4. 호출 직전 0으로 만든 errno. 5. 열린 참조가 파일의 수명을 유지한다.

참고: [Linux man-pages](https://man7.org/linux/man-pages/man2/link.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/stat.2.html), [Linux man-pages](https://man7.org/linux/man-pages/man7/path_resolution.7.html), [Linux man-pages](https://man7.org/linux/man-pages/man3/readdir.3.html), [Linux man-pages](https://man7.org/linux/man-pages/man2/unlink.2.html)

## 31. 실습 정리와 제출

mystat으로 종류, inode, 링크 수, 권한, 크기를 확인한다.

링크 생성 전후와 이름 삭제 전후의 결과를 비교한다.

mini_ls에서 디렉토리 끝과 오류를 구분한다.

코드와 실행 로그, 관찰한 차이를 설명하는 글을 제출한다.

파일 내용, 파일 속성, 파일 이름의 관계를 구분해 설명합니다.

교수자 메모: 필수 로그: 일반 파일과 디렉토리, 하드 링크와 심볼릭 링크, 깨진 링크, 없는 경로. 설명에는 stat/lstat의 차이와 디렉토리의 x 권한을 포함한다. 추가 구현은 이름 정렬 또는 숨김 파일 옵션 중 하나를 선택하게 할 수 있다.
