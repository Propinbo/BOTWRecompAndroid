#!/usr/bin/env python3
"""Antigravity PreToolUse safety gate for BOTWRecompAndroid-Mali.

Parses commands into shell statements and tokens so quoted prose, search patterns,
and inline scripts do not trigger false positives, while git flag/path/chaining
variants and PowerShell deletion aliases are checked accurately.
"""

from __future__ import annotations

import json
import os
import posixpath
import re
import sys
from typing import Any


WRITE_TOOLS = {
    "write_file",
    "edit_file",
    "write_to_file",
    "replace_file_content",
    "multi_replace_file_content",
}

GIT_VAL_GLOBAL_OPTS = {
    "-C",
    "-c",
    "--git-dir",
    "--work-tree",
    "--namespace",
    "--super-prefix",
    "--config-env",
    "--exec-path",
}

GIT_PUSH_VAL_OPTS = {"--repo", "--receive-pack", "--exec", "-o", "--push-option"}

ADB_VAL_GLOBAL_OPTS = {"-s", "-t", "-H", "-P", "-L"}


def normalize_path(path: str) -> str:
    p = path.replace("\\", "/").strip()
    return re.sub(r"^//\?/", "", p)


def decision(kind: str, reason: str | None = None) -> dict[str, Any]:
    out: dict[str, Any] = {"decision": kind}
    if reason:
        out["reason"] = reason
    return out


def split_statements(cmd: str) -> list[list[str]]:
    """Split a shell command string into unquoted statements of unquoted tokens."""
    statements: list[list[str]] = []
    tokens: list[str] = []
    buf: list[str] = []
    in_token = False
    quote: str | None = None
    i = 0
    n = len(cmd)

    def flush_tok() -> None:
        nonlocal in_token
        if in_token:
            tokens.append("".join(buf))
            buf.clear()
            in_token = False

    def flush_stmt() -> None:
        flush_tok()
        if tokens:
            statements.append(tokens.copy())
            tokens.clear()

    while i < n:
        ch = cmd[i]
        if quote is not None:
            if ch == quote:
                quote = None
            elif quote == '"' and ch == "\\" and i + 1 < n and cmd[i + 1] in ('"', "\\"):
                i += 1
                buf.append(cmd[i])
            else:
                buf.append(ch)
            i += 1
            continue

        if ch in ('"', "'"):
            quote = ch
            in_token = True
            i += 1
        elif ch in (" ", "\t", "\r"):
            flush_tok()
            i += 1
        elif ch == "\n":
            flush_stmt()
            i += 1
        elif ch in (";", "|", "&"):
            flush_stmt()
            if i + 1 < n and cmd[i + 1] == ch and ch in ("|", "&"):
                i += 2
            else:
                i += 1
        else:
            buf.append(ch)
            in_token = True
            i += 1

    flush_stmt()
    return statements


def unwrap_shell_wrappers(tokens: list[str]) -> list[list[str]]:
    """Unwrap cmd /c ... and powershell -Command ... into inner statements."""
    if not tokens:
        return []
    exe = os.path.basename(normalize_path(tokens[0])).lower()
    if exe in ("cmd", "cmd.exe"):
        for idx, tok in enumerate(tokens[1:], start=1):
            if tok.lower() in ("/c", "/k"):
                rest = tokens[idx + 1 :]
                if len(rest) == 1 and any(c in rest[0] for c in (" ", ";", "&", "|")):
                    return split_statements(rest[0])
                return [rest] if rest else []
    if exe in ("powershell", "powershell.exe", "pwsh", "pwsh.exe"):
        for idx, tok in enumerate(tokens[1:], start=1):
            if tok.lower() in ("-c", "-command"):
                rest = tokens[idx + 1 :]
                if len(rest) == 1:
                    return split_statements(rest[0])
                return [rest] if rest else []
    return [tokens]


def resolve_git_dir(cwd: str, c_args: list[str]) -> str:
    cur = normalize_path(cwd)
    for c_arg in c_args:
        cand = normalize_path(c_arg)
        if not cand:
            continue
        if re.match(r"^[a-zA-Z]:/", cand) or cand.startswith("/"):
            cur = cand
        else:
            cur = posixpath.join(cur, cand) if cur else cand
    # Preserve drive prefix like c:/ cleanly
    m = re.match(r"^([a-zA-Z]:)(/.*)$", cur)
    if m:
        return m.group(1) + posixpath.normpath(m.group(2))
    return posixpath.normpath(cur) if cur else ""


def is_inside_feature_worktree(resolved_dir: str) -> bool:
    norm = normalize_path(resolved_dir).rstrip("/")
    parts = [p for p in norm.split("/") if p]
    for i in range(len(parts) - 2):
        if parts[i] == ".mali" and parts[i + 1] == "worktrees" and parts[i + 2]:
            return True
    return False


def has_short_flag(tok: str, letter: str) -> bool:
    return tok.startswith("-") and not tok.startswith("--") and len(tok) > 1 and letter in tok[1:]


def check_git_statement(tokens: list[str], cwd: str) -> dict[str, Any] | None:
    c_args: list[str] = []
    has_explicit_git_paths = False
    i = 1
    n = len(tokens)
    while i < n:
        tok = tokens[i]
        if tok == "--":
            i += 1
            break
        if tok == "-C":
            if i + 1 < n:
                c_args.append(tokens[i + 1])
                i += 2
                continue
        elif tok.startswith("-C") and len(tok) > 2:
            c_args.append(tok[2:])
            i += 1
            continue
        if tok in GIT_VAL_GLOBAL_OPTS:
            if tok in ("--git-dir", "--work-tree"):
                has_explicit_git_paths = True
            i += 2
            continue
        if any(tok.startswith(opt + "=") for opt in GIT_VAL_GLOBAL_OPTS):
            if tok.startswith("--git-dir=") or tok.startswith("--work-tree="):
                has_explicit_git_paths = True
            i += 1
            continue
        if tok.startswith("-"):
            i += 1
            continue
        break

    if i >= n:
        return None

    subcmd = tokens[i]
    args = tokens[i + 1 :]

    if subcmd == "reset":
        if "--hard" in args:
            return decision("deny", "Blocked destructive git reset --hard.")

    elif subcmd == "clean":
        for a in args:
            if a == "--force" or has_short_flag(a, "f"):
                return decision("deny", "Blocked destructive git clean -f/--force.")

    elif subcmd == "stash":
        pos = [a for a in args if not a.startswith("-")]
        if pos and pos[0] == "clear":
            return decision("deny", "Blocked destructive git stash clear.")
        if pos and pos[0] == "drop":
            return decision("force_ask", "Discarding stashes requires confirmation.")

    elif subcmd == "push":
        pos: list[str] = []
        repo_override: str | None = None
        has_force_with_lease = False
        has_delete_flag = False
        j = 0
        while j < len(args):
            a = args[j]
            if a in ("--force", "--force-if-includes") or has_short_flag(a, "f"):
                return decision(
                    "deny",
                    "Blocked raw force push; use --force-with-lease on an explicit non-main origin branch.",
                )
            if a == "--force-with-lease" or a.startswith("--force-with-lease="):
                has_force_with_lease = True
                j += 1
                continue
            if a == "--delete" or has_short_flag(a, "d"):
                has_delete_flag = True
                j += 1
                continue
            if a == "--repo" and j + 1 < len(args):
                repo_override = args[j + 1]
                j += 2
                continue
            if a.startswith("--repo="):
                repo_override = a.split("=", 1)[1]
                j += 1
                continue
            if a in GIT_PUSH_VAL_OPTS:
                j += 2
                continue
            if any(a.startswith(opt + "=") for opt in GIT_PUSH_VAL_OPTS):
                j += 1
                continue
            if a == "--":
                pos.extend(args[j + 1 :])
                break
            if not a.startswith("-"):
                pos.append(a)
            j += 1

        if repo_override is not None:
            target_remote = repo_override
            refspecs = pos
        else:
            target_remote = pos[0] if pos else ""
            refspecs = pos[1:]

        if target_remote == "upstream" or "mamcjr/botwrecompandroid" in target_remote.lower():
            return decision("deny", "Blocked pushing directly to upstream.")

        for ref in refspecs:
            if ref.startswith("+"):
                return decision("deny", "Blocked +refspec force push.")
            dst_raw = ref.split(":", 1)[-1]
            dst_branch = (
                dst_raw[len("refs/heads/") :] if dst_raw.startswith("refs/heads/") else dst_raw
            )
            if (has_delete_flag or ref.startswith(":")) and dst_branch in ("main", "master"):
                return decision("deny", "Blocked deleting remote main/master branch.")

        if has_force_with_lease:
            if target_remote != "origin" or not refspecs:
                return decision(
                    "deny",
                    "Blocked force-with-lease push without explicit origin and non-main branch refspec.",
                )
            for ref in refspecs:
                dst = ref.split(":", 1)[-1]
                if dst.startswith("refs/heads/"):
                    dst = dst[len("refs/heads/") :]
                if dst in ("", "main", "master", "HEAD") or dst.startswith("refs/tags/"):
                    return decision(
                        "deny",
                        f"Blocked force-with-lease push targeting protected ref ({ref}).",
                    )
            return decision(
                "force_ask",
                "git push --force-with-lease requires explicit owner authorization.",
            )

        return decision("force_ask", "git push requires explicit owner authorization.")

    elif subcmd == "checkout":
        resolved = resolve_git_dir(cwd, c_args)
        pos_args: list[str] = []
        k = 0
        while k < len(args):
            a = args[k]
            if a in ("-b", "-B", "--orphan"):
                k += 2  # branch name is not a file path
                continue
            if a == "--":
                pos_args.extend(args[k + 1 :])
                break
            if not a.startswith("-"):
                pos_args.append(a)
            k += 1
        discards_file = any(
            a == "."
            or re.search(r"\.[a-zA-Z0-9]{1,5}$", a)
            or (resolved and os.path.isfile(os.path.join(resolved, a)))
            for a in pos_args
        )
        if (
            "--" in args
            or discards_file
            or "--force" in args
            or any(has_short_flag(a, "f") for a in args)
        ):
            return decision("force_ask", "Discarding working-tree changes requires confirmation.")

    elif subcmd == "restore":
        return decision("force_ask", "Restoring/discarding working-tree files requires confirmation.")

    elif subcmd == "commit":
        resolved = resolve_git_dir(cwd, c_args)
        if has_explicit_git_paths or not is_inside_feature_worktree(resolved):
            return decision(
                "force_ask",
                "git commit outside isolated .mali/worktrees/* requires explicit owner authorization.",
            )

    return None


def expand_nested_shell_tokens(tokens: list[str], depth: int = 3) -> list[list[str]]:
    """Expand nested quoted command strings (e.g. adb shell 'su -c "..."')."""
    if depth <= 0:
        return [tokens]
    statements: list[list[str]] = [tokens]
    extra: list[list[str]] = []
    for idx, tok in enumerate(tokens):
        if any(c in tok for c in (" ", "\t", ";", "&", "|", "\n")):
            for sub in split_statements(tok):
                extra.extend(expand_nested_shell_tokens(sub, depth - 1))
        elif tok in ("-c", "/c") and idx + 1 < len(tokens):
            for sub in split_statements(tokens[idx + 1]):
                extra.extend(expand_nested_shell_tokens(sub, depth - 1))
    return statements + extra


def check_adb_statement(tokens: list[str]) -> dict[str, Any] | None:
    i = 1
    n = len(tokens)
    while i < n:
        tok = tokens[i]
        if tok in ADB_VAL_GLOBAL_OPTS:
            i += 2
            continue
        if tok.startswith("-"):
            i += 1
            continue
        break
    if i >= n:
        return None
    subcmd = tokens[i]
    args = tokens[i + 1 :]

    if subcmd == "uninstall":
        pos = [a for a in args if not a.startswith("-")]
        if "com.botwrecomp.android" in pos:
            return decision(
                "deny",
                "Blocked uninstalling stock com.botwrecomp.android (would wipe saves).",
            )

    elif subcmd == "shell":
        candidate_stmts: list[list[str]] = []
        for arg in args:
            for st in split_statements(arg):
                candidate_stmts.extend(expand_nested_shell_tokens(st))
        candidate_stmts.append(args)
        for st_tokens in candidate_stmts:
            for idx in range(len(st_tokens) - 2):
                if (
                    st_tokens[idx] == "pm"
                    and st_tokens[idx + 1] in ("clear", "uninstall")
                    and "com.botwrecomp.android" in st_tokens[idx + 2 :]
                ):
                    return decision(
                        "deny",
                        f"Blocked pm {st_tokens[idx + 1]} on stock com.botwrecomp.android (would wipe saves).",
                    )
    return None


def check_gh_statement(tokens: list[str]) -> dict[str, Any] | None:
    if len(tokens) >= 3:
        area = tokens[1].lower()
        verb = tokens[2].lower()
        if area == "pr" and verb in {"create", "merge", "close", "reopen", "comment", "edit", "ready", "review"}:
            return decision("force_ask", "External GitHub PR mutation requires explicit owner authorization.")
        if area == "issue" and verb in {"create", "close", "reopen", "comment", "edit", "delete"}:
            return decision("force_ask", "External GitHub Issue mutation requires explicit owner authorization.")
    return None


def find_repo_root(cwd_norm: str) -> str:
    low = cwd_norm.lower().rstrip("/")
    marker = "/botwrecompandroid-mali"
    idx = low.find(marker)
    if idx != -1:
        return low[: idx + len(marker)]
    return low


def is_same_or_ancestor(candidate: str, protected: str) -> bool:
    c = candidate.rstrip("/")
    p = protected.rstrip("/")
    return c == p or p.startswith(c + "/")


def check_delete_statement(tokens: list[str], cwd: str) -> dict[str, Any] | None:
    exe = os.path.basename(normalize_path(tokens[0])).lower()
    args = tokens[1:]
    flags_low = [a.lower() for a in args if a.startswith("-") or (exe in ("rd", "rmdir", "del", "erase") and a.startswith("/"))]

    is_ps_rm = exe in ("remove-item", "ri", "rm", "rm.exe")
    has_recursive = any(
        f in ("-recurse", "-r")
        or has_short_flag(f, "r")
        or (exe in ("rd", "rmdir", "del", "erase") and bool(re.search(r"/s(?=/|$)", f)))
        for f in flags_low
    )

    if not has_recursive:
        return None

    repo_root = find_repo_root(normalize_path(cwd))
    protected_always = [
        f"{repo_root}/.mali/user-backup",
        f"{repo_root}/.mali/game-preparation",
        f"{repo_root}/android/deps/out",
    ]
    protected_ps = protected_always + [f"{repo_root}/.mali/worktrees"]

    targets = [normalize_path(a).lower() for a in args if a.lower() not in flags_low]
    for raw_t in targets:
        resolved_t = resolve_git_dir(cwd, [raw_t]).lower()
        for prot in protected_always:
            if is_same_or_ancestor(resolved_t, prot):
                return decision(
                    "deny",
                    f"Blocked recursive deletion affecting protected path ({prot}).",
                )
        if is_ps_rm:
            for prot in protected_ps:
                if is_same_or_ancestor(resolved_t, prot):
                    return decision(
                        "deny",
                        "Blocked Remove-Item/rm recursive deletion affecting junction-bearing tree; unlink junctions with cmd /c rmdir first.",
                    )
        for probe in (raw_t, resolved_t):
            if re.search(r"(?:^|/)\.mali/(?:user-backup|game-preparation)(?:/|$)", probe):
                return decision(
                    "deny",
                    "Blocked recursive deletion of protected user-backup or game-preparation directory.",
                )
            if is_ps_rm and re.search(r"(?:^|/)(?:\.mali/worktrees|android/deps/out)(?:/|$)", probe):
                return decision(
                    "deny",
                    "Blocked Remove-Item/rm recursive deletion on junction-bearing tree; unlink junctions with cmd /c rmdir first.",
                )
            if re.search(r"(?:^|/)android/deps/out(?:/|$)", probe) and ".mali/worktrees/" not in probe:
                return decision(
                    "deny",
                    "Blocked recursive deletion of root android/deps/out.",
                )
    return None


def check_run_command(cmd_raw: str, cwd: str) -> dict[str, Any]:
    raw_statements = split_statements(cmd_raw)
    statements: list[list[str]] = []
    for st in raw_statements:
        statements.extend(unwrap_shell_wrappers(st))

    ask_decision: dict[str, Any] | None = None
    for tokens in statements:
        if not tokens:
            continue
        exe = os.path.basename(normalize_path(tokens[0])).lower()
        res: dict[str, Any] | None = None
        if exe in ("git", "git.exe"):
            res = check_git_statement(tokens, cwd)
        elif exe in ("adb", "adb.exe"):
            res = check_adb_statement(tokens)
        elif exe in ("gh", "gh.exe"):
            res = check_gh_statement(tokens)
        elif exe in ("remove-item", "ri", "rm", "rm.exe", "rd", "rmdir", "del", "erase"):
            res = check_delete_statement(tokens, cwd)

        if res:
            if res["decision"] == "deny":
                return res
            if res["decision"] == "force_ask" and ask_decision is None:
                ask_decision = res

    return ask_decision if ask_decision is not None else decision("allow")


def check_write_tool(args: dict[str, Any]) -> dict[str, Any]:
    raw_path = (
        args.get("Path")
        or args.get("path")
        or args.get("TargetFile")
        or args.get("target_file")
        or ""
    )
    if not isinstance(raw_path, str) or not raw_path.strip():
        return decision("allow")

    p = normalize_path(raw_path)
    p_low = posixpath.normpath(p).lower()

    # Never write inside Dump/ or overwrite existing backup hashes/archives
    if re.search(r"(?:^|/)dump/", p_low):
        return decision("deny", "Blocked writing inside proprietary Dump/ directory.")
    if re.search(r"(?:^|/)\.mali/user-backup/", p_low) and (
        p_low.endswith(".sha256") or p_low.endswith(".tar") or p_low.endswith(".zip")
    ):
        return decision(
            "deny",
            "Blocked overwriting existing user-backup archive or SHA-256 manifest.",
        )

    # Ask before editing protected upstream/build control files
    if re.search(r"(?:^|/)(?:tools/patches\.py|runtime/gamelib\.h|\.gitignore)$", p_low):
        return decision(
            "force_ask",
            f"Modifying protected file ({os.path.basename(p)}) requires explicit owner authorization.",
        )

    return decision("allow")


def evaluate(payload: dict[str, Any]) -> dict[str, Any]:
    tool_call = payload.get("toolCall") or {}
    tool_name = str(tool_call.get("name") or "").lower()
    args = tool_call.get("args") or {}
    cwd = str(payload.get("cwd") or "")

    if tool_name == "run_command":
        cmd = (
            args.get("CommandLine")
            or args.get("commandLine")
            or args.get("command")
            or ""
        )
        return check_run_command(str(cmd), cwd)

    if tool_name in WRITE_TOOLS:
        return check_write_tool(args)

    return decision("allow")


def main() -> int:
    try:
        raw = sys.stdin.read()
        payload = json.loads(raw) if raw.strip() else {}
        result = evaluate(payload)
    except Exception as exc:  # fail open with explicit note only on malformed stdin
        result = decision("allow", f"safety_gate parse fallback: {exc}")
    sys.stdout.write(json.dumps(result) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
