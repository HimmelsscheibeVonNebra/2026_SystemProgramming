#!/usr/bin/env python3
from pathlib import Path
import subprocess, tempfile, sys, os
root = Path(__file__).resolve().parent
binary = root / (sys.argv[1] if len(sys.argv) > 1 else "file_copy")
def run(*args, expected=0):
    r = subprocess.run([str(binary), *map(str,args)], capture_output=True, timeout=5)
    assert r.returncode == expected, (args, r.returncode, r.stderr)
    return r
with tempfile.TemporaryDirectory(prefix="class03-02-") as td:
    p = Path(td); src = p / "in"
    datasets = [b"", b"A", b"A\0B\xff", "한글\n".encode()]
    datasets += [bytes(i % 256 for i in range(n)) for n in (4095,4096,4097,100003)]
    for i, data in enumerate(datasets):
        src.write_bytes(data); dest = p / f"out{i}"
        run(src, dest)
        assert dest.read_bytes() == data
    print("PASS empty, binary, UTF-8 and 4096-byte boundary copies")
    src.write_bytes(b"KEEP")
    run(src, src, expected=1); assert src.read_bytes() == b"KEEP"
    dest = p / "existing"; dest.write_bytes(b"ORIGINAL")
    run(src, dest, expected=1); assert dest.read_bytes() == b"ORIGINAL"
    os.link(src, p / "hardlink")
    run(src, p / "hardlink", expected=1); assert src.read_bytes() == b"KEEP"
    (p / "symlink").symlink_to(src)
    run(src, p / "symlink", expected=1); assert src.read_bytes() == b"KEEP"
    run(p / "missing", p / "not-created", expected=1)
    assert not (p / "not-created").exists()
    run(src, p / "missing-dir" / "out", expected=1)
    run(expected=2)
    print("PASS missing input, destination failure, existing file, aliases and usage")
    r = subprocess.run([str(root / "read_chunks"), str(src)], capture_output=True, timeout=5)
    assert r.returncode == 0 and r.stdout == b"KEEP"
    assert b"read returned 0" in r.stderr
    if sys.platform.startswith("linux") and Path("/dev/full").exists():
        with open("/dev/full", "wb", buffering=0) as full:
            r = subprocess.run([str(root / "read_chunks"), str(src)], stdout=full, stderr=subprocess.PIPE, timeout=5)
        assert r.returncode == 1
        print("PASS Linux /dev/full write failure")
    else:
        print("SKIP Linux /dev/full (use scripted ENOSPC test here)")
print("All copy tests passed")
