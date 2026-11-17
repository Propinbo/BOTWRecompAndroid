# MALI Status

Last updated: `2026-11-17` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.5-beta`** (`6113c61a1d0e6f78532eb7fdde9cc9348613ad4b`); **`upstream/main`:** `eefafaf9a78a6a7fe1f2bf94bae6db8d7002a5ae` (includes `#315` `ac38c7a`, `#477`, `#470`, `#465`).
- **Fork `main`:** `6113c61a1d0e6f78532eb7fdde9cc9348613ad4b` (`main == origin/main == v0.4.5-beta`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `6113c61` | Clean `v0.4.5-beta` baseline & workspace docs |
| `.mali/worktrees/mali-release-045` | `release/mali-0.4.5-beta` | `a1ef820` | Integrated Mali Release (`6113c61` + `#461` + `#436` + shader-stutter, pushed) |
| `.mali/worktrees/mali-shader-stutter` | `feat/mali-shader-stutter` | `bf9adc1` | **3-pillar** stutter fix on `eefafaf` (3-subagent reviewed, reboot-drained, pushed) |
| `.mali/worktrees/fix-rt-load-upload-296` | `fix/rt-load-upload-296` | `7077d56` | **OPEN PR #461** (`#296`/`#438` rebased on `26da3fa`, pushed) |
| `.mali/worktrees/soft-linear-upstream` | `fix/soft-linear-filter` | `a106b0c` | **OPEN PR #436** (`soft_linear` review fixes on `26da3fa`, pushed) |
| `.mali/worktrees/pipeline-prewarm-294` | `feat/pipeline-prewarm-294` | `6454ccc` | **OPEN PR #475** (opt-in `#294` + 11 i18n + ABBA on `26da3fa`, pushed) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (height check + 12-scene MainField set on `26da3fa`, pushed) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **SM-X730** | Mali-G925 MC12 (stock Mali) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `67/67` saves + `3/4` non-sys prefs (`diffs=0`); `3047` SPIR-V; `vk_pipelines` intact |
| **CPH2653** | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** only) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `83/84` (`0` save/INI diffs); `1865` SPIR-V; `vk_pipelines` intact |
| **SM-X926B** | Immortalis-G720 MC12 (`r44p1`, no Turnip) | Official `v0.3.4-beta` (`7b94efcc...3dcb`) | Setup `2026-11-12` (`43/43` payload, `botwgame.so`) |

Rollback APKs: signed `v0.4.5-beta` (`65ecd82f...c29b`) & `v0.4.3-beta` (`3d3ad8e7...10846`) under `.mali/apk/`.

## Open PRs & Active Priorities

- **PRs & Gate:** `#315` `MERGED`; `#480` (`c48d1b1`), `#461` (`7077d56`), `#436` (`a106b0c`), `#475` (`6454ccc`) `OPEN`. Mandatory pre-PR `2–3` `independent-verifier` review gate active in `AGENTS.md`.
- **`feat/mali-shader-stutter` (`bf9adc1` on `eefafaf`):** 3-subagent reviewed + post-review Turnip reboot-drain verified; local PR package in `.mali/mali-shader-stutter/upstream-pr/`; awaiting owner choice (1 PR vs 3-PR split).
- **`release/mali-0.4.5-beta` (`a1ef820` on `SM-X730` & `CPH2653`):** Live verified build (`-35.9%` gpipes, `99.8%` `.smeta`).
- **Next:** Owner decision on `#mali-shader-stutter` 1-vs-3 PR submission; optional `copy_texture` `gpu_owned` after `#461`. Parked: `Fullscreen 16:10`, `Memory 12b–12d/12f`, `Benchmark 11c–11d`.

## Recent Milestones

- `[2026-11-17]` **Pre-PR Subagent Review Gate, Progress Archive & `feat/mali-shader-stutter` Rebase (`bf9adc1`):** Added permanent 3-verifier gate; resolved all 3 reviewers' findings (`pthread_once`, `jobs_drain()`, 7 `translate.c` `.smeta` flags, `gmem_miss_log_on()`, `ss_forget()`); passed host + dual-device Bionic + Turnip `repeats=2` reboot smoke (`452/452` `.smeta`, `0 kept`); archived `11-05..14` & pushed `mali-workspace`.
- `[2026-11-17]` **CPH2653 Turnip A/B & `v0.4.5-beta` Upgrade (`a1ef820`):** `3/3` Bionic on Oryon + 3-pass Turnip `driver-id 18` (`30.17 fps`, `gpu.p50 21.918 ms`, wait `-82.9%`, max `94.21 -> 5.62 ms`, gpipes `-35.9%`, `99.8%` `.smeta`); upgraded `CPH2653` (`0` save/INI diffs, caches intact).
- `[2026-11-17]` **PR #315 Merged, PR #480 Opened & Review Updates (`26da3fa`):** Opened `#480` (`c48d1b1`), rebased `#461` (`7077d56`), resolved `#436` (`a106b0c`) & `#475` (`6454ccc`) review items; `SM-X730` restored (`2229426f...`, `67/67`).
- `[2026-11-17]` **Integrated `v0.4.5-beta` Mali Release & `SM-X730` Rollout (`a1ef820`):** Verified `Native=1` (`13->19.2 MHz`, `30.0 fps`) & `Native=0` (`max 0.98ms`); upgraded `SM-X730` (`67/67` saves). See `MALI_PROGRESS.md`.
