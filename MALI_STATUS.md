# MALI Status

Last updated: `2026-11-17` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.5-beta`** (`6113c61a1d0e6f78532eb7fdde9cc9348613ad4b`); **`upstream/main`:** `87df09e3dd2901c6ba6a53616c3ebbef9478e552` (`#315` `ac38c7a`, `#461` `689372b`).
- **Fork `main`:** `6113c61a1d0e6f78532eb7fdde9cc9348613ad4b` (`main == origin/main == v0.4.5-beta`, synced to releases).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `6113c61` | Clean `v0.4.5-beta` baseline & docs |
| `.mali/worktrees/mali-release-045` | `release/mali-0.4.5-beta` | `a1ef820` | Integrated Mali Release (`6113c61` + `#461` + `#436` + stutter) |
| `.mali/worktrees/mali-shader-stutter` | `feat/mali-shader-stutter` | `8d95e8c` | Combined 3-pillar source on `eefafaf` (`_WIN32`/test fixes) |
| `.mali/worktrees/urgent-shader-builds` | `feat/urgent-shader-builds` | `c1b10cc` | **OPEN PR #481** (urgent queue + steal + `jobs_drain` on `eefafaf`) |
| `.mali/worktrees/shader-meta-cache` | `feat/shader-meta-cache` | `7759c06` | **OPEN PR #482** (`MWSMET01` `.smeta` cache on `eefafaf`) |
| `.mali/worktrees/dyn-vertex-input` | `feat/vk-dyn-vertex-input` | `122ec0c` | **OPEN PR #483** (`VK_EXT_vertex_input_dynamic_state` on `eefafaf`) |
| `.mali/worktrees/soft-linear-upstream` | `fix/soft-linear-filter` | `a106b0c` | **OPEN PR #436** (maintainer queued final A/B) |
| `.mali/worktrees/pipeline-prewarm-294` | `feat/pipeline-prewarm-294` | `6454ccc` | **OPEN PR #475** (maintainer queued final A/B) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (`3/3` starter heights verified on `SM-X730`) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **SM-X730** | Mali-G925 MC12 (stock Mali) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `67/67` saves + `3/4` non-sys prefs; `3047` `.spv`; idle |
| **CPH2653** *(unplugged)* | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** only) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `83/84` (`0` save/INI diffs); `1865` `.spv`; verified prior to disconnect |
| **SM-X926B** | Immortalis-G720 MC12 (`r44p1`, no Turnip) | Official `v0.3.4-beta` (`7b94efcc...3dcb`) | Setup `2026-11-12` (`43/43` payload, `botwgame.so`) |

Rollback APKs: signed `v0.4.5-beta` (`65ecd82f...c29b`) & `v0.4.3-beta` (`3d3ad8e7...10846`) under `.mali/apk/`.

## Open PRs & Active Priorities

- **PRs & Gate:** `#315` (`ac38c7a`) & **`#461` (`689372b`) `MERGED`**; `fix-rt-load-upload-296` worktree retired. Open (`MERGEABLE`/`CLEAN`): **`#481`** (`c1b10cc`), **`#482`** (`7759c06`), **`#483`** (`122ec0c`), `#480` (`c48d1b1`, `3/3` starter heights confirmed), `#475` (`6454ccc`, maintainer final A/B), `#436` (`a106b0c`, maintainer final A/B).
- **`release/mali-0.4.5-beta` (`a1ef820` on `SM-X730` & `CPH2653`):** Live verified Mali/Turnip build.
- **Next:** Monitor upstream merges across `#436`/`#475`/`#480`/`#481`–`#483` (adjust `H-6` / starter scenes on `#480` if `mamcjr` hits POCO deltas; rebase `#481`/`#482` once first merges; FF `main` on `v0.4.6-beta`). Parked: `Fullscreen 16:10`, `Memory 12b–12d/12f`, `Benchmark 11c–11d`.

## Recent Milestones

- `[2026-11-17]` **Cleanup, Starter-Scene Check (#480) & Reviews (#436/#475):** Retired merged `#461` worktree cleanly; verified `3/3` starter scenes (`plateau` `250.21`, `hateno-lab` `356.02`, `hateno-village` `242.61`) on `SM-X730` (`0` diffs), replied on `#480`.
- `[2026-11-17]` **Round-2 Review, PR #461 Merge & 3-Way Stutter PRs (#481, #482, #483):** Resolved `_WIN32`/test findings (`8d95e8c`); `#461` merged (`689372b`); opened `#481` (`c1b10cc`), `#482` (`7759c06`), `#483` (`122ec0c`).
- `[2026-11-17]` **Pre-PR Subagent Review Gate, Progress Archive & `bf9adc1`:** Institutionalized `2–3` verifier gate in `AGENTS.md`; Turnip reboot smoke `452/452` `.smeta` (`0 kept`).
- `[2026-11-17]` **CPH2653 Turnip A/B & `v0.4.5-beta` Upgrade (`a1ef820`):** `30.17 fps`, `gpu.p50 21.918 ms`, wait `-82.9%`, gpipes `-35.9%`, `99.8%` `.smeta`. See `MALI_PROGRESS.md`.
