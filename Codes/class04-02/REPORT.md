# Week04-class02 · 파일 속성 실습 기록지

- 이름 / 학번:
- 운영체제·파일시스템·컴파일러:
- 실행한 빌드 명령과 결과:

## 1. 조회와 링크

| 경로 / 옵션 | 종류 | device | inode | links | size | 설명 |
|---|---|---|---|---|---|---|
| note.txt | | | | | | |
| hard.txt | | | | | | |
| soft.txt | | | | | | |
| --follow soft.txt | | | | | | |
| broken.txt | | | | | | |
| --follow broken.txt | | | | | | |

- 원본과 하드 링크가 같은 파일임을 보여 주는 필드:
- 심볼릭 링크의 size가 대상 파일의 size와 다른 이유:
- 없는 경로, 파일/child 경로의 오류와 종료 상태:
- st_mode 전체를 0644와 비교하면 안 되는 이유:

## 2. 권한

| 단계 | 요청 mode | umask | 예상 결과 | 실제 결과 |
|---|---|---|---|---|
| mask022.txt 생성 직후 | 0666 | 0022 | | |
| 첫 파일 fchmod 후 | 0640으로 변경 | 해당 없음 | | |
| mask077.txt 생성 | 0666 | 0077 | | |
| groupdir 생성 | 0777 | 0027 | | |

- note.txt에 chmod를 한 뒤 hard.txt에서도 변경이 보이는 이유:
- 000 권한인 파일의 속성을 조회할 수 있는 이유:
- 일반 파일의 w와 부모 디렉토리의 w·x가 제어하는 동작의 차이:

## 3. 크기

| st_size | st_blocks | st_blksize | 각 단위와 의미 |
|---|---|---|---|
| | | | |

- 1 MiB 위치에 한 바이트를 쓴 뒤 크기가 1,048,577인 이유:
- st_blocks에 st_blksize를 곱하면 안 되는 이유:
- 앞부분을 읽었을 때 나온 값:

## 4. 시간 속성

| 단계 | mode | atime | mtime | ctime |
|---|---|---|---|---|
| after_futimens | | | | |
| after_fchmod | | | | |

- mtime과 ctime의 차이:
- ctime을 생성 시각으로 해석하면 생기는 오류:
- 읽었는데 atime 표시값이 그대로일 수 있는 이유:

## 5. 열린 파일

| 단계 | inode | links | size | 경로 조회 / FD 읽기 |
|---|---|---|---|---|
| unlink 전 | | | | |
| unlink 후 | | | | |

- 이름이 없는데 read=hello가 가능한 이유:
- st_nlink가 열린 FD 수가 아닌 이유:

## 실제 명령과 출력 로그

```text

```

## 선택 확장

mini_ls에 추가한 속성 열, 코드와 실행 결과:
