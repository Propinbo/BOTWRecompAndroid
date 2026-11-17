# MALI Status

Last updated: `2026-11-17` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.5-beta`** (`6113c61a1d0e6f78532eb7fdde9cc9348613ad4b`); **`upstream/main`:** `26da3fae07f14c09e16e13a18d2f4f9a9ac474ef` (includes merged PR `#315` `ac38c7a` + `#477` + `#470`).
- **Fork `main`:** `6113c61a1d0e6f78532eb7fdde9cc9348613ad4b` (`main == origin/main == v0.4.5-beta`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `6113c61` | Clean `v0.4.5-beta` baseline & workspace docs |
| `.mali/worktrees/mali-release-045` | `release/mali-0.4.5-beta` | `a1ef820` | Integrated Mali Release (`6113c61` + `#461` + `#436` + shader-stutter, pushed) |
| `.mali/worktrees/mali-shader-stutter` | `feat/mali-shader-stutter` | `64450ec` | **3-pillar** Mali shader/pipeline stutter fix on `6113c61` (pushed) |
| `.mali/worktrees/fix-rt-load-upload-296` | `fix/rt-load-upload-296` | `7077d56` | **OPEN PR #461** (`#296`/`#438` rebased on `26da3fa`, pushed) |
| `.mali/worktrees/soft-linear-upstream` | `fix/soft-linear-filter` | `a106b0c` | **OPEN PR #436** (`soft_linear` review fixes on `26da3fa`, pushed) |
| `.mali/worktrees/pipeline-prewarm-294` | `feat/pipeline-prewarm-294` | `6454ccc` | **OPEN PR #475** (opt-in `#294` + 11 i18n + ABBA on `26da3fa`, pushed) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (height check + 12-scene MainField set on `26da3fa`, pushed) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **SM-X730** | Mali-G925 MC12 (stock Mali) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `67/67` saves + `3/4` non-sys prefs (`diffs=0`); `3047` SPIR-V; `vk_pipelines` intact |
| **CPH2653** | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** `2bbb279a...` only) | `v0.4.3-beta` (`3d3ad8e7...10846`) | `2026-11-16`: `84/84` (`diffs=0`); `1865` SPIR-V |
| **SM-X926B** | Immortalis-G720 MC12 (`r44p1`, no Turnip) | Official `v0.3.4-beta` (`7b94efcc...3dcb`) | Setup `2026-11-12` (`43/43` payload, `botwgame.so`) |

Rollback APKs: signed `v0.4.5-beta` (`65ecd82f...c29b`) & `v0.4.3-beta` (`3d3ad8e7...10846`) under `.mali/apk/`.

## Open PRs & Active Priorities

- **PR #315 (`MERGED` `ac38c7a`):** `pr315-benchmark-v2` retired; **PR #480** (`c48d1b1`), **PR #461** (`7077d56`), **PR #436** (`a106b0c`), **PR #475** (`6454ccc`) all `OPEN` on `26da3fa`; `evidence/pr436-pr461` (`8dc2d6d`) live.
- **`feat/mali-shader-stutter`** (`64450ec`) & **`release/mali-0.4.5-beta`** (`a1ef820`, installed on `SM-X730`): Urgent priority, `.smeta`, `DYN_VI`, `#461`, `#436`; verified on `SM-X730` (`Native=1` & `Native=0`, `30.0 fps`).
- **Next:** Cross-test `feat/mali-shader-stutter` on `CPH2653` (Turnip) + upgrade phone to `v0.4.5-beta`; optional `copy_texture` `gpu_owned` follow-up after `#461`; optional 30-min `#450` soak. Parked: `Fullscreen 16:10`, `Memory 12b–12d/12f`, `Benchmark 11c–11d`.

## Recent Milestones

- `[2026-11-17]` **PR #315 Merged, PR #480 Opened & Review Updates (`26da3fa`):** Opened `#480` (`c48d1b1`), rebased `#461` (`7077d56`), resolved all `#436` (`a106b0c`, `plateau` grass `30.0 fps`) & `#475` (`6454ccc`, 11 i18n, `ABBA` stutter) review items; `SM-X730` restored (`2229426f...`, `67/67`).
- `[2026-11-17]` **Integrated `v0.4.5-beta` Mali Release & `SM-X730` Rollout (`a1ef820`):** Built/signed `.mali/apk/v0.4.5-beta-mali/`; verified `Native=1` (`13->19.2 MHz`, `17.8s` load, `30.0 fps`, `0 >50ms`) & `Native=0` (`0` skips, `max 0.98ms`) on `SM-X730`; upgraded `SM-X730` (`67/67` saves, caches intact).
- `[2026-11-17]` **Mali 3-Pillar Shader Stutter Fix (`64450ec`):** Pushed `feat/mali-shader-stutter`; `SM-X730` compile wait `-80%`, peak frame `4.13x` lower, live `gpipes -34.4%`, `99.2%` `.smeta` hits, `0%` GPU regression. See `.mali/mali-shader-stutter/REPORT.md`.
