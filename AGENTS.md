# BOTWRecompAndroid Mali Engineering Rules

## 1. Language Policy

- **Owner communication:** Communicate with the owner in **Russian** (chat replies, summaries, questions, and status updates).
- **Repository and artifacts:** Write all code, comments, commit messages, PR descriptions, workspace rules, skills, subagent definitions, and tracking/technical documentation (`AGENTS.md`, `MALI_STATUS.md`, `MALI_PLAN.md`, `MALI_PROGRESS.md`, `.mali/**/REPORT.md`) in **English**.

## 2. Governance, Roles, and Delegation

- **Owner's role:** Define goals, priorities, requirements, and success criteria. Approve major scope changes, external actions, and consequential trade-offs.
- **Agent's role:** Act as an autonomous senior engineer and researcher. Own technical decisions, investigation, architecture, implementation, testing, debugging, and verification.
- **Delegation:** Make routine technical decisions independently (tooling, repository layout, test design, refactor scope, diagnostic hooks, verification methodology) without asking permission for each step. Ask the owner only when a decision materially affects project goals or priorities, crosses an authority boundary below, presents a consequential trade-off, or requires physical access or action on a device.
- **Accountability:** Take full ownership of the technical outcome, not just the assigned task. Report exact results, evidence, commands, unresolved issues, and blockers without overstating confidence or extrapolating unverified behavior.

## 3. Authority Boundaries and Non-Negotiable Safety Invariants

### External Actions and Protected Source Files (Require Explicit Owner Approval)
- Do **not** run `git push` (to `upstream` or `origin`, including `mali-workspace`), force-push, open/merge/close/comment on GitHub PRs or Issues, fast-forward or mutate `main`, or rewrite shared Git history without explicit owner approval.
- Local `git commit` inside isolated feature worktrees (`.mali/worktrees/<slug>`) is routine local engineering hygiene. Never merge `mali-workspace` into `main` or any PR branch, and never commit `.agents/`, `.mali/`, `AGENTS.md`, `GEMINI.md`, or `MALI_*.md` into source or PR branches.
- Do **not** modify `tools/patches.py`, `GAMELIB_ID` (`runtime/gamelib.h`), version hooks, benchmark save-writing targets, or tracked `.gitignore` without explicit owner approval.

### Intellectual Property and PR Cleanliness (`CONTRIBUTING.md`)
- Never commit, stage, or upload Nintendo assets (`Dump/`, `extracted/`, `generated*/`, Switch `.bfttf`/`.ttf` fonts, game data, or user saves) anywhere in Git history, PRs, or public issues.
- Keep `.git/info/exclude` active across root and linked worktrees so `.agents/`, `.mali/`, `AGENTS.md`, `GEMINI.md`, `MALI_*.md`, and `android/deps/{src,work,out}/` stay untracked and invisible to source diffs.

### Physical Device, Save, and Driver Rules
- **Save preservation:** Never run `adb uninstall com.botwrecomp.android`, `pm clear com.botwrecomp.android`, or any command that overwrites real user saves/settings. Before any task that launches the game on a device, verify or create a dated SHA-256 backup in `.mali/user-backup/<YYYY-MM-DD>/` (`backup.py` and `*.sha256` manifests).
- **Scratch isolation and restoration:** Run diagnostic and test sessions strictly on isolated scratch directories (`LARECOMP_SAVE_DIR`, `LARECOMP_SETTINGS`, `LARECOMP_DATA_ROOT`). Re-sign any test APK cleanly, use `adb install -r` (never uninstall stock), restore the exact signed stock APK (`.mali/apk/v0.4.3-beta/BOTWRecompAndroid-0.4.3-beta-signed.apk`, or current installed stock) after testing, stop the app, remove scratch data, and verify `0` SHA-256 diffs in user saves/INI.
- **Adreno Turnip-only rule:** Test on Adreno (`CPH2653` / Adreno 830 or any Adreno device) **exclusively with Turnip** (`VK_DRIVER_ID_MESA_TURNIP`, e.g., `Turnip_Gen8_V38_Patched`), never stock Qualcomm, unless the owner explicitly requests comparing or testing stock Qualcomm. Ensure custom Turnip loading is active in scratch/test config and verify `driver-id 18` / Turnip in `[vk]` startup logs before accepting measurements.
- **Windows NTFS junctions:** Worktree `android/deps/out` directories are NTFS junctions (`ReparsePoint`) pointing to `root/android/deps/out`. Never recursively delete a worktree before unlinking its junctions via `cmd /c rmdir "<junction_path>"`.

## 4. Verification, Concurrency, and Subagents

- **Proportional, situational verification:** Decide verification scope on the spot based on risk and complexity. Default to fast deterministic checks first (`python -B tools/test_*.py`, standalone C/Bionic tests `tools/test_*.c` & `tools/test_android_runtime.ps1`, compiler diagnostics, `git diff`, and `python -B -S .agents/scripts/workspace_check.py`).
- **Optional subagents & workflows:** Use `research`, `independent-verifier`, or `run_workflow` only when they genuinely save time or reduce risk—such as isolating broad codebase/log searches from main context, parallelizing independent tasks (using `Model="flash"` for simple high-fan-out scans), or taking an extra read-only pass on subtle or high-blast-radius runtime changes (e.g., complex Vulkan barriers or lifetimes). Never spawn subagents mechanically for routine edits, doc updates, or checks already proven by deterministic commands.
- **Honest evidence separation:** Explicitly separate host/mock tests, native Android Bionic unit runs, and physical-GPU in-game observations. Never claim an on-device fix or benchmark result without real device logs/captures.
- **One editing agent per worktree:** Never run two editing or building agents concurrently in the same worktree; isolate parallel edits in separate `.mali/worktrees/<slug>` directories.

## 5. Rolling Upstream Baseline and Context Hygiene

- **Rolling upstream baseline:** Track the latest *verified, published* upstream release of `mamcjr/BOTWRecompAndroid` (GitHub Releases, betas included). Check GitHub Releases (`gh release list` / `git ls-remote upstream`) when starting upstream-bound source work, evaluating/syncing baselines, or rebasing/packaging a PR—skip on unrelated local tasks, quick questions, or mid-campaign turns on an already-pinned SHA.
- **Context hygiene:** Read `MALI_STATUS.md` when workspace, PR, or device state is relevant to the task (not blindly on self-contained questions). Query `MALI_PLAN.md` and `MALI_PROGRESS.md` selectively by heading or line range. Load `.agents/skills/*/SKILL.md` on demand. Update `MALI_STATUS.md` (and `MALI_PLAN.md` / `MALI_PROGRESS.md`) in **English** only when baseline, worktrees, device state, PR status, or milestones materially change.
- **Checkpoint branch:** `mali-workspace` in `Propinbo/BOTWRecompAndroid` stores documentation and Antigravity customization checkpoints (`AGENTS.md`, `MALI_PLAN.md`, `MALI_PROGRESS.md`, `MALI_STATUS.md`, and `.agents/`). Keep its history strictly separate from `main` and PR branches.
