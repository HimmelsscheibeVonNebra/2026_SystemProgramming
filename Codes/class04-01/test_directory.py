"""임시 폴더에서 디렉토리 예제의 결과와 실패 시 데이터 보존을 확인한다."""

import os
from pathlib import Path
import stat
import subprocess
import tempfile
import unittest


BIN_DIR = Path(__file__).resolve().parent
PROGRAMS = ("dir_create", "dir_list", "dir_chdir", "dir_remove")


class DirectoryExamples(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="class04-directory-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve()

    def invoke(self, program, *args, expected=0, **kwargs):
        result = subprocess.run(
            [str(BIN_DIR / program), *map(str, args)],
            cwd=self.root, capture_output=True, text=True, **kwargs
        )
        self.assertEqual(result.returncode, expected, result.stderr)
        if expected == 0:
            self.assertEqual(result.stderr, "")
        else:
            self.assertTrue(result.stderr)
        return result

    def test_creation_umask_and_existing_data(self):
        target = self.root / "new directory"
        previous_umask = os.umask(0o027)
        try:
            self.invoke("dir_create", target)
        finally:
            os.umask(previous_umask)
        self.assertTrue(target.is_dir())
        self.assertEqual(stat.S_IMODE(target.stat().st_mode), 0o750)
        item = target / "keep.txt"
        item.write_text("keep me\n")
        self.invoke("dir_create", target, expected=1)
        self.invoke("dir_create", item, expected=1)
        self.assertEqual(item.read_text(), "keep me\n")
        self.invoke("dir_create", self.root / "absent/child", expected=1)
        self.assertFalse((self.root / "absent").exists())

    def test_listing_names_and_empty_directory(self):
        folder = self.root / "entries"
        folder.mkdir()
        empty = self.invoke("dir_list", folder)
        self.assertEqual(set(empty.stdout.splitlines()), {".", ".."})
        (folder / ".hidden").touch()
        (folder / "space name.txt").write_text("hello\n")
        (folder / "sub").mkdir()
        (folder / "broken").symlink_to("missing")
        result = self.invoke("dir_list", folder)
        self.assertEqual(
            set(result.stdout.splitlines()),
            {".", "..", ".hidden", "space name.txt", "sub", "broken"}
        )
        self.invoke("dir_list", folder / "space name.txt", expected=1)
        self.invoke("dir_list", folder / "missing", expected=1)

    def test_change_directory_is_local_to_child(self):
        target = self.root / "child directory"
        target.mkdir()
        parent_cwd = Path.cwd()
        result = self.invoke("dir_chdir", "child directory")
        self.assertEqual(result.stdout.splitlines(), [
            f"before: {self.root}", f"after: {target}"
        ])
        self.assertEqual(Path.cwd(), parent_cwd)
        self.invoke("dir_chdir", "missing", expected=1)
        item = self.root / "file.txt"
        item.touch()
        self.invoke("dir_chdir", item, expected=1)

    def test_removal_preserves_nonempty_directory_and_symlink(self):
        folder = self.root / "remove me"
        folder.mkdir()
        item = folder / ".hidden"
        item.write_text("do not delete\n")
        self.invoke("dir_remove", folder, expected=1)
        self.assertEqual(item.read_text(), "do not delete\n")
        self.invoke("dir_remove", item, expected=1)
        item.unlink()
        link = self.root / "directory-link"
        link.symlink_to(folder, target_is_directory=True)
        self.invoke("dir_remove", link, expected=1)
        self.assertTrue(link.is_symlink())
        self.assertTrue(folder.is_dir())
        self.invoke("dir_remove", folder)
        self.assertFalse(folder.exists())
        self.invoke("dir_remove", folder, expected=1)

    @unittest.skipIf(os.geteuid() == 0, "권한 검사는 일반 사용자로 실행")
    def test_permission_errors(self):
        locked = self.root / "locked"
        locked.mkdir()
        locked.chmod(0)
        try:
            self.invoke("dir_create", locked / "child", expected=1)
            self.invoke("dir_list", locked, expected=1)
            self.invoke("dir_chdir", locked, expected=1)
        finally:
            locked.chmod(0o700)

    def test_usage(self):
        for program in PROGRAMS:
            with self.subTest(program=program):
                self.invoke(program, expected=2)
                self.invoke(program, "one", "two", expected=2)


if __name__ == "__main__":
    unittest.main(verbosity=2)
