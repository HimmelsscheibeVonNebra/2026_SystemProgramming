# 2026 시스템 프로그래밍

**V12** · 2026-11-16

교안은 모두 `Presentations/`에, 실습 코드와 그 안내 문서(README·REPORT·INSTRUCTOR)는 모두 `Codes/classNN-MM/`에 있습니다. 교안에 나오는 `labs/classNN-MM`은 `Codes/classNN-MM/`에 해당합니다. 리눅스 커널 내부는 별도 프로젝트로 분리해 다루지 않습니다.

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

## 5주차

| 자료 | 교안 | 대본·실습 |
|---|---|---|
| 1차시: 시스템 정보와 자원 한계 | [ODP](Presentations/class05-01.odp) · [PDF](Presentations/class05-01.pdf) · [PPTX](Presentations/class05-01.pptx) | [120분 대본](Presentations/class05-01-script.md) · [차시 안내](Presentations/class05-01-README.md) · [실습](Codes/class05-01/README.md) · [전체 묶음](Presentations/class05-01-materials.zip) · [코드 리딩](Presentations/class05-01-code.odp) |

5주차 1차시는 35장입니다. uname·sysinfo로 시스템을 식별하고, sysconf의 반환값 계약(errno 초기화와 -1의 두 의미)을 다루며, getrlimit·setrlimit로 soft·hard 자원 한계를 조회·변경하고 EMFILE을 관찰합니다. 마지막에 /proc 관찰 창과 표준 API의 역할 분담을 정리합니다.
5주차 1차시 보조 교안 [실습 코드 한 줄씩 읽기](Presentations/class05-01-code.odp)(32장)는 다섯 예제 360행 전체를 교안 본편의 1부~4부 챕터별로 행 번호와 함께 싣고, 설명은 파란 글자로 구분하며 발표자 노트와 [원고](Presentations/class05-01-code.md)에 매 줄의 설명이 있습니다. 말미의 참고 슬라이드는 함수의 원형·glibc·커널 구현 위치를 kernel.org stable(linux-7.2.y) 링크로 안내하며, class02가 사용하는 시간 함수(time, clock_gettime, localtime 계열)도 함께 다룹니다.


## 7주차

| 자료 | 교안 | 실습 |
|---|---|---|
| 1차시: 프로세스 기본 · 주소 공간·상태·자원 | [ODP](Presentations/class07-01.odp) · [PDF](Presentations/class07-01.pdf) · [PPTX](Presentations/class07-01.pptx) | [실습](Codes/class07-01/README.md) · [코드 읽기 원고](Presentations/class07-01-code.md) |
| 2차시: 프로세스 심화 · 측정하고 실험하기 | [ODP](Presentations/class07-02.odp) · [PDF](Presentations/class07-02.pdf) · [PPTX](Presentations/class07-02.pptx) | [실습](Codes/class07-02/README.md) · [코드 읽기 원고](Presentations/class07-02-code.md) |

7주차 교안은 V05이며 1차시 82장, 2차시 90장입니다. 본편 내용은 V03과 같고, 각 부의 직접 확인 슬라이드 바로 뒤에 그 부 예제의 코드 읽기가 들어갑니다. 코드 읽기는 변수 표(전역·지역·static 지역·매개변수·힙 꼬리표)와 행 번호가 붙은 소스 전체, 행 묶음별 설명으로 되어 있고 발표자 노트에 상세 설명이 있습니다. 1차시 코드 읽기는 19–33장(addr_show·maps_show), 46–52장(state_show), 63–78장(hold_open·procdump)이고, 2차시는 15–26장(pagetouch), 39–55장(pstat·cpu_wait), 64–70장(reap_show), 78–87장(limit_lab)입니다. 1차시는 주소 공간·상태·FD 표·자원 한계를 프로그램의 보고와 ps·/proc의 관찰로 교차 확인하고, 2차시는 요구 페이징·%CPU·문맥 교환·좀비와 고아·한계 초과를 예측 → 측정 → 설명의 순서로 실험합니다. ODP·PPTX에서 본편 페이지는 그림, 코드 읽기 페이지는 편집 가능한 텍스트입니다. 실습 README는 예제마다 실행 명령, 출력 읽기, 핵심 포인트, macOS 차이를 담고, 코드 읽기 원고는 같은 내용의 Markdown입니다.

## 9주차

| 자료 | 교안 | 실습·안내 |
|---|---|---|
| 1차시: 시그널 | [PPTX V04](Presentations/week09-class01-v04.pptx) | [실습](Codes/class09-01/README.md) · [기록지](Codes/class09-01/REPORT.md) · [교수자 안내](Codes/class09-01/INSTRUCTOR.md) |
| 2차시: 시그널 예제 | — | [소스](Codes/class09-02/) |

## 10주차

| 자료 | 교안 | 실습·안내 |
|---|---|---|
| 1차시: fork·exec·wait | [Keynote V02](Presentations/week10-class01-v02.key) | [실습](Codes/class10-01/README.md) · [기록지](Codes/class10-01/REPORT.md) · [교수자 안내](Codes/class10-01/INSTRUCTOR.md) |
| 3차시: 부모가 자식을 제어하기 | [PPTX V01](Presentations/week10-class03-v01.pptx) | [실습](Codes/class10-03/README.md) |

## 11주차

| 자료 | 교안 | 실습·안내 |
|---|---|---|
| 1차시: 메모리 매핑 | [PPTX V08](Presentations/week11-class01-v08.pptx) · [이전 V02](Presentations/week11-class01-v02.pptx) | [실습 안내](Codes/class11-01/README.md) · [구성과 학습 효과](Codes/class11-01/LECTURE.md) |

78장 교안입니다. 파일 없는 입문 예제 `mmap_basic`으로 시작해 `mapcat`, `mapmode`, `countnl`을 예제별 섹션으로 학습합니다. 주석 포함 소스는 `Codes/class11-01/`에 있습니다.

## 12주차

| 자료 | 교안 | 실습 |
|---|---|---|
| 1차시: 파이프 | [PPTX V02](Presentations/week12-class01-v02.pptx) | [120분 대본](Presentations/week12-class01-script.md) · [소스](Codes/class12-01/) |

## 실습 실행

Ubuntu 일반 사용자 환경에서 C 컴파일러, `make`, `python3`를 준비합니다. 각 실습 폴더의 README를 따라 실행합니다.

```sh
make -C Codes/class03-01
make -C Codes/class03-02 test
make -C Codes/class03-02-text-binary test
make -C Codes/class04-01 test-directory
make -C Codes/class04-02 check
make -C Codes/class05-01 check
make -C Codes/class07-01 check
make -C Codes/class07-02 check
```

실행 파일과 임시 실습 데이터는 커밋 대상에서 제외합니다. 운영체제별 검증 범위와 Linux 전용 명령은 각 실습 안내에 기록했습니다.

ZIP 자료는 독립적으로 사용할 수 있도록 원래의 `weekXX-...` 파일명과 `labs/` 구조를 유지합니다.

## 변경 이력

| 버전 | 날짜 | 내용 |
|---|---|---|
| V01 | — | 3·4·5주차 색인과 실습 실행 안내. |
| V02 | 2026-10-13 | 7주차 항목(교안 PDF, 실습·예제 상세 설명 README)과 `make check` 명령 두 줄 추가. |
| V03 | 2026-10-13 | 7주차 설명 문단 간결화. |
| V04 | 2026-10-13 | 7주차 코드 전체 설명 문서 링크 추가. |
| V05 | 2026-10-13 | 7주차 코드 읽기 보조 교안(ODP·PDF·PPTX) 링크 추가. |
| V06 | 2026-10-13 | 7주차 교안을 코드 읽기 장과 합친 V04(ODP·PDF·PPTX)로 교체, 보조 교안 링크 제거. |
| V07 | 2026-10-13 | 7주차 교안 V05: 코드 읽기를 각 부의 직접 확인 뒤로 재배치. 쪽 번호 갱신. |
| V08 | 2026-10-14 | 8주차 이후 제작 규칙(architecture.md), output·docs·tools 폴더 안내 추가. |
| V09 | 2026-11-10 | 11주차 V08 교안·입문 예제와 학습 효과 안내, 주차별 폴더 정리. |
| V10 | 2026-11-10 | 11주차 코드를 Code/class11-01로, 교안을 하위 폴더 없는 Presentation으로 이동. |
| V11 | 2026-11-16 | 폴더 정리: 모든 교안을 Presentations로, 모든 실습 코드와 안내 문서를 Codes로 통합. Code·Presentation·output·docs 폴더 제거, 9·10·12주차 표 추가. |
| V12 | 2026-11-16 | 12주차 1차시 120분 진행 대본 링크 추가. |
