# MALI Status

Last updated: `2026-11-21` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.9-beta`** (`2026-11-21T04:30:22Z`, `7f5afab7`; merged **#504**, **#506**, & Switch core turns).
- **Fork `main`:** `7f5afab71f122f1622401c0f11775e40f78df0e2` (`main == origin/main == upstream/main == v0.4.9-beta`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `7f5afab` | Clean `v0.4.9-beta` baseline & docs |
| `.mali/worktrees/tile-local-read` | `feat/vk-local-read` | `9e75f99` | **OPEN PR #518** (`MERGEABLE`, `VK_KHR_dynamic_rendering_local_read`) |
| `.mali/worktrees/mali-inplace-feedback` | `feat/mali-inplace-feedback` | `2e36682` | **OPEN PR #507** (`MERGEABLE`, `7f5afab` rebase + 11-locale Mali UI note) |
| `.mali/worktrees/mali-release-048` | `release/mali-0.4.8-beta` | `5d93424` | Consolidated `v0.4.8-beta-mali` build (`#504`, `#506`, `#507`) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (`3/3` starter heights verified, `MERGEABLE`) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **Galaxy Tab S11 (SM-X730)** *(connected)* | Mali-G925 MC12 (`r49p1` stock Mali) | **`v0.4.8-beta` Mali (`5d93424`)** (`0c9f21f9...868d`) | `2026-11-21`: `68/68` saves (`diffs=0`), `4/4` prefs (`diffs=0`), `7417` `.spv`, `vk_pipelines.bin=b969f9af...`; idle |
| **OnePlus 13 (CPH2653)** *(unplugged)* | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** only) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `83/84` (`0` save/INI diffs); `1865` `.spv`; **PR #518 untested on Turnip** |

Rollback APKs under `.mali/apk/`: `v0.4.7-beta-mali`, `v0.4.6-beta-mali`, `v0.4.6-beta`, `v0.4.5-beta`, `v0.4.3-beta`. Evidence: `origin/evidence/pr-local-read-20261120` (`abf2ec4`), `origin/evidence/pr504-pr508-20261120` (`a1cb3dd`).

## Open PRs & Active Priorities

- **Merged:** **`#504`** (`5df9dd9`), **`#506`** (`c02beea`), **`#508`** (`3d28f80`), `#481`–`#485` (`#315`, `#436`, `#461`, `#475`).
- **Open (`MERGEABLE`):** **`#518`** (`9e75f99`, Mali `r49+` local-read), **`#507`** (`2e36682`, `7f5afab` rebase + Mali UI note), **`#480`** (`c48d1b1`).
- **Immediate Queue:** Await `mamcjr` review on `#507`/`#518`; verify `#518`/`#504` on `CPH2653` Turnip when plugged; advance **Graphics Accuracy** fixes (`#486`, grass writeback, `#244`).
- **Deferred (`2026-11-20`):** Android LMKD (`12b–12d/12f`), Mali grass striping (`477912f75d2e8819`), & 8BitDo Pro 2 gyro (`#338`).

## Recent Milestones

- `[2026-11-21]` **`v0.4.9-beta` Sync, Worktree Cleanup & PR #507 Update (`2e36682`):** FF `main` to `7f5afab7`; retired `#504`/`#506` worktrees; rebased/updated `#507` w/ Mali Turnip note in 11 locales (`0` CRIT/MAJ/MIN, `:app:assembleDebug` + `8/8` Bionic); PR `#507` comment posted.
- `[2026-11-21]` **Tile-Local Decals Phase 2 & PR #518 (`9e75f99`):** Implemented & verified `VK_KHR_dynamic_rendering_local_read` on Mali `r49+` (`0` breaks, `-70%/-90%` `fbcopy`, `+5.1%` Zora 60p); opened **PR #518** (`MERGEABLE`); saved `2026-11-21` backup & restored `v0.4.8-beta-mali`.
- `[2026-11-20]` **v0.4.8-beta Sync & `v0.4.8-beta-mali` Rollout:** Synced `main` (`2b71a10`), hardened `#504`/`#506`/`#507` (`10` PNGs `a1cb3dd`), rolled out `5d93424`.
- `[2026-11-20]` **PR #504–#508 Evidence & #508 Merged:** `#506`–`#508` isolated evidence (`4c74afc`); `#508` merged (`3d28f80`). See `MALI_PROGRESS.md`.
