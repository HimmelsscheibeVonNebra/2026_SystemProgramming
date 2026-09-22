# 2026 시스템 프로그래밍

강의 교안은 `Presentations/`, C 실습 소스는 `Codes/`에 있습니다. 교안 안의 `labs/` 경로는 이 저장소의 `Codes/`에 해당합니다.

## 3주차

| 자료 | 교안 | 원고·실습 |
|---|---|---|
| 1차시: 파일 생성·쓰기·읽기·위치 이동 | [ODP](Presentations/class03-01.odp) · [PDF](Presentations/class03-01.pdf) | [원고](Presentations/class03-01.md) · [실습](Codes/class03-01/README.md) |
| 2차시: 견고한 반복문과 파일 복사 | [ODP](Presentations/class03-02.odp) · [PDF](Presentations/class03-02.pdf) | [차시 안내](Presentations/class03-02-README.md) · [실습](Codes/class03-02/README.md) |
| 2차시 확장: 버퍼와 stdio | [ODP](Presentations/class03-02-stdio.odp) · [PDF](Presentations/class03-02-stdio.pdf) | [원고](Presentations/class03-02-stdio.md) |
| 텍스트와 바이너리 입출력 | [ODP](Presentations/class03-02-text-binary.odp) · [PDF](Presentations/class03-02-text-binary.pdf) | [원고](Presentations/class03-02-text-binary.md) · [실습](Codes/class03-02-text-binary/README.md) |

배포 묶음: [1차시](Presentations/class03-01-materials.zip), [2차시](Presentations/class03-02-materials.zip), [텍스트·바이너리 실습 소스](Presentations/class03-02-text-binary-sources.zip).

## 4주차

| 자료 | 교안 | 대본·실습 |
|---|---|---|
| 1차시: 파일과 디렉토리 | [ODP](Presentations/class04-01.odp) · [PDF](Presentations/class04-01.pdf) · [PPTX](Presentations/class04-01.pptx) | [120분 대본](Presentations/class04-01-script.md) · [실습](Codes/class04-01/README.md) · [디렉토리 실습](Codes/class04-01/DIRECTORY.md) |
| 2차시: 파일 속성 | [ODP](Presentations/class04-02.odp) · [PDF](Presentations/class04-02.pdf) · [PPTX](Presentations/class04-02.pptx) | [120분 대본](Presentations/class04-02-script.md) · [실습](Codes/class04-02/README.md) · [전체 묶음](Presentations/class04-02-materials.zip) |

4주차 2차시는 38장입니다. 4–7쪽의 디렉토리·경로·inode 그림은 [CS 537 원문 이미지와 출처 안내](Presentations/assets/cs537/README.md)를 참고하세요.

## 실습 실행

Ubuntu 일반 사용자 환경에서 C 컴파일러, `make`, `python3`를 준비합니다. 각 실습 폴더의 README를 따라 실행합니다.

```sh
make -C Codes/class03-01
make -C Codes/class03-02 test
make -C Codes/class03-02-text-binary test
make -C Codes/class04-01 test-directory
make -C Codes/class04-02 check
```

실행 파일과 임시 실습 데이터는 커밋 대상에서 제외합니다. 운영체제별 검증 범위와 Linux 전용 명령은 각 실습 안내에 기록했습니다.

ZIP 자료는 독립적으로 사용할 수 있도록 원래의 `weekXX-...` 파일명과 `labs/` 구조를 유지합니다.
