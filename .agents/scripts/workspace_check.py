#!/usr/bin/env python3
"""Deterministic workspace, documentation, and PR hygiene checker.

Provides sub-second local verification for:
- `status`: Git worktrees, clean working trees, `.git/info/exclude`, junctions, and deps.
- `docs`: Tracking-doc size caps, MALI_PROGRESS ToC parity, UTF-8/English-only checks,
  privacy/leak guards, and zero `__pycache__` verification.
- `pr`: Pre-PR cleanliness, `git merge-tree --write-tree`, forbidden paths, protected
  files, and English-only commits/diffs.
"""

from __future__ import annotations

import argparse
import os
import pathlib
import re
import shutil
import stat
import subprocess
import sys

SCRIPT_DIR = pathlib.Path(__file__).resolve().parent
AGENTS_DIR = SCRIPT_DIR.parent
REPO_ROOT = AGENTS_DIR.parent

CYRILLIC_RE = re.compile(r"[\u0400-\u04FF]")
WIN_USER_PATH_RE = re.compile(r"[a-zA-Z]:[/\\]Users[/\\](?!Public\b|Default\b)[A-Za-z0-9_.-]+", re.IGNORECASE)

SIZE_CAPS = {
    "AGENTS.md": 24000,
    "MALI_STATUS.md": 4500,
    "MALI_PLAN.md": 16000,
    "MALI_PROGRESS.md": 80000,
}

DOC_FILES = [
    "AGENTS.md",
    "MALI_STATUS.md",
    "MALI_PLAN.md",
    "MALI_PROGRESS.md",
]

REQUIRED_EXCLUDES = (
    "/.agents/",
    "/.mali/",
    "/AGENTS.md",
    "/GEMINI.md",
)

FORBIDDEN_PR_PATTERNS = (
    re.compile(r"(?:^|/)(?:Dump|extracted)(?:/|$)", re.IGNORECASE),
    re.compile(r"(?:^|/)generated", re.IGNORECASE),
    re.compile(r"\.bfttf$", re.IGNORECASE),
    re.compile(r"(?:^|/)(?:\.mali|\.agents)(?:/|$)", re.IGNORECASE),
    re.compile(r"(?:^|/)(?:AGENTS\.md|GEMINI\.md|MALI_[^/]*\.md)$", re.IGNORECASE),
)

PROTECTED_PR_FILES = {
    "tools/patches.py",
    "runtime/gamelib.h",
    ".gitignore",
}


def run_git(repo: pathlib.Path, args: list[str], check: bool = True) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", "-C", str(repo), *args],
        text=True,
        encoding="utf-8",
        errors="replace",
        capture_output=True,
        check=check,
    )


def is_junction_or_symlink(path: pathlib.Path) -> bool:
    if hasattr(os.path, "isjunction") and os.path.isjunction(path):
        return True
    if os.path.islink(path):
        return True
    st = os.lstat(path)
    attrs = getattr(st, "st_file_attributes", 0)
    reparse = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)
    return bool(attrs & reparse)


def parse_worktrees(root: pathlib.Path) -> list[dict[str, str]]:
    res = run_git(root, ["worktree", "list", "--porcelain"])
    entries: list[dict[str, str]] = []
    cur: dict[str, str] = {}
    for line in res.stdout.splitlines():
        if not line.strip():
            if cur:
                entries.append(cur)
                cur = {}
            continue
        key, _, val = line.partition(" ")
        cur[key] = val
    if cur:
        entries.append(cur)
    return entries


def find_adb() -> str | None:
    found = shutil.which("adb")
    if found:
        return found
    for env_key in ("ANDROID_HOME", "ANDROID_SDK_ROOT", "LOCALAPPDATA"):
        base = os.environ.get(env_key)
        if not base:
            continue
        cand = (
            pathlib.Path(base) / "Android" / "Sdk" / "platform-tools" / "adb.exe"
            if env_key == "LOCALAPPDATA"
            else pathlib.Path(base) / "platform-tools" / ("adb.exe" if os.name == "nt" else "adb")
        )
        if cand.is_file():
            return str(cand)
    return None


def get_connected_adb_serials() -> list[str]:
    adb = find_adb()
    if not adb:
        return []
    try:
        proc = subprocess.run(
            [adb, "devices"],
            text=True,
            encoding="utf-8",
            errors="replace",
            capture_output=True,
            timeout=3,
        )
    except Exception:
        return []
    if proc.returncode != 0:
        return []
    serials: list[str] = []
    for line in proc.stdout.splitlines():
        line = line.strip()
        if not line or line.startswith("List of devices"):
            continue
        parts = line.split()
        if len(parts) >= 2 and len(parts[0]) >= 6:
            serials.append(parts[0])
    return serials


def build_privacy_patterns() -> list[tuple[str, re.Pattern[str]]]:
    patterns: list[tuple[str, re.Pattern[str]]] = [
        ("cyrillic", CYRILLIC_RE),
        ("local-user-path", WIN_USER_PATH_RE),
    ]
    home_name = pathlib.Path.home().name
    if home_name and home_name.lower() not in ("public", "default", "user", "users"):
        patterns.append(
            ("home-user-path", re.compile(r"Users[/\\]" + re.escape(home_name), re.IGNORECASE))
        )
    for serial in get_connected_adb_serials():
        patterns.append(
            ("connected-adb-serial", re.compile(r"\b" + re.escape(serial) + r"\b"))
        )
    return patterns


def cmd_status() -> int:
    errors: list[str] = []
    worktrees = parse_worktrees(REPO_ROOT)

    shaderc = REPO_ROOT / "android" / "deps" / "out" / "lib" / "libshaderc_shared.so"
    shaderc_ok = shaderc.is_file()
    if not shaderc_ok:
        errors.append(f"Missing root native dep: {shaderc}")

    print(f"root_libshaderc={'OK' if shaderc_ok else 'MISSING'} worktrees={len(worktrees)}")
    for entry in worktrees:
        wt_path = pathlib.Path(entry["worktree"])
        head = entry.get("HEAD", "")[:7]
        branch_raw = entry.get("branch", "detached")
        branch = branch_raw.removeprefix("refs/heads/")

        st_proc = run_git(wt_path, ["status", "-s", "-uall"])
        dirty_lines = [l for l in st_proc.stdout.splitlines() if l.strip()]
        dirty_state = f"dirty({len(dirty_lines)})" if dirty_lines else "clean"
        if dirty_lines:
            errors.append(f"Dirty worktree {wt_path}: {dirty_lines[:5]}")

        excl_proc = run_git(wt_path, ["rev-parse", "--git-path", "info/exclude"])
        excl_raw = pathlib.Path(excl_proc.stdout.strip())
        excl_path = excl_raw if excl_raw.is_absolute() else (wt_path / excl_raw)
        excl_ok = False
        if excl_path.is_file():
            excl_text = excl_path.read_text(encoding="utf-8", errors="replace")
            excl_ok = all(pat in excl_text for pat in REQUIRED_EXCLUDES)
        if not excl_ok:
            errors.append(f"Incomplete or missing exclude file for {wt_path}: {excl_path}")

        is_root = wt_path.resolve() == REPO_ROOT.resolve()
        deps_out = wt_path / "android" / "deps" / "out"
        if is_root:
            junc_state = "root"
        elif not deps_out.exists():
            junc_state = "none"
        else:
            is_junc = is_junction_or_symlink(deps_out)
            target_ok = (deps_out / "lib" / "libshaderc_shared.so").is_file()
            if is_junc and target_ok:
                junc_state = "junction-ok"
            else:
                junc_state = "invalid-dir" if not is_junc else "broken-target"
                errors.append(f"Linked worktree {wt_path} android/deps/out is {junc_state}")

        print(f"  {wt_path} [{branch} @ {head}] status={dirty_state} exclude={'OK' if excl_ok else 'BAD'} deps={junc_state}")

    if errors:
        for err in errors:
            print(f"FAIL: {err}", file=sys.stderr)
        return 1
    print("PASS: workspace status clean")
    return 0


def check_progress_toc(progress_text: str, errors: list[str]) -> tuple[int, int]:
    toc: list[str] = []
    headings: list[str] = []
    in_toc = False
    for line in progress_text.splitlines():
        if line == "## Table of Contents":
            in_toc = True
            continue
        if in_toc:
            if line.startswith("---"):
                in_toc = False
                continue
            if line.startswith("- "):
                toc.append(line[2:].strip())
        elif line.startswith("## "):
            headings.append(line[3:].strip())

    if not toc:
        errors.append("MALI_PROGRESS.md: empty or missing Table of Contents")
    elif toc != headings:
        errors.append(
            f"MALI_PROGRESS.md: ToC/heading mismatch (toc={len(toc)}, headings={len(headings)})"
        )
        for idx in range(max(len(toc), len(headings))):
            t_item = toc[idx] if idx < len(toc) else "<missing>"
            h_item = headings[idx] if idx < len(headings) else "<missing>"
            if t_item != h_item:
                errors.append(f"  first mismatch at #{idx + 1}: toc={t_item!r} vs heading={h_item!r}")
                break
    return len(toc), len(headings)


def find_pycache_dirs() -> list[pathlib.Path]:
    scan_roots: list[pathlib.Path] = [REPO_ROOT / ".agents", REPO_ROOT / "tools"]
    for wt in parse_worktrees(REPO_ROOT):
        wt_path = pathlib.Path(wt["worktree"])
        if wt_path.resolve() != REPO_ROOT.resolve():
            scan_roots.append(wt_path / "tools")
            scan_roots.append(wt_path / ".agents")

    found: list[pathlib.Path] = []
    for base in scan_roots:
        if not base.exists():
            continue
        for root_str, dirs, files in os.walk(base):
            if "__pycache__" in dirs:
                found.append(pathlib.Path(root_str) / "__pycache__")
            for fname in files:
                if fname.endswith(".pyc"):
                    found.append(pathlib.Path(root_str) / fname)
    return found


def cmd_docs() -> int:
    errors: list[str] = []
    patterns = build_privacy_patterns()

    files_to_scan: list[pathlib.Path] = [REPO_ROOT / name for name in DOC_FILES]
    files_to_scan.extend(
        sorted(p for p in (REPO_ROOT / ".agents").rglob("*") if p.is_file() and "__pycache__" not in p.parts)
    )

    progress_text = ""
    sizes: list[str] = []
    for path in files_to_scan:
        if not path.is_file():
            errors.append(f"Missing expected file: {path}")
            continue
        rel = path.relative_to(REPO_ROOT).as_posix()
        raw = path.read_bytes()
        if rel in SIZE_CAPS:
            cap = SIZE_CAPS[rel]
            sizes.append(f"{rel}={len(raw)}/{cap}B")
            if len(raw) > cap:
                errors.append(f"Size cap exceeded for {rel}: {len(raw)} > {cap} bytes")
        try:
            text = raw.decode("utf-8")
        except UnicodeDecodeError as exc:
            errors.append(f"Invalid UTF-8 in {rel}: {exc}")
            continue
        if rel == "MALI_PROGRESS.md":
            progress_text = text

        for lineno, line in enumerate(text.splitlines(), 1):
            for label, pat in patterns:
                if pat.search(line):
                    errors.append(f"{rel}:{lineno}: matched {label}")

    toc_cnt, head_cnt = check_progress_toc(progress_text, errors) if progress_text else (0, 0)

    pycache_hits = find_pycache_dirs()
    for p in pycache_hits:
        errors.append(f"Bytecode artifact found: {p}")

    print(
        f"scanned={len(files_to_scan)} toc={toc_cnt}/{head_cnt} pycache={len(pycache_hits)} "
        + " ".join(sizes)
    )
    if errors:
        for err in errors:
            print(f"FAIL: {err}", file=sys.stderr)
        return 1
    print("PASS: docs, .agents/, ToC parity, and bytecode checks clean")
    return 0


def cmd_pr(worktree_arg: str, base: str, allow_protected: bool) -> int:
    wt = pathlib.Path(worktree_arg)
    if not wt.is_absolute():
        cand_cwd = pathlib.Path.cwd() / wt
        cand_root = REPO_ROOT / wt
        wt = cand_cwd if cand_cwd.exists() else cand_root
    wt = wt.resolve()
    if not wt.is_dir():
        print(f"FAIL: Worktree directory does not exist: {wt}", file=sys.stderr)
        return 1

    errors: list[str] = []

    st_proc = run_git(wt, ["status", "-s", "-uall"])
    dirty_lines = [l for l in st_proc.stdout.splitlines() if l.strip()]
    if dirty_lines:
        errors.append(f"Working tree is dirty ({len(dirty_lines)} entries): {dirty_lines[:5]}")

    mt_proc = run_git(wt, ["merge-tree", "--write-tree", base, "HEAD"], check=False)
    mergeable = mt_proc.returncode == 0
    if not mergeable:
        detail = (mt_proc.stdout + "\n" + mt_proc.stderr).strip().splitlines()
        errors.append(f"merge-tree conflict vs {base}: {'; '.join(detail[:6])}")

    cnt_proc = run_git(wt, ["rev-list", "--count", f"{base}..HEAD"], check=False)
    commit_cnt = cnt_proc.stdout.strip() if cnt_proc.returncode == 0 else "?"

    diff_names = run_git(wt, ["diff", "--name-status", f"{base}...HEAD"])
    changed_files: list[str] = []
    for line in diff_names.stdout.splitlines():
        if not line.strip():
            continue
        parts = line.split("\t")
        for p in parts[1:]:
            changed_files.append(p)
            for pat in FORBIDDEN_PR_PATTERNS:
                if pat.search(p):
                    errors.append(f"Forbidden PR path ({pat.pattern}): {p}")
            if not allow_protected and p in PROTECTED_PR_FILES:
                errors.append(f"Protected file modified without --allow-protected: {p}")

    log_proc = run_git(wt, ["log", "--format=%H %B", f"{base}..HEAD"])
    for lineno, line in enumerate(log_proc.stdout.splitlines(), 1):
        if CYRILLIC_RE.search(line):
            errors.append(f"Cyrillic found in commit message line {lineno}")

    diff_proc = run_git(wt, ["diff", "-U0", f"{base}...HEAD"])
    cur_file = "<unknown>"
    for line in diff_proc.stdout.splitlines():
        if line.startswith("+++ b/"):
            cur_file = line[6:]
            continue
        if line.startswith("+") and not line.startswith("+++"):
            if cur_file != "runtime/ui/i18n_ru.inc" and CYRILLIC_RE.search(line):
                errors.append(f"Cyrillic found in added diff line ({cur_file})")

    print(
        f"worktree={wt.name} base={base} commits={commit_cnt} files={len(changed_files)} "
        f"mergeable={'YES' if mergeable else 'CONFLICT'}"
    )
    if errors:
        for err in errors:
            print(f"FAIL: {err}", file=sys.stderr)
        return 1
    print("PASS: PR branch clean and mergeable")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Deterministic BOTWRecompAndroid-Mali workspace checks.")
    sub = parser.add_subparsers(dest="subcommand", required=True)
    sub.add_parser("status", help="Check worktrees, excludes, junctions, and native deps.")
    sub.add_parser("docs", help="Check doc budgets, MALI_PROGRESS ToC parity, leaks, and pycache.")
    pr_p = sub.add_parser("pr", help="Audit a PR worktree for cleanliness and mergeability.")
    pr_p.add_argument("worktree", help="Path to the PR worktree.")
    pr_p.add_argument("--base", default="upstream/main", help="Base ref to check against (default: upstream/main).")
    pr_p.add_argument("--allow-protected", action="store_true", help="Allow changes to protected files.")
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    if args.subcommand == "status":
        return cmd_status()
    if args.subcommand == "docs":
        return cmd_docs()
    if args.subcommand == "pr":
        return cmd_pr(args.worktree, args.base, args.allow_protected)
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
