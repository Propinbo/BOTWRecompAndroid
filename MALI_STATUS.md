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
| `.mali/worktrees/pipeline-prewarm-294` | `feat/pipeline-prewarm-294` | `56716f0` | **OPEN PR #475** (opt-in `#294` prewarming on `6113c61`, pushed) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `63f8747` | **12-scene** MainField set on `d928ccc` (`24/24 ok`, pushed) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **SM-X730** | Mali-G925 MC12 (stock Mali) | `v0.4.3-beta` (`3d3ad8e7...10846`) | `2026-11-17`: `67/67` saves + `4/4` prefs (`diffs=0`); `3047` SPIR-V; stock restored |
| **CPH2653** | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** `2bbb279a...` only) | `v0.4.3-beta` (`3d3ad8e7...10846`) | `2026-11-16`: `84/84` (`diffs=0`); `1865` SPIR-V |
| **SM-X926B** | Immortalis-G720 MC12 (`r44p1`, no Turnip) | Official `v0.3.4-beta` (`7b94efcc...3dcb`) | Setup `2026-11-12` (`43/43` payload, `botwgame.so`) |

Rollback APK: `.mali/apk/v0.4.3-beta/BOTWRecompAndroid-0.4.3-beta-signed.apk` (`3d3ad8e7...10846`).

## Open PRs & Active Priorities

- **PR #475** (`OPEN`, pushed `56716f0`, `MERGEABLE`/`CLEAN`): Opt-in `#294` loading-screen pipeline prewarming (`Video > Pipeline prewarming` / `LARECOMP_PIPE_PREWARM=1`, off by default); `6/6` Bionic + 3-pass `SM-X730` verified (`256` rec -> `6` queued, `5/5` ready); `#294` commented.
- **PR #461** (`OPEN`, `6f83821`, `CLEAN`), **PR #436** (`OPEN`, `f92454e`, `CLEAN`), **PR #315** (`OPEN`, `d928ccc`, `CLEAN`): Clean comments (`Draft` header removed); `evidence/pr436-pr461` (`8dc2d6d`) live with `runA`/`runB` PNGs embedded in `#436` + log links in `#436`/`#461`.
- **`benchmark-scenes-v2`** (`63f8747` on `d928ccc`, pushed): 12-scene MainField set queued after `#315`.
- **Next:** Paired `v0.4.5-beta` rollout on `CPH2653`; optional 30-min `#450` session. Parked: `Fullscreen 16:10`, `Memory 12b–12d/12f`, `Benchmark 11c–11d`.

## Recent Milestones

- `[2026-11-17]` **PR #475 (`#294` Prewarming) & `#436`/`#461` Evidence:** Published `evidence/pr436-pr461` (`8dc2d6d`), updated `#315/#436/#461` comments, and opened PR #475 (`56716f0`). See `.mali/pipeline-prewarm-294/upstream-pr/REPORT.md`.
- `[2026-11-17]` **PRs Pushed & 12-Scene Upgrade (`63f8747`):** Pushed `#315/#461/#436`; verified 12-scene `cap30` (`24/24 ok`), `cap60`, & 5-min `#450` eviction (`0` errors). See `.mali/benchmark/scenes-v3-20261117/REPORT.md`.
- `[2026-11-16]` **Workspace Cleanup, `main` FF & PR #461:** Freed `57.59 GiB`, FF'd `main` to `v0.4.5-beta`, synced `mali-workspace`, solved `#296`/`#438`.
