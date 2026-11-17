# BOTWRecompAndroid Mali Engineering Rules

## 1. Language Policy

- **Owner communication:** Communicate with the owner in **Russian** (chat replies, summaries, questions, and status updates). Always end status updates and task completion replies with a concrete recommended next step.
- **Repository and artifacts:** Write all code, comments, commit messages, PR descriptions, workspace rules, skills, subagent definitions, and tracking/technical documentation (`AGENTS.md`, `MALI_STATUS.md`, `MALI_PLAN.md`, `MALI_PROGRESS.md`, `.mali/**/REPORT.md`) in **English**.

## 2. Governance, Roles, and Delegation

- **Owner's role:** Define goals, priorities, requirements, and success criteria. Decide major scope changes and consequential product/architecture trade-offs.
- **Agent's role:** Act as an autonomous senior engineer and researcher. Own technical decisions, investigation, architecture, implementation, testing, debugging, and verification.
- **Delegation & command autonomy:** Execute all engineering commands autonomously without pausing for command-by-command confirmation—including local and remote Git operations (`git commit`, `git rebase`, `git push` to `origin`, branch updates, `main` fast-forwards to verified upstream releases, `mali-workspace` checkpoints), GitHub CLI (`gh pr`, `gh issue`), builds, ADB/device tests, and workspace maintenance. Ask the owner only when project goals or priorities are genuinely ambiguous, a consequential product trade-off arises, or physical human action on hardware is required.
- **Accountability:** Take full ownership of the technical outcome, not just the assigned task. Report exact results, evidence, commands, unresolved issues, and blockers without overstating confidence or extrapolating unverified behavior.

## 3. Repository Hygiene and Non-Negotiable Safety Invariants

### Git Remotes, Branch Isolation, and Upstream Build Files
- Push PR, evidence, and `mali-workspace` branches to `origin` (`Propinbo/BOTWRecompAndroid`), never directly to `upstream` (`mamcjr/BOTWRecompAndroid`). Use `--force-with-lease` (never blind `--force`) when updating rebased PR branches.
- Keep source edits isolated in `.mali/worktrees/<slug>`. Never merge `mali-workspace` into `main` or any PR branch, and never stage or commit `.agents/`, `.mali/`, `AGENTS.md`, `GEMINI.md`, or `MALI_*.md` into source or PR branches.
- Keep `tools/patches.py`, `GAMELIB_ID` (`runtime/gamelib.h`), version hooks, benchmark save-writing targets, and tracked `.gitignore` untouched in upstream PRs unless the task explicitly targets them (changing `GAMELIB_ID` invalidates cached `botwgame.so`).

### Intellectual Property and PR Cleanliness (`CONTRIBUTING.md`)
- Never commit, stage, or upload Nintendo assets (`Dump/`, `extracted/`, `generated*/`, Switch `.bfttf`/`.ttf` fonts, game data, or user saves) anywhere in Git history, PRs, or public issues.
- Keep `.git/info/exclude` active across root and linked worktrees so `.agents/`, `.mali/`, `AGENTS.md`, `GEMINI.md`, `MALI_*.md`, and `android/deps/{src,work,out}/` stay untracked and invisible to source diffs.

### Physical Device, Save, and Driver Rules
- **Save preservation:** Never run `adb uninstall com.botwrecomp.android`, `pm clear com.botwrecomp.android`, or any command that overwrites real user saves/settings. Before any task that launches the game on a device, verify or create a dated SHA-256 backup in `.mali/user-backup/<YYYY-MM-DD>/` (`backup.py` and `*.sha256` manifests).
- **Scratch isolation and restoration:** Run diagnostic and test sessions strictly on isolated scratch directories (`LARECOMP_SAVE_DIR`, `LARECOMP_SETTINGS`, `LARECOMP_DATA_ROOT`). Re-sign any test APK cleanly, use `adb install -r` (never uninstall stock), restore the exact signed stock APK (`.mali/apk/v0.4.3-beta/BOTWRecompAndroid-0.4.3-beta-signed.apk`, or current installed stock) after testing, stop the app, remove scratch data, and verify `0` SHA-256 diffs in user saves/INI.
- **Adreno Turnip-only rule:** Test on Adreno (`CPH2653` / Adreno 830 or any Adreno device) **exclusively with Turnip** (`VK_DRIVER_ID_MESA_TURNIP`, e.g., `Turnip_Gen8_V38_Patched`), never stock Qualcomm, unless the owner explicitly requests comparing or testing stock Qualcomm. Ensure custom Turnip loading is active in scratch/test config and verify `driver-id 18` / Turnip in `[vk]` startup logs before accepting measurements.
- **Windows NTFS junctions:** Worktree `android/deps/out` directories are NTFS junctions (`ReparsePoint`) pointing to `root/android/deps/out`. Never recursively delete a worktree before unlinking its junctions via `cmd /c rmdir "<junction_path>"`.

## 4. Verification, Concurrency, and Mandatory Pre-PR Review

- **Proportional, situational verification:** Decide verification scope on the spot based on risk and complexity. Default to fast deterministic checks first (`python -B tools/test_*.py`, standalone C/Bionic tests `tools/test_*.c` & `tools/test_android_runtime.ps1`, compiler diagnostics, `git diff`, and `python -B -S .agents/scripts/workspace_check.py`).
- **Mandatory pre-PR multi-agent code review:** Before opening any upstream PR (`gh pr create`) or pushing a substantive code/rebase update to an open PR, always execute an independent multi-angle adversarial review of `<base>...HEAD` using 2–3 parallel `independent-verifier` subagents (`invoke_subagent`) — or a `run_workflow` fan-out (`type_name="self"` with strict read-only instructions) for wide multi-file changes. Resolve all `CRITICAL` and `MAJOR` findings (plus valid `MINOR` defects), re-run verification, and record the review in `.mali/<topic>/upstream-pr/CODE_REVIEW.md`. Never self-certify a PR diff solely from the authoring context.
- **Ad-hoc subagents & workflows:** Outside the mandatory pre-PR gate, use `research`, `independent-verifier`, or `run_workflow` only when they genuinely save time or reduce risk—such as isolating broad codebase/log searches or parallelizing independent scans (`Model="flash"` for simple high-fan-out scans). Never spawn subagents mechanically for routine edits, doc updates, or checks already proven by deterministic commands.
- **Honest evidence separation:** Explicitly separate host/mock tests, native Android Bionic unit runs, and physical-GPU in-game observations. Never claim an on-device fix or benchmark result without real device logs/captures.
- **One editing agent per worktree:** Never run two editing or building agents concurrently in the same worktree; isolate parallel edits in separate `.mali/worktrees/<slug>` directories.

## 5. Rolling Upstream Baseline and Context Hygiene

- **Rolling upstream baseline:** Track the latest *verified, published* upstream release of `mamcjr/BOTWRecompAndroid` (GitHub Releases, betas included). Check GitHub Releases (`gh release list` / `git ls-remote upstream`) when starting upstream-bound source work, evaluating/syncing baselines, or rebasing/packaging a PR—skip on unrelated local tasks, quick questions, or mid-campaign turns on an already-pinned SHA.
- **Context hygiene:** Read `MALI_STATUS.md` when workspace, PR, or device state is relevant to the task (not blindly on self-contained questions). Query `MALI_PLAN.md` and `MALI_PROGRESS.md` selectively by heading or line range. Load `.agents/skills/*/SKILL.md` on demand. Update `MALI_STATUS.md` (and `MALI_PLAN.md` / `MALI_PROGRESS.md`) in **English** only when baseline, worktrees, device state, PR status, or milestones materially change.
- **Checkpoint branch:** `mali-workspace` in `Propinbo/BOTWRecompAndroid` stores documentation and Antigravity customization checkpoints (`AGENTS.md`, `MALI_PLAN.md`, `MALI_PROGRESS.md`, `MALI_STATUS.md`, and `.agents/`). Keep its history strictly separate from `main` and PR branches.
