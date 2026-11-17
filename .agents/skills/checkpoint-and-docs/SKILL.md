---
name: checkpoint-and-docs
description: >-
  Maintains MALI_STATUS.md, MALI_PLAN.md, and MALI_PROGRESS.md in clean English within strict size budgets, archives old epochs under .mali/archive/, and manages mali-workspace checkpoint synchronization. Use when updating tracking docs or preparing a documentation checkpoint.
---

# Documentation and Checkpoint Maintenance

## 1. File Roles, Size Budgets, and Language

All tracking documents must be **100% English** (zero Cyrillic):
- **`MALI_STATUS.md`** (Target: `<= 4.5 KB`, `~70-90` lines): Live in-place state dashboard updated when baseline, worktrees, device state, PR status, or milestones materially change (latest upstream release check, active worktrees, hardware/APK/backup inventory, open PR statuses, active priorities/blockers, and 1-line links to the latest 5–6 completed entries in `MALI_PROGRESS.md`). Never turn `MALI_STATUS.md` into a multi-page chronological dump.
- **`MALI_PLAN.md`** (Target: `<= 16 KB`): Active roadmap, upstream baseline status, owner goals/constraints/success criteria for active/parked initiatives, and a compact index table of completed campaigns.
- **`MALI_PROGRESS.md`** (Target: indexed active epoch `<= 80 KB`): Uniform `## [YYYY-MM-DD] <Slug> — <Title> (<Status>)` headings with a top Table of Contents and a bottom Historical Archive Digest table pointing to `.mali/archive/MALI_PROGRESS_*.md`. Preserve every SHA, hash, address, measurement, and `.mali/` evidence path.

## 2. Epoch Archiving

When `MALI_PROGRESS.md` grows large, move closed older epochs cleanly into `.mali/archive/MALI_PROGRESS_<start>_to_<end>.md`, add a summary row in the Historical Archive Digest table, and keep an intact pre-edit snapshot in `.mali/archive/`.

## 3. Local Exclusion and `mali-workspace` Checkpoints

1. Verify `.git/info/exclude` blocks `/.agents/`, `/.mali/`, `/AGENTS.md`, `/GEMINI.md`, `/MALI_PLAN.md`, `/MALI_PROGRESS.md`, `/MALI_STATUS.md`, and `/android/deps/{src,work,out}/`.
2. Never merge `mali-workspace` into `main` or any PR branch, and never push to `origin/mali-workspace` without explicit owner approval.
3. Run [workspace_check.py](../../scripts/workspace_check.py) (`python -B -S .agents/scripts/workspace_check.py docs`) before committing a checkpoint on `mali-workspace` to verify UTF-8, size caps, `MALI_PROGRESS.md` ToC parity, zero Cyrillic, zero local-path/serial leaks, and zero `__pycache__`. Use `[skip ci]` in checkpoint commit messages.
4. **Recovery into a fresh `main` checkout:**
   ```powershell
   git fetch --no-tags https://github.com/Propinbo/BOTWRecompAndroid.git refs/heads/mali-workspace:refs/remotes/origin/mali-workspace
   git restore --source=origin/mali-workspace --worktree -- AGENTS.md MALI_PLAN.md MALI_PROGRESS.md MALI_STATUS.md .agents
   ```
   Verify none of those paths are staged (`git status`).
