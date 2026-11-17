# MALI Status

Last updated: `2026-11-17` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.5-beta`** (`6113c61a1d0e6f78532eb7fdde9cc9348613ad4b`): restart shader free + `#450` Mali program eviction + MediaTek `13 -> 19.2 MHz` Native clock fix (all verified on `SM-X730`), full-span copy texture hash, vertex cache retention; `#296` still open upstream.
- **Fork `main`:** `6113c61a1d0e6f78532eb7fdde9cc9348613ad4b` (`main == origin/main == upstream/main`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `6113c61` | Clean `v0.4.5-beta` baseline & workspace docs |
| `.mali/worktrees/fix-rt-load-upload-296` | `fix/rt-load-upload-296` | `6f83821` | **OPEN PR #461** (rebased/narrowed `#296`/`#438` on `6113c61`, pushed) |
| `.mali/worktrees/soft-linear-upstream` | `fix/soft-linear-filter` | `f92454e` | **OPEN PR #436** (`soft_linear` + `REPEAT`/mip on `6113c61`, pushed) |
| `.mali/worktrees/pr315-benchmark-v2` | `pr315-benchmark-v2` | `d928ccc` | **OPEN PR #315** (rebased on `6113c61`, pushed -> `benchmark-v2`) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `63f8747` | **12-scene** MainField set on `d928ccc` (`24/24 ok`, pushed) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **SM-X730** | Mali-G925 MC12 (stock Mali) | `v0.4.3-beta` (`3d3ad8e7...10846`) | `2026-11-17`: `67/67` saves + `4/4` prefs (`diffs=0`); `3047` SPIR-V; stock restored |
| **CPH2653** | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** `2bbb279a...` only) | `v0.4.3-beta` (`3d3ad8e7...10846`) | `2026-11-16`: `84/84` (`diffs=0`); `1865` SPIR-V |
| **SM-X926B** | Immortalis-G720 MC12 (`r44p1`, no Turnip) | Official `v0.3.4-beta` (`7b94efcc...3dcb`) | Setup `2026-11-12` (`43/43` payload, `botwgame.so`) |

Rollback APK: `.mali/apk/v0.4.3-beta/BOTWRecompAndroid-0.4.3-beta-signed.apk` (`3d3ad8e7...10846`).

## Open PRs & Active Priorities

- **PR #461** (`OPEN`, pushed `6f83821`, `MERGEABLE`/`CLEAN`): Narrowed to `#296`/`#438` writeback RT sync + `undiscard()`; `13/13` Bionic PASS, description + reply posted.
- **PR #436** (`OPEN`, pushed `f92454e`, `MERGEABLE`/`CLEAN`): Lazy `vk_any_format_no_linear()`, `CLAMP`/`REPEAT`, `soft_linear_mip`, `last_prog` init; `SM-X730` A/B clear; description + reply posted.
- **PR #315** (`OPEN`, pushed `d928ccc`, `MERGEABLE`/`CLEAN`): Rebased onto `6113c61`, reply posted.
- **`benchmark-scenes-v2`** (pushed `63f8747` on `d928ccc`): Upgraded 12-scene MainField set (`24/24 ok` `cap30`, `60 fps` DVFS check `4/4 ok`), queued for upstream PR after `#315` merges. See `.mali/benchmark/scenes-v3-20261117/REPORT.md`.
- **Next:** Paired stock `v0.4.5-beta` device rollout when `CPH2653` is connected + optional 30-min `#450` Mali session. Parked: `Fullscreen 16:10`, `Memory 12b–12d/12f`, `Benchmark 11c–11d`.

## Recent Milestones

- `[2026-11-17]` **PRs Pushed/Replied, 12-Scene Upgrade (`63f8747`), `60 fps` DVFS & `#450` Verified:** Pushed/replied `#315/#461/#436`; validated 12-scene `cap30` (`24/24 ok`, `46` reboot frees), `cap60` separation (`14.17–19.85 ms`, `39.4–55.9 fps`), & `5`-min `#450` eviction (`311` programs/`779` pipeline keys freed, `0` errors). See `.mali/benchmark/scenes-v3-20261117/REPORT.md`.
- `[2026-11-16]` **Workspace Cleanup, `main` FF & Antigravity Migration:** Freed `57.59 GiB`, purged local legacy remnants/refs, FF `main` to `v0.4.5-beta`, synced `mali-workspace`.
- `[2026-11-16]` **PR #461 & `#296`/`#438` Root Cause:** Fixed missing `lv=0` RT upload & Case 1/2 sync (`0230d2f`); `SM-X730` (`93` WB) & `CPH2653` Turnip (`93` WB). See `.mali/underwater-walk-296/upstream-pr/REPORT.md`.
