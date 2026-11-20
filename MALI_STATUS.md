# MALI Status

Last updated: `2026-11-20` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.8-beta`** (`2026-11-20T11:51:32Z`, *sync pending*; merged **PR `#508`** `3d28f80`).
- **Fork `main`:** `2c72f20d542e44ef3350c2c05cb036d7fb1dfcb8` (`main == origin/main == v0.4.7-beta`, released `2026-11-18`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `2c72f20` | Clean `v0.4.7-beta` baseline & docs |
| `.mali/worktrees/fullscreen-narrow` | `feat/fullscreen-narrow` | `25ebfb6` | **OPEN PR #504** (`MERGEABLE`, narrow landscape Unlocked / `#474`) |
| `.mali/worktrees/cbwin-mali-snap` | `feat/cbwin-mali-snap` | `26c4a5a` | **OPEN PR #506** (`CONFLICTING` w/ merged `#508`, `cbwin` + Mali copy) |
| `.mali/worktrees/mali-inplace-feedback` | `feat/mali-inplace-feedback` | `0769b0e` | **OPEN PR #507** (`CONFLICTING`, Mali `r49+` auto `inplace_reads` + i18n) |
| `.mali/worktrees/gcopy-dchain` | `feat/gcopy-dchain` | `7845d58` | **MERGED PR #508** (`3d28f80`, `gcopy_mode` dispatch chaining; ready to retire) |
| `.mali/worktrees/mali-release-047` | `release/mali-0.4.7-beta` | `b6ed7e0` | Consolidated `v0.4.7-beta-mali` build (`#504`, `#506`, `#507`, `#508`) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (`3/3` starter heights verified, `MERGEABLE`) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **Galaxy Tab S11 (SM-X730)** *(connected)* | Mali-G925 MC12 (`r49p1` stock Mali) | **`v0.4.7-beta` Mali (`b6ed7e0`)** (`5eb931e0...3ffe`) | `2026-11-20`: `68/68` saves (`diffs=0`), `3/4` prefs (`[audit] CLEAN`), `4834` `.spv`; idle |
| **OnePlus 13 (CPH2653)** *(unplugged)* | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** only) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `83/84` (`0` save/INI diffs); `1865` `.spv`; verified prior to disconnect |

Rollback APKs under `.mali/apk/`: `v0.4.6-beta-mali` (`3dc08f7b...0aab`), `v0.4.6-beta` (`aa9ca25b...3e27`), `v0.4.5-beta` (`65ecd82f...c29b`), `v0.4.3-beta` (`3d3ad8e7...10846`). Evidence branch: `origin/evidence/pr504-pr508-20261120` (`4c74afc`). Scope: `SM-X730` & `CPH2653` (Turnip-only).

## Open PRs & Active Priorities

- **Merged (`v0.4.8-beta` / `v0.4.7-beta`):** **`#508`** (`3d28f80`), `#481`–`#483`, `#485` (earlier: `#315`, `#436`, `#461`, `#475`).
- **Open:** **`#504`** (`25ebfb6`, `MERGEABLE`, review asks `fovy` creep check), **`#506`** (`26c4a5a`, `CONFLICTING`, rebase onto `v0.4.8-beta` & address `gcopy_gen`/water notes), **`#507`** (`0769b0e`, `CONFLICTING`, rebase & reply on `r49+` evidence / split), **`#480`** (`c48d1b1`, `MERGEABLE`).
- **Immediate Queue:** Sync `main` to `v0.4.8-beta`, retire `#508` worktree, answer `mamcjr` reviews & rebase `#506`/`#507`, audit `#504` `fovy` creep, then continue Master Plan **Item 3** (LMKD survival `12b–12d/12f`) & **Phase 2** (`VK_KHR_dynamic_rendering_local_read`).
- **Deferred (`2026-11-20`):** Mali grass striping (`477912f75d2e8819`, needs `CPH2653` Turnip 1:1) & 8BitDo Pro 2 gyro (`#338`).

## Recent Milestones

- `[2026-11-20]` **PR #504–#508 Isolated Evidence, #506/#507 Hardening & #508 Merged:** Hardened `#506` (`26c4a5a`) & `#507` (`0769b0e` `Mali r49+` gating); ran isolated `SM-X730` 3-pass benchmarks & published 9 PNGs (`evidence/pr504-pr508-20261120`); deployed `b6ed7e0` (`5eb931e0...3ffe`, `8/8` Bionic, `68/68` saves CLEAN); `#508` merged in `v0.4.8-beta`.
- `[2026-11-20]` **Mali Perf Phase 1 & Decal Probe + PR #504:** Opened `#504` (`#474`) & `#506`–`#508`; captured `zora-plaza` `[gpuprof]` (`9.230 ms`, `8` breaks) + `558` GLSL; synced `v0.4.7-beta` (`2c72f20`).
- `[2026-11-18]` **`v0.4.6-beta`, PRs `#481–#485` Fixes & Mali Rollout:** FF `0897fb8`; resolved reviews; `1,186/1,186` `0`-drain (`#485`) & `ABBA` Mali perf (`cbwin_snap`+`inplace`+`dchain`). See `MALI_PROGRESS.md`.
