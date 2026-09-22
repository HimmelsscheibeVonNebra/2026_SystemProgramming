# Week03-class01 실습 기록

학번 / 이름 / 실행 환경:

## 복습

1. 소스 수정 후 다시 컴파일해야 하는 이유:
2. `>`와 `>>`의 차이:
3. `echo $?`는 어느 명령의 상태를 보여 주는가?

## 네 프로그램 실행 결과

| 프로그램 | 실험 | 예상 | 실제 결과와 이유 |
|---|---|---|---|
| file_create | 새 파일 / 같은 파일 재생성 | | |
| file_create | umask 022 / 027 | | |
| file_write | truncate로 XY 기록 | | |
| file_write | append로 ZZ 기록 | | |
| file_read | 빈 파일 / 없는 파일 | | |
| file_read | 32바이트 파일, 버퍼 16 / 7 | | |
| file_seek | ABCDEFGHIJ의 위치 3에 XY | | |
| file_seek | 위치 0에 ZZ / 끝 위치 10에 ! | | |

## 코드 설명과 수정

1. open이 0을 반환해도 성공인 이유:
2. read 반환값으로 출력 길이를 정하는 이유:
3. file_seek에서 lseek를 두 번 호출하는 이유:
4. SEEK_SET, SEEK_CUR, SEEK_END의 기준:
5. SEEK_END로 마지막 두 바이트를 바꾼 코드와 결과:

제출 파일: file_create.c, file_write.c, file_read.c, file_seek.c, 이 기록지.
SEEK_END 변형은 별도 file_seek_end.c로 함께 제출한다.
