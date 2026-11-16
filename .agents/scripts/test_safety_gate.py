#!/usr/bin/env python3
"""Unit tests for .agents/scripts/safety_gate.py PreToolUse hook decisions."""

from __future__ import annotations

import json
import pathlib
import subprocess
import sys
import unittest

SCRIPT_DIR = pathlib.Path(__file__).resolve().parent
AGENTS_DIR = SCRIPT_DIR.parent
ROOT_CWD = r"c:\workspace\BOTWRecompAndroid-Mali"
WT_CWD = r"c:\workspace\BOTWRecompAndroid-Mali\.mali\worktrees\fix-rt-load-upload-296"


def run_gate(payload: dict) -> dict:
    proc = subprocess.run(
        [sys.executable, "scripts/safety_gate.py"],
        cwd=str(AGENTS_DIR),
        input=json.dumps(payload),
        text=True,
        capture_output=True,
        check=True,
    )
    return json.loads(proc.stdout.strip())


def run_cmd_payload(cmd: str, cwd: str = ROOT_CWD) -> dict:
    return {
        "hookEventName": "PreToolUse",
        "cwd": cwd,
        "toolCall": {
            "id": "t1",
            "name": "run_command",
            "args": {"CommandLine": cmd},
        },
    }


def write_payload(tool: str, path: str) -> dict:
    return {
        "hookEventName": "PreToolUse",
        "cwd": ROOT_CWD,
        "toolCall": {
            "id": "t2",
            "name": tool,
            "args": {"Path": path},
        },
    }


class SafetyGateTests(unittest.TestCase):
    def test_allows_readonly_quoted_prose_and_test_commands(self) -> None:
        for cmd in [
            "git status -s",
            "git diff HEAD~1",
            "gh release list -R mamcjr/BOTWRecompAndroid -L 5",
            "gh pr view 461 -R mamcjr/BOTWRecompAndroid",
            "python tools/test_vulkan_compat.py",
            "git clean -nd path-foo",
            "git checkout -b feature/new-fix",
            "git checkout -b v0.4.5",
            'rg "adb uninstall com.botwrecomp.android" AGENTS.md',
            'python -c "print(\'git push upstream && rm -rf .mali/user-backup\')"',
            "adb uninstall com.botwrecomp.android.waterdiag",
            "cmd /c rmdir .mali\\worktrees\\old\\android\\deps\\out",
            "rm -rf .mali/user-backup-notes\nWrite-Host ok",
        ]:
            res = run_gate(run_cmd_payload(cmd))
            self.assertEqual(res["decision"], "allow", f"Expected allow for: {cmd} (got {res})")

    def test_denies_destructive_git_including_flag_variants(self) -> None:
        for cmd in [
            "git reset --hard HEAD",
            "git --no-pager reset --hard",
            "git.exe reset --hard",
            "git clean -fd",
            "git clean -fxd",
            "git clean --force",
            "git stash clear",
            "git push --force origin main",
            "git push --force origin mali-workspace",
            "git push -f origin main",
            "git push origin +mali-workspace",
            "git push --force-with-lease",
            "git push --force-with-lease origin",
            "git push --force-with-lease origin main",
            "git push --force-with-lease origin HEAD:main",
            "git push --force-with-lease origin HEAD",
            "git push --force-with-lease upstream mali-workspace",
            "git push --delete origin main",
            "git push upstream main",
            "git push --repo=upstream main",
        ]:
            res = run_gate(run_cmd_payload(cmd))
            self.assertEqual(res["decision"], "deny", f"Expected deny for: {cmd} (got {res})")

    def test_denies_stock_save_wiping_adb(self) -> None:
        for cmd in [
            "adb -s TESTSERIAL01 uninstall com.botwrecomp.android",
            "adb shell pm clear com.botwrecomp.android",
            'adb -s TESTSERIAL01 shell "pm clear com.botwrecomp.android"',
            'adb -s TESTSERIAL01 shell su -c "pm clear com.botwrecomp.android"',
            'adb -s TESTSERIAL01 shell \'su -c "pm clear com.botwrecomp.android"\'',
            'adb -s TESTSERIAL01 shell "su -c \'pm clear com.botwrecomp.android\'"',
            "adb -s TESTSERIAL01 shell pm uninstall com.botwrecomp.android",
        ]:
            res = run_gate(run_cmd_payload(cmd))
            self.assertEqual(res["decision"], "deny", f"Expected deny for: {cmd} (got {res})")

    def test_denies_deletion_aliases_and_reordered_flags(self) -> None:
        for cmd in [
            "Remove-Item -Recurse -Force .mali\\worktrees\\fix-rt-load-upload-296",
            "Remove-Item -Recurse .mali",
            "cmd /c rd /s /q .mali",
            "rm -rf .mali",
            "Remove-Item -Recurse android\\deps",
            "cmd /c rd /s /q android\\deps",
            "Remove-Item -Recurse .",
            "ri -Recurse -Force .mali\\worktrees\\fix-1",
            "rm -Recurse -Force android\\deps\\out",
            "rm.exe -rf .mali\\worktrees\\fix-rt-load-upload-296",
            "Remove-Item -Recurse -Force .mali\\user-backup\\2026-11-16",
            f"Remove-Item -Recurse {ROOT_CWD}\\.mali\\tmp\\..\\user-backup\\2026-11-16",
            "rm -f -r .mali/user-backup/2026-11-16",
            "cmd /c rd /s /q .mali\\user-backup\\2026-11-16",
            "cmd /c rd /q /s .mali\\user-backup\\2026-11-16",
            "cmd /c rd /s/q .mali\\user-backup\\2026-11-16",
            f"cmd /c rd /s /q {ROOT_CWD}\\.mali\\tmp\\..\\user-backup\\2026-11-16",
            f"cmd /c rd /s /q {ROOT_CWD}\\android\\deps\\tmp\\..\\out",
            f'cmd /c rd /s /q "\\\\?\\{ROOT_CWD}\\.mali\\user-backup\\2026-11-16"',
            'powershell -Command "Remove-Item -Recurse .mali\\worktrees\\x"',
        ]:
            res = run_gate(run_cmd_payload(cmd))
            self.assertEqual(res["decision"], "deny", f"Expected deny for: {cmd} (got {res})")

    def test_force_asks_external_mutations_and_discards(self) -> None:
        for cmd in [
            "git push origin mali-workspace",
            "git push origin fix/upstream-sync",
            "git push --force-with-lease origin mali-workspace",
            "git push --force-with-lease=mali-workspace:deadbeef origin mali-workspace",
            "git push --force-with-lease origin pr315-benchmark-v2:benchmark-v2",
            "gh pr create --title test --body test",
            "gh issue comment 296 --body hi",
            "git checkout -- runtime/nvn/vulkan/exec.c",
            "git checkout runtime/nvn/vulkan/exec.c",
            "git checkout . && echo ok",
            "git restore runtime/main.c",
            "git stash drop stash@{0}",
        ]:
            res = run_gate(run_cmd_payload(cmd))
            self.assertEqual(res["decision"], "force_ask", f"Expected force_ask for: {cmd} (got {res})")

    def test_commit_worktree_scoping_and_traversal_guards(self) -> None:
        # Allowed in genuine feature worktree (even with quoted prose or -c before -C)
        for cmd, cwd in [
            ("git -C .mali/worktrees/fix-rt-load-upload-296 commit -m 'fix'", ROOT_CWD),
            ("git -c user.name=x -C .mali/worktrees/fix-1 commit -m 'test'", ROOT_CWD),
            ('git commit -m "never git push upstream"', WT_CWD),
        ]:
            res = run_gate(run_cmd_payload(cmd, cwd=cwd))
            self.assertEqual(res["decision"], "allow", f"Expected allow for: {cmd} (got {res})")

        # Must force_ask when committing on root or escaping worktree via '..' or chaining
        for cmd, cwd in [
            ("git commit -m 'local wip'", ROOT_CWD),
            ("git -C .mali/worktrees/fix-1 status && git commit -m root", ROOT_CWD),
            ("git -C .mali/worktrees/fix-1/../../.. commit -m root", ROOT_CWD),
            ("git --git-dir=.git commit -m root", WT_CWD),
            ("git --work-tree=.mali/worktrees/x commit -m root", ROOT_CWD),
        ]:
            res = run_gate(run_cmd_payload(cmd, cwd=cwd))
            self.assertEqual(res["decision"], "force_ask", f"Expected force_ask for: {cmd} (got {res})")

    def test_write_tool_protections(self) -> None:
        for denied in [
            f"{ROOT_CWD}\\Dump\\test.bin",
            f"{ROOT_CWD}\\.mali\\user-backup\\2026-11-16\\mali\\stock-v190.sha256",
            f"\\\\?\\{ROOT_CWD}\\.mali\\user-backup\\2026-11-16\\test.sha256",
            ".mali/user-backup/2026-11-16/test.sha256",
        ]:
            res = run_gate(write_payload("write_file", denied))
            self.assertEqual(res["decision"], "deny", f"Expected deny for: {denied} (got {res})")

        for protected in [
            f"{ROOT_CWD}\\tools\\patches.py",
            "tools/patches.py",
            f"{ROOT_CWD}\\runtime\\gamelib.h",
            f"{ROOT_CWD}\\.gitignore",
        ]:
            res = run_gate(write_payload("edit_file", protected))
            self.assertEqual(res["decision"], "force_ask", f"Expected force_ask for: {protected} (got {res})")

        for normal in [
            f"{ROOT_CWD}\\MALI_STATUS.md",
            f"{ROOT_CWD}\\.git\\info\\exclude",
            f"{ROOT_CWD}\\runtime\\nvn\\vulkan\\exec.c",
        ]:
            res = run_gate(write_payload("edit_file", normal))
            self.assertEqual(res["decision"], "allow", f"Expected allow for: {normal} (got {res})")

    def test_windows_cmd_invocation_from_agents_dir(self) -> None:
        proc = subprocess.run(
            ["cmd", "/c", "python scripts/safety_gate.py"],
            cwd=str(AGENTS_DIR),
            input=json.dumps(run_cmd_payload("git status")),
            text=True,
            capture_output=True,
            check=True,
        )
        self.assertEqual(json.loads(proc.stdout.strip())["decision"], "allow")


if __name__ == "__main__":
    unittest.main(verbosity=2)
