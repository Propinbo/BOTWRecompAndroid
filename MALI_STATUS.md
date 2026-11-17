# MALI Status

Last updated: `2026-11-18` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.6-beta`** (`0897fb889719d0489a7bcbd8dba320b997d377ce`), including `#436`, `#475`, and `#401` freeze watchdog.
- **Fork `main`:** `0897fb889719d0489a7bcbd8dba320b997d377ce` (`main == origin/main == v0.4.6-beta`, synced).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `0897fb8` | Clean `v0.4.6-beta` baseline & docs |
| `.mali/worktrees/mali-release-046` | `release/mali-0.4.6-beta` | `65297ca` | Integrated Mali Release (`0897fb8` + `#481–#483` + `perf-046`) |
| `.mali/worktrees/mali-perf-046` | `feat/mali-perf-046` | `83bc58c` | Stacked Mali `cbwin_snap`, in-place feedback & `dchain` |
| `.mali/worktrees/mali-async-tex-slots` | `feat/mali-async-tex-slots` | `5eea75f` | **OPEN PR #485** (async Mali texture pool slots, `0` conflicts) |
| `.mali/worktrees/urgent-shader-builds` | `feat/urgent-shader-builds` | `eaec823` | **OPEN PR #481** (rebased on `0897fb8`, `CLEAN`) |
| `.mali/worktrees/shader-meta-cache` | `feat/shader-meta-cache` | `ce13c72` | **OPEN PR #482** (rebased on `0897fb8`, `CLEAN`) |
| `.mali/worktrees/dyn-vertex-input` | `feat/vk-dyn-vertex-input` | `3383e1e` | **OPEN PR #483** (rebased on `0897fb8`, `CLEAN`) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (`3/3` starter heights verified, `CLEAN`) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **SM-X730** | Mali-G925 MC12 (`r49p1` stock Mali) | `v0.4.6-beta` Mali (`449fb940...69c2`) | `2026-11-17`: `67/67` saves + `3/4` non-sys prefs (`0` diffs); `3047` `.spv`; idle |
| **CPH2653** *(unplugged)* | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** only) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `83/84` (`0` save/INI diffs); `1865` `.spv`; verified prior to disconnect |
| **SM-X926B** | Immortalis-G720 MC12 (`r44p1`, no Turnip) | Official `v0.3.4-beta` (`7b94efcc...3dcb`) | Setup `2026-11-12` (`43/43` payload, `botwgame.so`) |

Rollback APKs: signed `v0.4.6-beta` (`aa9ca25b...3e27`), `v0.4.5-beta` (`65ecd82f...c29b`), & `v0.4.3-beta` (`3d3ad8e7...10846`) under `.mali/apk/`.

## Open PRs & Active Priorities

- **Merged in `v0.4.6-beta`:** `#315`, `#436`, `#461`, `#475`.
- **Open (`MERGEABLE`/`CLEAN` on `0897fb8`):** **`#480`** (`c48d1b1`), **`#481`** (`eaec823`), **`#482`** (`ce13c72`), **`#483`** (`3383e1e`), **`#485`** (`5eea75f`).
- **Ready locally (`origin/feat/mali-perf-046` `83bc58c`):** Cooled 60 FPS `ABBA` on `SM-X730` measured **`+6.6%..+11.1%` FPS** (`opt_1` peak **`+20.1%`**, `gpu_p50 -8.5%..-27.2%`, `upload -83.5%`, `fbcopy 650->0`, `dchained 43–46%`), zero artifacts across `6/6` PNGs; `12/12` Bionic tests PASS on `release/mali-0.4.6-beta`.
- **Next:** Monitor upstream merges across `#480`–`#483` & `#485`; split `feat/mali-perf-046` into clean PRs (`dchain`, Mali `inplace_reads`, `cbwin_snap`) after `#482`/`#485` land (optional Turnip-only A/B on `CPH2653` once plugged in). Parked: `Fullscreen 16:10`, `Memory 12b–12d/12f`, `Benchmark 11c–11d`.

## Recent Milestones

- `[2026-11-18]` **`v0.4.6-beta` Sync, PR #485 Async Slots, 60 FPS Mali Perf `ABBA` & Release Rollout:** FF `main` (`0897fb8`); rebased/pushed `#481–#483`; opened `#485`; verified `cbwin_snap`+`inplace`+`dchain` (`+6.6%..+11.1%` FPS, `86/86` `slot drains 0`); installed `0.4.6-beta-mali` (`449fb940...69c2`) on `SM-X730`; retired `0.4.5` worktrees.
- `[2026-11-17]` **Cleanup, Starter-Scene Check (#480) & Reviews (#436/#475):** Retired `#461` worktree; verified `3/3` starter scenes (`<= 0.79` u delta) on `SM-X730`.
- `[2026-11-17]` **Round-2 Review, PR #461 Merge & 3-Way Stutter PRs (#481, #482, #483):** Resolved `_WIN32`/test findings; `#461` merged; opened `#481`–`#483`.
- `[2026-11-17]` **Pre-PR Subagent Review Gate & `CPH2653` Turnip A/B (`a1ef820`):** `30.17 fps`, wait `-82.9%`, gpipes `-35.9%`, `99.8%` `.smeta`. See `MALI_PROGRESS.md`.
