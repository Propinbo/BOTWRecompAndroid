# MALI Status

Last updated: `2026-11-16` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.5-beta`** (`6113c61a1d0e6f78532eb7fdde9cc9348613ad4b`, `2026-11-16T18:47:36Z`): restart shader free (resolves parked `12e`), `#450` Mali shader eviction `>1000`, MediaTek `13 -> 19.2 MHz` Native clock fix (untested on hardware), full-span copy texture hash, vertex cache retention; `#296` still listed in Known Issues.
- **Fork `main`:** Fast-forwarded to **`v0.4.5-beta`** (`6113c61a1d0e6f78532eb7fdde9cc9348613ad4b`), clean tree (`main == origin/main == upstream/main`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `6113c61` | Clean `v0.4.5-beta` baseline & workspace docs |
| `.mali/worktrees/fix-rt-load-upload-296` | `fix/rt-load-upload-296` | `0230d2f` | **OPEN PR #461** (`#296`/`#438` on `280048bc`) |
| `.mali/worktrees/soft-linear-upstream` | `fix/soft-linear-filter` | `ab828a1` | **OPEN PR #436** (`soft_linear` on `5946454c`) |
| `.mali/worktrees/pr315-benchmark-v2` | `pr315-benchmark-v2` | `674c8b0` | **OPEN PR #315** (`origin/benchmark-v2`) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `7b69caa` | Unpushed 11-scene set (`44360ae..7b69caa`) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **SM-X730** | Mali-G925 MC12 (stock Mali) | `v0.4.3-beta` (`3d3ad8e7...10846`) | `2026-11-16`: `69/69` (`diffs=0`, `ActivityThread.IDS.xml` system-updated); `3047` SPIR-V |
| **CPH2653** | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** `2bbb279a...` only) | `v0.4.3-beta` (`3d3ad8e7...10846`) | `2026-11-16`: `84/84` (`diffs=0`); `1865` SPIR-V |
| **SM-X926B** | Immortalis-G720 MC12 (`r44p1`, no Turnip) | Official `v0.3.4-beta` (`7b94efcc...3dcb`) | Setup `2026-11-12` (`43/43` payload, `botwgame.so`) |

Rollback APK: `.mali/apk/v0.4.3-beta/BOTWRecompAndroid-0.4.3-beta-signed.apk` (`3d3ad8e7...10846`).

## Open PRs & Active Priorities

- **PR #461** (`OPEN`, `MERGEABLE`/`CLEAN` vs `6113c61a`): `#296`/`#438` writeback RT sync & 128B linear pitch (`0230d2f`).
- **PR #436** (`OPEN`, `MERGEABLE`/`CLEAN` vs `6113c61a`): `soft_linear` fallback for formats without HW linear support (`ab828a1`).
- **PR #315** (`OPEN`, `CONFLICTING`): Benchmark v2 stalls/cooling/timing (`674c8b0`); needs rebase before `benchmark-scenes-v2` (`7b69caa`).
- **Next:** Paired device update to `v0.4.5-beta` (`SM-X730` + `CPH2653` when both attached) & testing MediaTek `13 MHz` Native clock + `#450` shader eviction on `SM-X730`. Parked: `Fullscreen 16:10`, `Memory 12b–12d/12f`, `Benchmark 11c–11d`.

## Recent Milestones

- `[2026-11-16]` **Workspace Cleanup, `main` FF & Antigravity Migration:** Freed `57.59 GiB`, purged local legacy agent remnants/refs, fast-forwarded `main` to `v0.4.5-beta` (`6113c61a`), synced `mali-workspace` checkpoint. See `MALI_PROGRESS.md`.
- `[2026-11-16]` **PR #461 & `#296`/`#438` Root Cause:** Fixed missing `lv=0` RT upload & Case 1/2 sync (`0230d2f`); `19/19` Bionic/host, `SM-X730` (`93` WB, `30.0 fps`) & `CPH2653` Turnip (`93` WB, `40.0 fps`). See `.mali/underwater-walk-296/upstream-pr/REPORT.md`.
- `[2026-11-15]` **Mali RAM Audits (30-min & Entry vs Turnip):** `VmRSS +625.06 MiB`, `0` passive leak; `3.4 GB` vs `1.5–2.0 GB` attributed (`placed`, `mtrack`, `BC 0`, `scudo`); Turnip-only rule added. See `.mali/memory-long-mali-20261115/REPORT.md`.
- `[2026-11-15]` **PR #436 (`soft_linear`) & `v0.4.3-beta` Update:** Fixed Mali Dueling Peaks water cube (`ab828a1`); updated `SM-X730` & `CPH2653` to `v0.4.3-beta`. See `.mali/water-artifact/upstream-pr/REPORT.md`.
