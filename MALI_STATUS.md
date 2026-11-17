# MALI Status

Last updated: `2026-11-17` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.5-beta`** (`6113c61a1d0e6f78532eb7fdde9cc9348613ad4b`, `2026-11-16T18:47:36Z`): restart shader free, `#450` Mali eviction `>1000`, MediaTek `13 -> 19.2 MHz` Native clock fix (active scaling confirmed on `SM-X730` `Dimensity 9400+`), full-span copy texture hash, vertex cache retention; `#296` in Known Issues.
- **Fork `main`:** Fast-forwarded to **`v0.4.5-beta`** (`6113c61a1d0e6f78532eb7fdde9cc9348613ad4b`), clean tree (`main == origin/main == upstream/main`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `6113c61` | Clean `v0.4.5-beta` baseline & workspace docs |
| `.mali/worktrees/fix-rt-load-upload-296` | `fix/rt-load-upload-296` | `6f83821` | **OPEN PR #461** (`#296`/`#438` narrowed on `6113c61`, local) |
| `.mali/worktrees/soft-linear-upstream` | `fix/soft-linear-filter` | `f92454e` | **OPEN PR #436** (`soft_linear` + `REPEAT`/mip on `6113c61`, local) |
| `.mali/worktrees/pr315-benchmark-v2` | `pr315-benchmark-v2` | `d928ccc` | **OPEN PR #315** (rebased on `6113c61`, local) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `7b69caa` | Unpushed 11-scene set (audited in `CANDIDATE_ANALYSIS_2026_11_17.md`) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **SM-X730** | Mali-G925 MC12 (stock Mali) | `v0.4.3-beta` (`3d3ad8e7...10846`) | `2026-11-17`: `67/67` saves + `4/4` prefs (`diffs=0`); `3047` SPIR-V; stock restored |
| **CPH2653** | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** `2bbb279a...` only) | `v0.4.3-beta` (`3d3ad8e7...10846`) | `2026-11-16`: `84/84` (`diffs=0`); `1865` SPIR-V |
| **SM-X926B** | Immortalis-G720 MC12 (`r44p1`, no Turnip) | Official `v0.3.4-beta` (`7b94efcc...3dcb`) | Setup `2026-11-12` (`43/43` payload, `botwgame.so`) |

Rollback APK: `.mali/apk/v0.4.3-beta/BOTWRecompAndroid-0.4.3-beta-signed.apk` (`3d3ad8e7...10846`).

## Open PRs & Active Priorities

- **PR #461** (`OPEN` `0230d2f` -> local **`6f83821`** on `6113c61`, `6` files, `mergeable=YES`): Narrowed to `#296`/`#438` writeback RT sync + `undiscard()`. `13/13` Bionic PASS (`10/13` fail on `6113c61`); `SM-X730` Run A `30.0 fps`, `0` errors. Draft: `.mali/pr-reviews-20261117/reply-pr461.md`.
- **PR #436** (`OPEN` `ab828a1` -> local **`f92454e`** on `6113c61`, `6` files, `mergeable=YES`): Lazy `vk_any_format_no_linear()`, `CLAMP`/`REPEAT` wrapping (`wrap=1` water clipmaps), `soft_linear_mip`, `last_prog` init. `test_soft_linear.py` + `SM-X730` `#461+#436` Run B clear phantom water (`30.0 fps`, `0` errors). Draft: `.mali/pr-reviews-20261117/reply-pr436.md`.
- **PR #315** (`OPEN` `674c8b0` -> local **`d928ccc`** on `6113c61`, `16` files, `mergeable=YES`): Rebased (`seq_wait_done` + `BP_GPU_WAIT_NS`, headers restored, `6/6` Bionic + `assembleDebug` PASS). Draft: `.mali/pr-reviews-20261117/reply-pr315.md`.
- **Next:** Owner approval to push `#315/#461/#436` + replies (and `mali-workspace`); upgrade `benchmark-scenes-v2` + `fps_mode=2`/`0`; `#450` 30-min Mali test + paired `v0.4.5-beta` device updates. Parked: `Fullscreen 16:10`, `Memory 12b–12d/12f`, `Benchmark 11c–11d`.

## Recent Milestones

- `[2026-11-17]` **PR #315/#461/#436 Updates, `SM-X730` A/B + Clock Check & Scene Audit:** Rebased/updated all 3 PRs onto `v0.4.5-beta` (`d928ccc`, `6f83821`, `f92454e`); verified on `SM-X730` (`LARECOMP_NATIVE=1`) `13->19.2 MHz` clock scaling (`0` errors, `30.0 fps`), `#461` alone (`runA`) vs `#461+#436` (`runB` removes phantom water); audited 11 scenes (`.mali/benchmark/scenes-v2/CANDIDATE_ANALYSIS_2026_11_17.md`).
- `[2026-11-16]` **Workspace Cleanup, `main` FF & Antigravity Migration:** Freed `57.59 GiB`, purged local legacy remnants/refs, FF `main` to `v0.4.5-beta`, synced `mali-workspace`.
- `[2026-11-16]` **PR #461 & `#296`/`#438` Root Cause:** Fixed missing `lv=0` RT upload & Case 1/2 sync (`0230d2f`); `SM-X730` (`93` WB) & `CPH2653` Turnip (`93` WB). See `.mali/underwater-walk-296/upstream-pr/REPORT.md`.
