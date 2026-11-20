# MALI Status

Last updated: `2026-11-20` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.8-beta`** (`2026-11-20T11:51:32Z`, merged **PR `#508`** `3d28f80`).
- **Fork `main`:** `2b71a10c934e52780fc769b6c8b50de3278a7c2c` (`main == origin/main == v0.4.8-beta`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `2b71a10` | Clean `v0.4.8-beta` baseline & docs |
| `.mali/worktrees/fullscreen-narrow` | `feat/fullscreen-narrow` | `186ea04` | **OPEN PR #504** (`MERGEABLE`, narrow landscape Unlocked / `#474`) |
| `.mali/worktrees/cbwin-mali-snap` | `feat/cbwin-mali-snap` | `adbafd6` | **OPEN PR #506** (`MERGEABLE`, `cbwin` + isolated `cbwin_retire_*`) |
| `.mali/worktrees/mali-inplace-feedback` | `feat/mali-inplace-feedback` | `fe2f659` | **OPEN PR #507** (`MERGEABLE`, Mali `"On"` opt-in `inplace_reads`) |
| `.mali/worktrees/mali-release-048` | `release/mali-0.4.8-beta` | `5d93424` | Consolidated `v0.4.8-beta-mali` build (`#504`, `#506`, `#507`) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (`3/3` starter heights verified, `MERGEABLE`) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **Galaxy Tab S11 (SM-X730)** *(connected)* | Mali-G925 MC12 (`r49p1` stock Mali) | **`v0.4.8-beta` Mali (`5d93424`)** (`0c9f21f9...868d`) | `2026-11-20-b`: `68/68` saves (`diffs=0`), `3/4` prefs (`[audit] CLEAN`), `6729` `.spv`; idle |
| **OnePlus 13 (CPH2653)** *(unplugged)* | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** only) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `83/84` (`0` save/INI diffs); `1865` `.spv`; verified prior to disconnect |

Rollback APKs under `.mali/apk/`: `v0.4.7-beta-mali` (`5eb931e0...3ffe`), `v0.4.6-beta-mali` (`3dc08f7b...0aab`), `v0.4.6-beta` (`aa9ca25b...3e27`), `v0.4.5-beta` (`65ecd82f...c29b`), `v0.4.3-beta` (`3d3ad8e7...10846`). Evidence branch: `origin/evidence/pr504-pr508-20261120` (`a1cb3dd`). Scope: `SM-X730` & `CPH2653` (Turnip-only).

## Open PRs & Active Priorities

- **Merged (`v0.4.8-beta` / `v0.4.7-beta`):** **`#508`** (`3d28f80`), `#481`–`#483`, `#485` (earlier: `#315`, `#436`, `#461`, `#475`).
- **Open (`MERGEABLE`):** **`#504`** (`186ea04`, `mAngle` preserved), **`#506`** (`adbafd6`, `#508` rebase + isolated `cbwin_retire_*` + Mali `[cbuf]`/water), **`#507`** (`fe2f659`, strict `"On"` setting + Title/Plateau/Zora A/B), **`#480`** (`c48d1b1`).
- **Immediate Queue:** Wait for `mamcjr` review/merge of `#504`/`#506`/`#507`, then continue **Phase 2** (`VK_KHR_dynamic_rendering_local_read`) or **Graphics Accuracy** fixes.
- **Deferred (`2026-11-20`):** **Android LMKD survival (`12b–12d / 12f`)**, Mali grass striping (`477912f75d2e8819`, needs `CPH2653` Turnip 1:1), & 8BitDo Pro 2 gyro (`#338`).

## Recent Milestones

- `[2026-11-20]` **v0.4.8-beta Sync, PR #504/#506/#507 Review Fixes & `v0.4.8-beta-mali` Rollout:** Synced `main` to `2b71a10`; retired `gcopy-dchain` & `mali-release-047`; hardened/rebased `#504` (`186ea04`), `#506` (`adbafd6`), `#507` (`fe2f659`); published 10 `*_v048_*.png` + replied on GitHub (`OPEN, MERGEABLE`); deployed `5d93424` (`0c9f21f9...868d`, `8/8` Bionic, `68/68` saves CLEAN).
- `[2026-11-20]` **PR #504–#508 Isolated Evidence, #506/#507 Hardening & #508 Merged:** Hardened `#506` (`26c4a5a`) & `#507` (`0769b0e`); ran `SM-X730` A/B & published 9 PNGs (`4c74afc`); `#508` merged in `v0.4.8-beta`.
- `[2026-11-20]` **Mali Perf Phase 1 & Decal Probe + PR #504:** Opened `#504` & `#506`–`#508`; captured `zora-plaza` `[gpuprof]` (`9.230 ms`, `8` breaks) + `558` GLSL; synced `v0.4.7-beta`.
- `[2026-11-18]` **`v0.4.6-beta`, PRs `#481–#485` Fixes & Mali Rollout:** FF `0897fb8`; `1,186/1,186` `0`-drain (`#485`) & `ABBA` Mali perf. See `MALI_PROGRESS.md`.
