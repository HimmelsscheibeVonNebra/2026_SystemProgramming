# 사용할 컴파일러와 경고·디버깅 옵션입니다.
CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -g
TARGET = helloworld
PREFIX = /usr/local
UNITDIR = /etc/systemd/system

# .PHONY는 아래 이름을 실제 파일이 아닌 '실행할 작업'으로 선언합니다.
# 예를 들어 clean이라는 파일이 있어도 make clean은 삭제 명령을 실행합니다.
# 이 선언이 없으면 같은 이름의 파일 때문에 작업을 생략할 수 있습니다.
# helloworld는 실제로 만드는 실행 파일이므로 .PHONY에 넣지 않습니다.
# 그래야 소스가 바뀌거나 실행 파일이 없을 때만 다시 컴파일합니다.
.PHONY: all run clean install uninstall service-test load unload

all: $(TARGET)

# 소스가 실행 파일보다 새롭거나 실행 파일이 없으면 컴파일합니다.
# $<는 첫 의존 파일, $@는 타깃입니다. 명령 앞 들여쓰기는 탭입니다.
$(TARGET): helloworld.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

# 필요하면 빌드한 뒤 실행합니다. Ctrl+C로 종료합니다.
run: $(TARGET)
	./$(TARGET)

# 현재 폴더의 실행 파일만 삭제합니다. 설치된 서비스는 유지됩니다.
clean:
	rm -f $(TARGET)

# 서비스의 ExecStart 경로와 PREFIX/bin/helloworld가 일치해야 합니다.
# DESTDIR은 패키징용 임시 설치 루트이며 기본값은 빈 문자열입니다.
# 파일만 설치합니다. daemon-reload와 서비스 시작은 별도로 수행합니다.
install: $(TARGET)
	install -d "$(DESTDIR)$(PREFIX)/bin" "$(DESTDIR)$(UNITDIR)"
	install -m 0755 $(TARGET) "$(DESTDIR)$(PREFIX)/bin/$(TARGET)"
	install -m 0644 helloworld.service "$(DESTDIR)$(UNITDIR)/helloworld.service"

# 제거 전에 systemctl disable --now로 서비스를 중지하고 등록을 해제합니다.
uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/$(TARGET)"
	rm -f "$(DESTDIR)$(UNITDIR)/helloworld.service"

# 별도 테스트 서비스를 등록하고 시작·중지·해제까지 확인합니다.
service-test:
	sudo $(MAKE) -f service-test.mk test

# 테스트 서비스 설정을 등록하고 systemd가 읽도록 합니다.
# 시작과 부팅 실행 등록은 별도의 start, enable 타깃으로 수행합니다.
load:
	sudo $(MAKE) -f service-test.mk load

# 테스트 서비스를 중지하고 부팅 등록 및 설정 파일을 제거합니다.
unload:
	sudo $(MAKE) -f service-test.mk unload
