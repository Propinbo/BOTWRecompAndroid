# MALI Status

Last updated: `2026-11-20` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.7-beta`** (`2c72f20d542e44ef3350c2c05cb036d7fb1dfcb8`, `2026-11-18`), merging PRs `#481`, `#482`, `#483`, `#485`, `#452`, `#453` (`cbwin` snapshot), `#455`, `#456`, `#457` (Shadows), `#471/#478` (`MADV_PAGEOUT` map fix), & `#401` thread dump.
- **Fork `main`:** `2c72f20d542e44ef3350c2c05cb036d7fb1dfcb8` (`main == origin/main == v0.4.7-beta`, synced).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `2c72f20` | Clean `v0.4.7-beta` baseline & docs |
| `.mali/worktrees/fullscreen-narrow` | `feat/fullscreen-narrow` | `25ebfb6` | **OPEN PR #504** (narrow landscape Unlocked aspect / `#474`) |
| `.mali/worktrees/cbwin-mali-snap` | `feat/cbwin-mali-snap` | `5aec777` | **OPEN PR #506** (`cbwin` snapshot cache + Mali `gcopy_mode`) |
| `.mali/worktrees/mali-inplace-feedback` | `feat/mali-inplace-feedback` | `a3f8763` | **OPEN PR #507** (automatic Mali `inplace_reads` + 11-locale i18n) |
| `.mali/worktrees/gcopy-dchain` | `feat/gcopy-dchain` | `7845d58` | **OPEN PR #508** (`gcopy_mode` compute dispatch barrier chaining) |
| `.mali/worktrees/mali-release-047` | `release/mali-0.4.7-beta` | `bce7d47` | Integrated `v0.4.7-beta-mali` build (`#504`, `#506`, `#507`, `#508`) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (`3/3` starter heights verified, `MERGEABLE`) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **Galaxy Tab S11 (SM-X730)** *(connected)* | Mali-G925 MC12 (`r49p1` stock Mali) | **`v0.4.7-beta` Mali** (`056244d6...ea47`) | `2026-11-20`: `68/68` saves (`diffs=0`), `3/4` prefs (`[post] CLEAN`), `4834` `.spv`; idle |
| **OnePlus 13 (CPH2653)** *(unplugged)* | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** only) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `83/84` (`0` save/INI diffs); `1865` `.spv`; verified prior to disconnect |

Rollback APKs under `.mali/apk/`: `v0.4.6-beta-mali` (`3dc08f7b...0aab`), `v0.4.6-beta` (`aa9ca25b...3e27`), `v0.4.5-beta` (`65ecd82f...c29b`), `v0.4.3-beta` (`3d3ad8e7...10846`). Scope: `SM-X730` & `CPH2653` (Turnip-only).

## Open PRs & Active Priorities

- **Merged (`v0.4.7-beta`):** `#481`, `#482`, `#483`, `#485` (earlier: `#315`, `#436`, `#461`, `#475`).
- **Open (`MERGEABLE`):** **`#506`** (`5aec777`), **`#507`** (`a3f8763`), **`#508`** (`7845d58`), **`#504`** (`25ebfb6`), **`#480`** (`c48d1b1`).
- **Queue (Spheres 1–5, 7–8; excl. 6):** (1) **PR #504 (`#474`)** in `v0.4.7-beta-mali` on `SM-X730` (needs `CPH2653` Turnip check); (2) **Mali Perf Phase 1 DONE (`#506`–`#508`) -> Phase 2 (`VK_KHR_dynamic_rendering_local_read` decals)** design (`decal-probe/` `9.230 ms`, `8` breaks, `558` GLSL); (3) **LMKD survival `12b–12d/12f` (Sphere 3)**.
- **Deferred (`2026-11-20`):** Mali grass striping (`477912f75d2e8819`, needs `CPH2653` Turnip 1:1) & 8BitDo Pro 2 gyro (`#338`).

## Recent Milestones

- `[2026-11-20]` **PRs #506–#508, `v0.4.7-beta-mali` & Decal Probe (Phase 1 DONE):** Audited (`0 CRIT/0 MAJ`) `#506`–`#508`; deployed `bce7d47` (`056244d6...ea47`) to `SM-X730` (`8/8`+`4/4` Bionic, `zora-plaza`/`kakariko-day` 16:10, `68/68` saves CLEAN); captured `[gpuprof]` + `558` GLSL; retired `mali-perf-046`.
- `[2026-11-20]` **PR #504 (`#474`) Opened:** Narrow-landscape Unlocked (`25ebfb6`), 2-round review, TSAN (`0` races), `SM-X730` 8-scene & `7/7` Bionic PASS.
- `[2026-11-20]` **`v0.4.7-beta` Sync (`#481–#485` Merged):** FF `main` to `2c72f20`; retired 5 worktrees; pinned 2-device scope & Master Plan.
- `[2026-11-18]` **`v0.4.6-beta`, PRs `#481–#485` Fixes & Mali Rollout:** FF `0897fb8`; resolved reviews; `1,186/1,186` `0`-drain (`#485`) & `ABBA` Mali perf (`cbwin_snap`+`inplace`+`dchain`). See `MALI_PROGRESS.md`.
