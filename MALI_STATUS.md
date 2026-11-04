# MALI status (read this first; keep it short)

Updated: 2026-11-04 (Europe/Moscow). History lives in `MALI_PROGRESS.md`: find a section by heading, never read the whole file.

## Base
- Active baseline: upstream `v0.3.1-beta`, tag `c48606cfafaaa4f41083830bbec2fd8b9ab01b47` = `upstream/main` = fork `main` = local `main`. Last release check 2026-11-04T19:55Z (see table in `MALI_PLAN.md`); recheck at session start.
- Tablet (Immortalis-G925; serial from `adb devices`): stock 0.3.1 debug APK `9e11940d...54fe` verified on device 2026-11-04T19:55Z after step 11b; app stopped, scratch dirs removed.
- Checkout: `benchmark-mvp`, clean.
- `BOTWRecompAndroid-checkpoint/` (untracked dir at repo root) is an old local copy; ignore it.

## Branches (fork `Propinbo/BOTWRecompAndroid`)
- `mali-workspace`: documentation checkpoint only (published through the step 11b refinements; find the commit with `git log origin/mali-workspace`).
- `benchmark-mvp` `6a808ef` (3 commits on `c48606c`): step 11b, local only, not pushed.
- `pr-maintenance5` `e9ef72c`: pushed to fork, PR #255 open.
- `format-probe` `24cd72b`: pushed to fork only, no PR (test_vk_formats probe for #244).
- Local only, not pushed: `audit-fixes-030`, `audit-fixes-0213`, `pr210-followups`, `vuid-fixes-030`, `vuid-fixes-0213`.
- Merged upstream earlier: #183 (`pace-vsync-phase`), #210 (`mali-bc-render-target-0213`), #245 (`fix-test-nvn-lifetime`).

## Open upstream items (`mamcjr/BOTWRecompAndroid`)
- PR #255 (maintenance5 for points without PointSize, no empty `vkCmdCopyBuffer` regions): open, mergeable, 0 comments at last check; validation, FPS and runner checks passed.
- Issue #214 (built-in benchmark proposal): open; maintainer accepted scenes, scratch-save memory writes, 1.9.0 only, fixed cap via env; hooks Native-only for the prototype.
- Issue #244 (formats without feature report, UB): open; our probe report published as a comment, 2 comments total at last check.
- Issue #174: see its sections in `MALI_PROGRESS.md`; maintainer asked for more data, not pursued now.

## Authorized now
- Nothing in progress. Step 11b and its refinements are COMPLETE (results: `MALI_PROGRESS.md` "Step 11b refinements before the PR", `.mali/benchmark/mvp-11b/`).
- Standing: documentation-only checkpoints of the four docs to `mali-workspace` (including `MALI_STATUS.md` since 2026-11-04).

## Awaiting the user's decision
- Steps 11c, 11d: each needs its own command (`Приступай к шагу N`).
- PR for `benchmark-mvp` (text in `.mali/benchmark/mvp-11b/PR.md`, shown to the user), whether `cache_cold` stays this strict (13 of 18 warm windows), a way to start without adb (none in the app), upstream pushes, new PRs, source commits: need separate authorization. `audit-fixes-030` stays unpushed by the user's decision.

## Key user decisions (pointers into `MALI_PROGRESS.md` by heading)
- Role contract: user owns product, agent owns technical choices; protect the user's saves; scratch copies only. See `AGENTS.md` and "Built-in benchmark proposal and role contract".
- Rolling-upstream policy (2026-11-01) and PR text rules (no "Generated with" line, no unpushed branches mentioned). See "Migration to 0.3.1 and branch `pr-maintenance5`".
- Step 11 and its split 11a-11d approved after the maintainer's answer. See "Step 11 added to the plan".
- Test strategy: metamorphic checks first, negative control before a test is binding, `fps_mode=4` instead of `LARECOMP_FPS=40`. See "Benchmark test strategy for step 11".
- PR auto-fix events: CI/conflict fixes stay local, show diff, wait for confirmation (memory rule).

## Where the main reports are
- Benchmark: `.mali/benchmark/FEASIBILITY.md`, `TEST_STRATEGY.md`, `probe-11a/`, `mvp-11b/`.
- PR #255 evidence: `.mali/pr-maintenance5-031/` (`PR.md`, `device/sheet.jpg`, `runner.log`, APKs).
- Format probe: `.mali/format-probe/` (report JSON, `run.log`, reply draft).
- G925 audit: `.mali/audit-g925-20261103/` (`SUMMARY.md`, `FINDINGS.md`, `FIXES.md`).
- VUID triage: `.mali/vuid-triage/`; migration 0.3.0: `.mali/migration-030/`; fix-nvn-lifetime: `.mali/fix-nvn-lifetime/`.
- Teleport dissolve and milestones 1-10: `.mali/milestone-*`, `.mali/issue-174-*`.
- Checkpoint manifest: `.mali/checkpoint-manifest.json`.
