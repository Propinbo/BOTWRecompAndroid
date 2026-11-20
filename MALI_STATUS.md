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
| `.mali/worktrees/mali-perf-046` | `feat/mali-perf-046` | `4ddf99b` | Mali `cbwin_snap`, `inplace_reads` & `dchain` (reconcile vs `#453` on `2c72f20`) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (`3/3` starter heights verified, `MERGEABLE`) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **Galaxy Tab S11 (SM-X730)** *(connected)* | Mali-G925 MC12 (`r49p1` stock Mali) | `v0.4.6-beta` Mali (`3dc08f7b...0aab`) | `2026-11-20`: `68/68` saves, `3/4` prefs (`[post] CLEAN`), `4834` `.spv`; idle |
| **OnePlus 13 (CPH2653)** *(unplugged)* | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** only) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `83/84` (`0` save/INI diffs); `1865` `.spv`; verified prior to disconnect |

Rollback APKs: signed `v0.4.6-beta` (`aa9ca25b...3e27`), `v0.4.5-beta` (`65ecd82f...c29b`), & `v0.4.3-beta` (`3d3ad8e7...10846`) under `.mali/apk/`. Device scope strictly pinned to owner's 2 devices (`SM-X730` & `CPH2653` Turnip-only).

## Open PRs & Active Priorities

- **Merged upstream (`v0.4.7-beta`):** `#481`, `#482`, `#483`, `#485` (earlier: `#315`, `#436`, `#461`, `#475`).
- **Open (`MERGEABLE`):** **`#504`** (`25ebfb6`, narrow landscape `#474`) & **`#480`** (`c48d1b1`).
- **Ready locally (`feat/mali-perf-046` `4ddf99b`):** `+6.6%..+11.1%` 60 FPS on `SM-X730` (`upload -83.5%`, `fbcopy 650->0`); rebase onto `2c72f20`, reconcile `cbwin` vs `#453`, and split PRs (`cbwin_snap`, `inplace_reads`, `dchain`).
- **Active queue (Master Plan Spheres 1–5, 7–8; Sphere 6 excluded):** (1) **PR #504 (`#474`) Turnip cross-check / `v0.4.7-beta-mali` integration** (`16:10` verified on `SM-X730`, pending `CPH2653` Turnip cross-check when attached); (2) **Rebase/split `feat/mali-perf-046` + `v0.4.7-beta-mali` APK (Sphere 2)**; (3) **Background/HOME LMKD survival `12b–12d/12f` (Sphere 3)**.
- **Deferred per owner (`2026-11-20`):** Mali grass row striping (`477912f75d2e8819` isolated, `c2..c7` neutral, awaiting `CPH2653` Turnip 1:1 / screenshot) & 8BitDo Pro 2 gyro (`#338`).

## Recent Milestones

- `[2026-11-20]` **PR #504 Opened — Narrow-Landscape Unlocked Aspect (`#474`):** Shipped `25ebfb6` on `v0.4.7-beta`; 2-round 3-verifier review + TSAN (`0` races) + `SM-X730` 8-scenario & `7/7` Bionic PASS.
- `[2026-11-20]` **`v0.4.7-beta` Sync (`#481–#485` Merged), Grass Triage (Parked) & Master Plan:** FF `main` to `2c72f20`; retired 5 merged/superseded worktrees (`worktrees=3`); pinned 2-device scope (`SM-X730` & `CPH2653`); saved Master Plan (Spheres 1–5, 7–8).
- `[2026-11-18]` **PR #481–#485 Maintainer Review Fixes & Mali Stress Validation:** Resolved all reviews (`WD_R_BUILD_WAIT`, build ID + 88B `ShaderKey`, Turnip/ARM r49+ gate); verified `SM-X730` A/B & `1,186/1,186` `0`-drain windows (`#485`).
- `[2026-11-18]` **`v0.4.6-beta` Sync, PR #485, 60 FPS Mali Perf `ABBA` & Release Rollout:** FF `0897fb8`; opened `#485`; verified `cbwin_snap`+`inplace`+`dchain`.
- `[2026-11-17]` **PR #480 Starter-Scene Check & Cleanup:** Retired `#461`; verified `3/3` starter heights (`<= 0.79` u delta). See `MALI_PROGRESS.md`.
