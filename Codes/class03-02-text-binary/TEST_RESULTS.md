# 소스코드 검증 결과

검증일: 2026-09-15

## 환경과 결과

| 환경 | 컴파일러 | 결과 |
| --- | --- | --- |
| macOS / Darwin 25.5.0 | Apple Clang 21.0.0 | 통합 검사 178개 통과 |
| Linux 7.0.0-30-generic / Ubuntu | GCC 13.3.0 | 통합 검사 184개 통과 |
| macOS AddressSanitizer + UndefinedBehaviorSanitizer | Apple Clang 21.0.0 | 동일한 178개 검사 통과 |

각 환경에서 write_all 함수 테스트 8개도 통과했습니다.
통합 검사 수는 프로그램 종료 상태와 파일 내용 등의 개별 검증 조건 수입니다.
macOS의 /dev/full 검사는 건너뛰었으며 Linux에서 실행했습니다.

기본 빌드 옵션:

```text
-std=c17 -Wall -Wextra -Wpedantic -Werror -g
```

메모리·정의되지 않은 동작 검사 추가 옵션:

```text
-fsanitize=address,undefined -fno-omit-frame-pointer
```

## 확인 범위

- 텍스트 저장·덮어쓰기·추가·읽기 및 lseek 위치 변경
- ASCII, UTF-8, NUL 포함 데이터, 빈 파일
- 1, 15, 16, 17, 4095, 4096, 4097, 65537바이트 읽기
- 시작·끝·끝 너머 쓰기, 빈 문자열 쓰기, 잘못된 위치 값
- 바이너리 파일의 native int 바이트 표현과 전체 읽기
- 인덱스 0/1/2 선택 읽기, 두 번째 항목 수정, 다른 항목 보존
- INT_MIN/INT_MAX 값, 정수 범위 초과 및 잘못된 문자열 거부
- 3 * sizeof(int)보다 짧은 모든 파일 길이: 빈 파일과 불완전한 항목 포함
- 추가 바이트가 붙은 파일 거부, 오류 시 원본 내용 보존
- 인자 누락, 존재하지 않는 경로, 디렉터리, 접근 권한 거부
- 파이프 입력 읽기 성공, 비탐색 스트림의 fseek 실패 처리
- Linux /dev/full: 저수준 쓰기, stdio 지연 쓰기, stdout 오류
- 가짜 write: 부분 쓰기, 반복 EINTR, 진행 없는 0, ENOSPC, EBADF, EAGAIN, 빈 요청

## Linux strace 확인

서버의 격리된 작업 폴더: `/tmp/week03-text-binary.kfUB8S`

텍스트 예제는 lseek 위치 3, read 반환 7, 다시 lseek 위치 3, write XY 2바이트, close 성공을 확인했습니다.
실습 소스는 최대 16바이트를 읽으므로 출력은 DEFGHIJ입니다. 슬라이드의 2바이트 DE 예제와 읽기 길이는 다르지만 최종 수정 결과는 동일한 ABCXYFGHIJ입니다.

바이너리는 int 크기 4바이트 환경에서 두 번째 항목 위치 4에 99의 메모리 바이트를 기록했습니다.
재읽기 결과는 `10 99 30`, 종료 상태는 0입니다.
stdio의 읽기 버퍼와 위치 조정 때문에 fseek 호출과 lseek 시스템 호출은 일대일로 대응하지 않았습니다.

## 범위와 사용 조건

정상 경로뿐 아니라 실제 입출력 오류를 검사했습니다. EINTR·부분 쓰기·진행 없는 반환은 가짜 write로 재현한 함수 테스트입니다.
모든 파일시스템·정전·실제 신호 타이밍의 동작을 검증한 것은 아닙니다.
바이너리 형식은 같은 환경에서 사용하도록 만든 헤더 없는 int 3개입니다.

## 재실행

```sh
make test
```
