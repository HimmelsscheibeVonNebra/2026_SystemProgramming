
/*
 * mapcat.c
 *
 * mmap()을 이용하여 파일을 메모리에 매핑하고
 * 파일 내용을 표준 출력(stdout)으로 출력한다.
 *
 * 사용법:
 *   ./mapcat FILE
 *   ./mapcat -p FILE
 *
 * 옵션:
 *   -p : 파일 출력 후 프로세스를 종료하지 않고
 *        Enter 입력을 기다린다.
 *
 *        대기 중 다른 터미널에서
 *        /proc/PID/maps를 확인할 수 있다.
 *
 * 주요 학습 내용:
 *   1. open()과 파일 디스크립터
 *   2. fstat()을 이용한 파일 정보 확인
 *   3. mmap()을 이용한 가상 메모리 매핑
 *   4. write()를 이용한 표준 출력
 *   5. munmap()을 이용한 매핑 해제
 *   6. /proc/PID/maps를 이용한 메모리 관찰
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

/*
 * write_all()
 *
 * write()는 요청한 바이트 수를 한 번에
 * 모두 기록한다는 보장이 없다.
 *
 * 따라서 남은 데이터가 없어질 때까지
 * write()를 반복 호출한다.
 *
 * fd : 출력 대상 파일 디스크립터
 * p  : 출력할 데이터의 시작 주소
 * n  : 출력할 데이터의 전체 크기
 *
 * 반환값:
 *    0 : 성공
 *   -1 : 실패 (errno 설정)
 */
static int write_all(int fd, const char *p, size_t n)
{
    while (n > 0) {

        // p에서 최대 n바이트를 fd에 출력
        ssize_t k = write(fd, p, n);

        if (k < 0) {

            /*
             * EINTR:
             * 시그널 때문에 시스템 콜이 중단된 경우
             *
             * 오류로 처리하지 않고 다시 시도한다.
             */
            if (errno == EINTR)
                continue;

            return -1;
        }

        // 진행이 없으면 무한 반복을 방지
        if (k == 0) {
            errno = EIO;
            return -1;
        }

        /*
         * 출력한 바이트만큼 포인터 이동
         *
         * 예:
         *   p = 0x1000
         *   k = 100
         *
         *   새로운 p = 0x1064
         */
        p += k;

        // 남은 출력 크기 감소
        n -= (size_t)k;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    /*
     * -p 옵션 확인
     *
     * ./mapcat file.txt
     *   argc = 2
     *   pause_flag = 0
     *
     * ./mapcat -p file.txt
     *   argc = 3
     *   pause_flag = 1
     */
    int pause_flag =
        (argc == 3 && strcmp(argv[1], "-p") == 0);

    // 잘못된 인자 개수 검사
    if (argc != 2 + pause_flag) {
        fprintf(stderr,
                "usage: %s [-p] FILE\n", argv[0]);
        return 2;
    }

    // -p 옵션 유무에 따라 파일 이름의 위치 결정
    const char *path = argv[1 + pause_flag];

    /*
     * 1. 파일 열기
     *
     * O_RDONLY: 읽기 전용
     *
     * 성공하면 파일 디스크립터(fd)를 반환한다.
     * 실패하면 -1을 반환한다.
     */
    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        perror(path);
        return 1;
    }

    /*
     * 2. 파일 정보 확인
     *
     * fstat()은 파일 크기, 종류, 권한 등의
     * 메타데이터를 struct stat에 저장한다.
     */
    struct stat st;

    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }

    /*
     * 일반 파일(regular file)인지 검사
     *
     * 디렉터리, 파이프, 소켓 등은 제외한다.
     */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr,
                "%s: not a regular file\n", path);
        close(fd);
        return 1;
    }

    /*
     * 3. 빈 파일 검사
     *
     * mmap()의 length는 0보다 커야 한다.
     *
     * 파일 크기가 0이면 매핑하지 않는다.
     */
    if (st.st_size == 0) {
        fprintf(stderr,
                "%s: empty file, nothing to map\n", path);
        close(fd);
        return 0;
    }

    // 파일 크기를 매핑 길이로 사용
    size_t len = (size_t)st.st_size;

    /*
     * 4. mmap() 호출
     *
     * 파일을 프로세스의 가상 주소 공간에 매핑
     *
     * NULL:
     *   매핑 시작 주소는 커널이 결정한다.
     *
     * len:
     *   매핑할 바이트 수
     *
     * PROT_READ:
     *   읽기 권한만 허용한다.
     *
     * MAP_PRIVATE:
     *   private 매핑을 생성한다.
     *   수정 시 Copy-on-Write가 적용된다.
     *
     * fd:
     *   매핑할 파일의 디스크립터
     *
     * 0:
     *   파일 시작 위치부터 매핑한다.
     *
     * 성공:
     *   매핑된 가상 메모리 주소 반환
     *
     * 실패:
     *   MAP_FAILED 반환
     */
    char *p = mmap(
        NULL,
        len,
        PROT_READ,
        MAP_PRIVATE,
        fd,
        0
    );

    if (p == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    /*
     * 5. 파일 디스크립터 닫기
     *
     * mmap()이 성공한 이후에는
     * fd를 닫아도 매핑이 유지된다.
     *
     * 파일 디스크립터와 메모리 매핑은
     * 서로 별도로 관리되기 때문이다.
     */
    close(fd);

    /*
     * 6. 매핑된 메모리에서 데이터 출력
     *
     * p는 파일 내용이 매핑된 가상 주소다.
     *
     * 별도의 read() 호출 없이
     * 해당 메모리를 write()에 전달한다.
     */
    if (write_all(STDOUT_FILENO, p, len) < 0) {
        perror("write");
        munmap(p, len);
        return 1;
    }

    /*
     * 7. -p 옵션이 있으면 대기
     *
     * 프로세스를 살아 있게 유지하여
     * /proc/PID/maps를 관찰할 수 있도록 한다.
     */
    if (pause_flag) {
        fprintf(stderr,
                "\n[mapcat] pid=%d "
                "mapped %zu bytes at %p "
                "- Enter to unmap\n",
                (int)getpid(),
                len,
                (void *)p);

        getchar();
    }

    /*
     * 8. 메모리 매핑 해제
     *
     * mmap()으로 생성한 가상 메모리 영역을
     * 프로세스 주소 공간에서 제거한다.
     */
    if (munmap(p, len) < 0) {
        perror("munmap");
        return 1;
    }

    return 0;
}
