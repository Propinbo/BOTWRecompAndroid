# MALI Status

Last updated: `2026-11-18` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.6-beta`** (`0897fb889719d0489a7bcbd8dba320b997d377ce`), including `#436`, `#475`, and `#401` freeze watchdog.
- **Fork `main`:** `0897fb889719d0489a7bcbd8dba320b997d377ce` (`main == origin/main == v0.4.6-beta`, synced).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `0897fb8` | Clean `v0.4.6-beta` baseline & docs |
| `.mali/worktrees/mali-release-046` | `release/mali-0.4.6-beta` | `d8fe781` | Integrated Mali Release (`011e1da` + `#481–#483` + `#485` + `perf-046`) |
| `.mali/worktrees/mali-perf-046` | `feat/mali-perf-046` | `4ddf99b` | Stacked Mali `cbwin_snap`, in-place feedback & `dchain` |
| `.mali/worktrees/mali-async-tex-slots` | `feat/mali-async-tex-slots` | `8bb3c02` | **OPEN PR #485** (`1,186/1,186` windows `0` drains, `MERGEABLE`) |
| `.mali/worktrees/urgent-shader-builds` | `feat/urgent-shader-builds` | `e8fac0f` | **OPEN PR #481** (rebased on `011e1da`, `WD_R_BUILD_WAIT`, `MERGEABLE`) |
| `.mali/worktrees/shader-meta-cache` | `feat/shader-meta-cache` | `b4e80e7` | **OPEN PR #482** (stacked on `e8fac0f`, build ID + 88B key, `MERGEABLE`) |
| `.mali/worktrees/dyn-vertex-input` | `feat/vk-dyn-vertex-input` | `8146cfa` | **OPEN PR #483** (rebased on `011e1da`, Turnip/ARM r49+, `MERGEABLE`) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (`3/3` starter heights verified, `MERGEABLE`) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **SM-X730** | Mali-G925 MC12 (`r49p1` stock Mali) | `v0.4.6-beta` Mali (`3dc08f7b...0aab`) | `2026-11-18`: `67/67` saves + `0` non-sys pref diffs (`3/4`); `3047` `.spv`; idle |
| **CPH2653** *(unplugged)* | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** only) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `83/84` (`0` save/INI diffs); `1865` `.spv`; verified prior to disconnect |
| **SM-X926B** | Immortalis-G720 MC12 (`r44p1`, no Turnip) | Official `v0.3.4-beta` (`7b94efcc...3dcb`) | Setup `2026-11-12` (`43/43` payload, `botwgame.so`) |

Rollback APKs: signed `v0.4.6-beta` (`aa9ca25b...3e27`), `v0.4.5-beta` (`65ecd82f...c29b`), & `v0.4.3-beta` (`3d3ad8e7...10846`) under `.mali/apk/`.

## Open PRs & Active Priorities

- **Merged in `v0.4.6-beta`:** `#315`, `#436`, `#461`, `#475`.
- **Open (`MERGEABLE`/`CLEAN` against `upstream/main` `011e1da`):** **`#480`** (`c48d1b1`), **`#481`** (`e8fac0f`), **`#482`** (`b4e80e7` stacked on `#481`), **`#483`** (`8146cfa`), **`#485`** (`8bb3c02`).
- **Ready locally (`origin/feat/mali-perf-046` `4ddf99b`):** Cooled 60 FPS `ABBA` on `SM-X730` measured **`+6.6%..+11.1%` FPS** (`opt_1` peak **`+20.1%`**, `gpu_p50 -8.5%..-27.2%`, `upload -83.5%`, `fbcopy 650->0`, `dchained 43–46%`), zero artifacts; `12/12` Bionic tests PASS on `release/mali-0.4.6-beta` (`d8fe781`).
- **Next:** Monitor upstream merges across `#480`–`#483` & `#485`; split `feat/mali-perf-046` into clean PRs (`dchain`, Mali `inplace_reads`, `cbwin_snap`) after `#482`/`#485` land. Parked: `Fullscreen 16:10`, `Memory 12b–12d/12f`, `Benchmark 11c–11d`.

## Recent Milestones

- `[2026-11-18]` **PR #481–#485 Maintainer Review Fixes & Mali Stress Validation:** Addressed all reviews (`WD_R_BUILD_WAIT`, build ID + 88B `ShaderKey` + `mw_meta_exists()`, Turnip/ARM r49+ gate); verified `SM-X730` A/B & `1,186/1,186` `0`-drain windows (`#485`); rebuilt/installed `0.4.6-beta-mali` (`3dc08f7b...0aab`); posted GitHub replies.
- `[2026-11-18]` **`v0.4.6-beta` Sync, PR #485 Async Slots, 60 FPS Mali Perf `ABBA` & Release Rollout:** FF `main` (`0897fb8`); opened `#485`; verified `cbwin_snap`+`inplace`+`dchain`; retired `0.4.5` worktrees.
- `[2026-11-17]` **Cleanup, Starter-Scene Check (#480) & Reviews (#436/#475):** Retired `#461` worktree; verified `3/3` starter scenes (`<= 0.79` u delta) on `SM-X730`.
- `[2026-11-17]` **Round-2 Review, PR #461 Merge & 3-Way Stutter PR Split (#481, #482, #483):** Resolved `_WIN32`/test findings; `#461` merged; opened `#481`–`#483`. See `MALI_PROGRESS.md`.
