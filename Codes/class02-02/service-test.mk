# 서비스 수명 주기를 Make 타깃으로 실습합니다.
# 전체 확인: sudo make -f service-test.mk test
# 개별 실습: sudo make -f service-test.mk register (이후 enable, start 등)
SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:
.NOTPARALLEL:
.DEFAULT_GOAL := help

UNIT ?= week02-make-test.service
export UNIT
export TEST_MAKEFILE := $(abspath $(firstword $(MAKEFILE_LIST)))

# .PHONY는 파일을 만드는 타깃이 아니라 실행할 작업임을 표시합니다.
# 같은 이름의 파일이 있어도 요청한 작업과 필요한 의존 작업을 수행합니다.
# 이 파일의 타깃은 모두 서비스 제어 또는 확인 작업이므로 여기에 나열합니다.
.PHONY: help check owned register load enable start status stop disable unregister unload test

help:
	@printf '%s\n' \
	  '전체 테스트: sudo make -f service-test.mk test' \
	  '개별 순서: load enable start status stop disable unload' \
	  '예: sudo make -f service-test.mk load' \
	  '실습 종료 후 unload로 테스트 서비스를 제거하세요.'

# 기존 등록·해제 타깃을 의존 대상으로 재사용합니다.
load: register

unload: unregister

# 관리자 권한, systemd 환경, 수업용 이름을 확인합니다.
check:
	@test "$$(id -u)" -eq 0 || { echo 'sudo make로 실행하세요.' >&2; exit 1; }
	[[ "$$(ps -p 1 -o comm=)" == systemd ]]
	[[ "$$UNIT" =~ ^week02-make-test(-[A-Za-z0-9]+)?\.service$$ ]]

# 이 Makefile이 만든 설정만 조작합니다.
owned: check
	@grep -qx '# Managed-by: week02-service-test.mk' "/etc/systemd/system/$$UNIT"

register: check
	@test ! -e "/etc/systemd/system/$$UNIT"
	test ! -L "/etc/systemd/system/$$UNIT"
	# 다른 경로에 같은 이름의 서비스가 있어도 등록을 중단합니다.
	[[ "$$(systemctl show "$$UNIT" -p LoadState --value)" == not-found ]]
	# 컴파일 없이 등록과 해제를 관찰할 수 있도록 sleep을 실행합니다.
	umask 022
	printf '%s\n' \
	  '# Managed-by: week02-service-test.mk' \
	  '[Unit]' \
	  'Description=Temporary Week02 Make service test' \
	  '[Service]' \
	  'Type=simple' \
	  'ExecStart=/usr/bin/sleep 300' \
	  'DynamicUser=yes' \
	  '[Install]' \
	  'WantedBy=multi-user.target' > "/etc/systemd/system/$$UNIT"
	systemctl daemon-reload
	[[ "$$(systemctl show "$$UNIT" -p LoadState --value)" == loaded ]]
	printf '[통과] 등록: %s\n' "$$UNIT"

enable: owned
	@systemctl enable "$$UNIT"
	systemctl is-enabled --quiet "$$UNIT"
	echo '[통과] 부팅 실행 등록'

start: owned
	@systemctl start "$$UNIT"
	sleep 1
	systemctl is-active --quiet "$$UNIT"
	pid=$$(systemctl show "$$UNIT" -p MainPID --value)
	[[ "$$pid" =~ ^[0-9]+$$ && "$$pid" -gt 0 ]]
	printf '[통과] 시작: PID=%s\n' "$$pid"

status: owned
	@systemctl show "$$UNIT" \
	  -p LoadState -p ActiveState -p MainPID -p UnitFileState

stop: owned
	@systemctl stop "$$UNIT"
	[[ "$$(systemctl show "$$UNIT" -p ActiveState --value)" == inactive ]]
	[[ "$$(systemctl show "$$UNIT" -p MainPID --value)" == 0 ]]
	echo '[통과] 중지: inactive, PID=0'

disable: owned
	@systemctl disable "$$UNIT"
	# disabled 상태의 종료값은 0이 아니므로 출력 문자열을 비교합니다.
	state=$$(systemctl is-enabled "$$UNIT" 2>/dev/null || true)
	[[ "$$state" == disabled ]]
	echo '[통과] 부팅 실행 등록 해제'

unregister: owned
	@systemctl disable --now "$$UNIT"
	rm "/etc/systemd/system/$$UNIT"
	systemctl daemon-reload
	systemctl reset-failed "$$UNIT" 2>/dev/null || true
	[[ "$$(systemctl show "$$UNIT" -p LoadState --value)" == not-found ]]
	test ! -e "/etc/systemd/system/$$UNIT"
	echo '[통과] 설정 파일 제거 및 등록 해제'

# 매번 고유한 서비스 이름으로 실행하여 기존 서비스를 보호합니다.
# 의존 타깃을 나열하는 대신 재귀 make로 실행 순서를 명시합니다.
test: check
	@session_dir=$$(mktemp -d /run/week02-make-test-XXXXXX)
	service="$${session_dir##*/}.service"
	cleanup() {
	  result=$$?
	  trap - EXIT
	  if [[ -e "/etc/systemd/system/$$service" ]]; then
	    $(MAKE) --no-print-directory -f "$$TEST_MAKEFILE" UNIT="$$service" unload || {
	      printf '자동 정리 실패. 확인할 서비스: %s\n' "$$service" >&2
	      result=1
	    }
	  fi
	  rmdir "$$session_dir" || result=1
	  exit "$$result"
	}
	trap cleanup EXIT
	trap 'exit 130' INT
	trap 'exit 143' TERM
	for target in load enable start status stop disable unload; do
	  $(MAKE) --no-print-directory -f "$$TEST_MAKEFILE" UNIT="$$service" "$$target"
	done
	echo '전체 테스트 통과. 테스트 서비스는 제거했습니다. 재부팅은 수행하지 않았습니다.'
