#!/usr/bin/env bash
# Ubuntu의 systemd 서비스 등록과 해제를 확인하는 독립 실행 스크립트입니다.
# 실행: sudo bash test-service.sh
set -euo pipefail

if [[ $EUID -ne 0 ]]; then
    printf 'sudo bash %q 로 실행하세요.\n' "$0" >&2
    exit 1
fi
if [[ $(ps -p 1 -o comm=) != systemd ]]; then
    printf 'PID 1이 systemd인 Ubuntu에서 실행하세요.\n' >&2
    exit 1
fi
for tool in systemctl mktemp install sleep; do
    command -v "$tool" >/dev/null || {
        printf '필요한 명령이 없습니다: %s\n' "$tool" >&2
        exit 1
    }
done

# 고유한 이름으로 기존 수업 서비스와 충돌하지 않게 합니다.
test_dir=$(mktemp -d /run/week02-service-test.XXXXXX)
unit="$(basename "$test_dir").service"
unit_path="/etc/systemd/system/$unit"
owned_unit=0

cleanup() {
    local failed=0
    if (( owned_unit )); then
        systemctl disable --now "$unit" || failed=1
        # 중지/등록 해제에 실패하면 설정 파일을 남겨 수동 복구가 가능하게 합니다.
        if (( failed == 0 )); then
            rm -f "$unit_path" || failed=1
            systemctl daemon-reload || failed=1
            systemctl reset-failed "$unit" 2>/dev/null || true
        fi
    fi
    rm -f "$test_dir/$unit" || failed=1
    rmdir "$test_dir" || failed=1
    return "$failed"
}

on_exit() {
    local result=$?
    trap - EXIT
    if ! cleanup; then
        printf '자동 정리에 실패했습니다. 서비스와 경로를 확인하세요:\n%s\n%s\n' \
            "$unit" "$unit_path" >&2
        result=1
    fi
    if (( result != 0 )); then
        printf '테스트 실패: 위 오류 메시지를 확인하세요.\n' >&2
    fi
    exit "$result"
}
trap on_exit EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

if [[ -e $unit_path || -L $unit_path ]]; then
    printf '같은 이름의 설정이 이미 존재합니다: %s\n' "$unit_path" >&2
    exit 1
fi

# 별도 C 빌드 없이 서비스의 등록/해제 자체를 테스트합니다.
# sleep은 최대 300초간 대기하며 테스트 중 stop으로 종료합니다.
sleep_path=$(command -v sleep)
cat > "$test_dir/$unit" <<EOF
[Unit]
Description=Temporary Week02 service lifecycle test

[Service]
Type=simple
ExecStart=$sleep_path 300
DynamicUser=yes

[Install]
WantedBy=multi-user.target
EOF

printf '테스트 서비스: %s\n' "$unit"
owned_unit=1
install -m 0644 "$test_dir/$unit" "$unit_path"
systemctl daemon-reload
[[ $(systemctl show "$unit" -p LoadState --value) == loaded ]]
printf '[통과] 설정 파일 등록 및 로드\n'

systemctl enable "$unit"
systemctl is-enabled --quiet "$unit"
printf '[통과] 부팅 시 실행 등록(enable)\n'

systemctl start "$unit"
sleep 1
systemctl is-active --quiet "$unit"
pid=$(systemctl show "$unit" -p MainPID --value)
[[ $pid =~ ^[0-9]+$ && $pid -gt 0 ]]
printf '[통과] 서비스 시작: active, PID=%s\n' "$pid"

systemctl stop "$unit"
[[ $(systemctl show "$unit" -p ActiveState --value) == inactive ]]
[[ $(systemctl show "$unit" -p MainPID --value) == 0 ]]
printf '[통과] 서비스 중지: inactive, PID=0\n'

systemctl disable "$unit"
# disabled에서는 is-enabled가 0이 아닌 종료값을 반환하므로 출력으로 비교합니다.
enabled_state=$(systemctl is-enabled "$unit" 2>/dev/null || true)
[[ $enabled_state == disabled ]]
printf '[통과] 부팅 시 실행 등록 해제(disable)\n'

rm "$unit_path"
systemctl daemon-reload
systemctl reset-failed "$unit" 2>/dev/null || true
[[ ! -e $unit_path && ! -L $unit_path ]]
[[ $(systemctl show "$unit" -p LoadState --value) == not-found ]]
owned_unit=0
printf '[통과] 설정 파일 제거 및 등록 해제 확인\n'
printf '모든 서비스 테스트를 통과했습니다. 재부팅 자체는 수행하지 않았습니다.\n'
