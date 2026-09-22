# CS 537 디렉토리 그림 출처

저자: Barton P. Miller, University of Wisconsin–Madison

원문: [CS 537 Notes, Section 25: Directories](https://pages.cs.wisc.edu/~bart/537/lecturenotes/s25.html)

교안 4–7쪽에 원본 이미지 4개를 각각 삽입했습니다. 아래 파일은 원본 바이트를 그대로 보관하며, 교안에서는 비율을 유지해 배치했습니다.

| 교안 | 그림 | 원본 |
|---|---|---|
| 4 | 디렉토리 트리 | [s27.unixdir.gif](https://pages.cs.wisc.edu/~bart/537/lecturenotes/figures/s27.unixdir.gif) |
| 5 | 절대 경로 | [AbsolutePathNameExample.JPG](https://pages.cs.wisc.edu/~bart/537/lecturenotes/figures/AbsolutePathNameExample.JPG) |
| 6 | 상대 경로 | [RelativePathNameExample.JPG](https://pages.cs.wisc.edu/~bart/537/lecturenotes/figures/RelativePathNameExample.JPG) |
| 7 | 디렉토리·inode·데이터 블록 | [dir+inode.jpg](https://pages.cs.wisc.edu/~bart/537/lecturenotes/figures/dir%2Binode.jpg) |

원문 저작권 표기: Copyright © 2013, 2018, 2020 Barton P. Miller. 원문은 위스콘신대학교 외부 학생·교사의 개인적 인쇄 이용을 안내하며, 추가 복제에는 저자의 허락이 필요하다고 명시합니다. 이 자료는 이미지에 별도의 공개 라이선스를 부여하지 않습니다. 이용 조건은 원문 하단을 참조합니다.

마지막 그림의 /a/b 디렉토리 엔트리는 `c`라고 적혀 있지만 본문과 오른쪽 경로는 `c.c`입니다. 원본을 수정하지 않고 교안과 대본에 표기 불일치를 설명했습니다. 그림의 1–8은 디렉토리당 데이터 한 블록, 캐시 생략 등 단순화 조건에서의 읽기 순서이며, 실제 Linux가 항상 수행하는 물리 I/O 횟수를 뜻하지 않습니다.

확인일: 2026-09-22
