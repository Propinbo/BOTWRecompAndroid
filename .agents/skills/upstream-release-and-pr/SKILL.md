---
name: upstream-release-and-pr
description: >-
  Guides upstream GitHub Release checks, campaign SHA pinning, isolated worktree setup with Windows NTFS junctions, pre-PR cleanliness audits, and local PR packaging for mamcjr/BOTWRecompAndroid. Use when checking releases, starting a feature worktree, rebasing, or preparing an upstream PR.
---

# Upstream Release Check and PR Workflow

## 1. Rolling Upstream Release Check

1. Check releases when starting upstream-bound source work, evaluating/syncing baselines, or rebasing/packaging a PR (skip on unrelated local tasks or mid-campaign turns pinned to a chosen SHA):
   - `gh release list -R mamcjr/BOTWRecompAndroid -L 5`
   - `git ls-remote upstream refs/heads/main refs/tags/<newest_tag> refs/tags/<newest_tag>^{}`
2. Inspect release notes (`gh release view <tag> -R mamcjr/BOTWRecompAndroid`) and open/merged PRs (`gh pr list -R mamcjr/BOTWRecompAndroid --author Propinbo`) to check for overlapping upstream fixes or regressions.
3. Pin an exact commit SHA at the start of any bounded experiment or PR branch; never switch baselines mid-experiment.
4. Record any checked release in `MALI_STATUS.md` (and `MALI_PLAN.md` if baseline/active targets change). Keep active PR branches pinned to their chosen SHA until rebasing; fast-forward fork/local `main` to verified upstream releases when syncing baselines.

## 2. Isolated Feature Worktrees (Windows NTFS Junctions)

1. Never edit source directly on root `main` when preparing a feature or PR; create an isolated worktree under `.mali/worktrees/<slug>` and initialize nested submodules (`third_party/libadrenotools/lib/linkernsbypass`) before Android builds:
   ```powershell
   git worktree add -b <branch> .mali/worktrees/<slug> <pinned_sha>
   cmd /c mklink /J "<abs_worktree>\android\deps\out" "<abs_repo_root>\android\deps\out"
   git -C .mali/worktrees/<slug> submodule update --init --recursive
   ```
   The Android Gradle wrapper lives at `<abs_worktree>\android\botwrecompandroid\gradlew.bat` (use JDK 17).
2. Keep strictly one editing/building agent per worktree.
3. To remove a finished/superseded worktree safely on Windows:
   - First unlink the junction (`cmd /c rmdir "<abs_worktree>\android\deps\out"` — NEVER `Remove-Item -Recurse` on a junction!).
   - Verify `root\android\deps\out\lib\libshaderc_shared.so` still exists and zero `ReparsePoint`s remain inside `<abs_worktree>`.
   - Then delete the directory (`cmd /c rd /s /q "\\?\<abs_worktree>"`) and run `git worktree prune`.

## 3. Mandatory Pre-PR Multi-Agent Code Review Gate

1. Before running `gh pr create` or pushing substantive code/rebase changes to an open upstream PR, dispatch **2–3 parallel `invoke_subagent(TypeName="independent-verifier", ...)` calls** (strictly read-only) covering `<base>...HEAD` across orthogonal lenses:
   - **Lens 1 (`Concurrency, Lifetimes & Reset Safety`):** Threading, lock hierarchy, C11 atomics, worker drain vs. `vk_exec_reset` / `plat_reset_guest` / `map_placed`, `RT_ON_RESET`, and teardown cleanup.
   - **Lens 2 (`Vulkan Spec, Cache Integrity & Mali/Turnip Divergence`):** VUIDs, barriers, dynamic state (`ss_forget`/`SS_VI`), cache keys/corruption guards, Mali (`BC 0`, fmt `91`, LRU eviction) vs. Adreno+Turnip (`driver-id 18`, `placed maps`, `!g_inplace_copy`).
   - **Lens 3 (`Upstream Style, Benchmark/Flag-Off Overhead & Test Coverage`):** `CONTRIBUTING.md` comment style (*why*, not *what*), `#315` metrics / zero flag-off cost, `#ifdef __ANDROID__` portability, unit/negative-control coverage, and hygiene.
2. For wide multi-module PRs where per-file fan-out is cleaner, `run_workflow` (`type_name="self"` with explicit read-only instructions and structured JSON output schema) may supplement `independent-verifier`.
3. Resolve every `CRITICAL` and `MAJOR` finding (plus valid `MINOR` defects), re-verify tests on the final diff, and archive the review record in `.mali/<topic>/upstream-pr/CODE_REVIEW.md`.

## 4. Pre-PR Audit and Local Packaging (`CONTRIBUTING.md`)

1. Run [workspace_check.py](../../scripts/workspace_check.py) (`python -B -S .agents/scripts/workspace_check.py pr <worktree> --base upstream/main`) for instant validation of working-tree cleanliness, `git merge-tree --write-tree`, forbidden paths (`Dump/`, `extracted/`, `generated*`, `.bfttf`, `.mali/`, `.agents/`, `AGENTS.md`, `MALI_*.md`), protected files (`tools/patches.py`, `GAMELIB_ID`, `.gitignore`), and English-only commits/diffs.
2. Audit `git diff --stat <base>..HEAD` and `git log -n 5` to confirm one focused change per PR (`CONTRIBUTING.md`).
3. Export a clean `.patch` file and draft English `PR_DRAFT.md`, `CODE_REVIEW.md`, + `REPORT.md` locally under `.mali/<topic>/upstream-pr/`.
4. Run relevant host/Bionic/device tests after final post-review edits, then push the branch to `origin` and create or update the PR via `gh pr`.
