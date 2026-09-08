#!/usr/bin/env bash
set -eu
lab_dir=$(mktemp -d "./run-XXXXXX")
mkdir -p "$lab_dir/data" "$lab_dir/web" "$lab_dir/logs"
printf 'alpha\nbeta\nalpha\n' > "$lab_dir/data/items.txt"
printf 'Hello Linux lab!\n' > "$lab_dir/web/index.html"
cat > "$lab_dir/logs/auth-demo.log" <<'LOG'
Sep 07 09:00:00 lab sshd[101]: Accepted publickey for student from 192.0.2.10 port 50101 ssh2
Sep 07 09:01:00 lab sshd[102]: Failed password for invalid user guest from 198.51.100.23 port 50102 ssh2
Sep 07 09:01:05 lab sshd[103]: Failed password for invalid user guest from 198.51.100.23 port 50103 ssh2
Sep 07 09:02:00 lab sshd[104]: Accepted password for student from 198.51.100.23 port 50104 ssh2
LOG
printf '%s\n' "$lab_dir"
