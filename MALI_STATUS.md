# MALI Status

Last updated: `2026-11-25` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.5.0-beta`** (`2026-11-25T14:30:28Z`, `d68d698`, `versionCode 53`; `upstream/main` @ `208746bb`).
- **Fork `main`:** `208746bb8a0d2200d8130cc6afe9fee109e4bd80` (`main == origin/main == upstream/main`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `208746b` | Clean `v0.5.0-beta` / `upstream/main` baseline & docs |
| `.mali/worktrees/pr-mali-shader-cleanup` | `feat/mali-spirv-cleanup-discard` | `7517817` | **OPEN PR #582** (`CLEANUP` + `1:1` `DISCARD`) |
| `.mali/worktrees/mali-shader-opt` | `feat/mali-shader-opt` | `d63e3a0`.. | **Track 5 Phase 2 parked** (`RelaxedPrecision`, `PHASE2_AB_REPORT.md`) |
| `.mali/worktrees/mali-bc-astc` | `feat/mali-bc-astc` | `d63e3a0` | **OPEN PR #579** (`BC3`/`BC5_UNORM` -> `ASTC 4x4` + `r49+` `UNORM8`) |
| `.mali/worktrees/mali-direct-heap` | `feat/mali-direct-heap` | `f856e65` | **Phase 4A/3B checkpoint** (`DIRECT_HEAP=0`, opt-in `-1.68 GB` RAM) |
| `.mali/worktrees/mali-cpu-thermals` | `feat/mali-cpu-thermals` | `0a89336` | CPU/thermals P1+P2 (`300s` `40fps` `PASS`) |
| `.mali/worktrees/fix-android-nintendo-gyro` | `fix/android-nintendo-gyro` | `6f30e81` | **OPEN PR #545** (axes/clock/accel/bias) |
| `.mali/worktrees/fix-narrow-shadow-539` | `fix/narrow-shadow-539` | `d27739c` | **OPEN PR #543** (`Fixes #539` Vert+ shadows) |
| `.mali/worktrees/warm-cache-pipe-fastpath` | `feat/vk-warm-cache-fastpath` | `87ba2a4` | **OPEN PR #529** (FPS-scaled fastpath + peek) |
| `.mali/worktrees/op13-turnip-opt` | `feat/op13-turnip-opt` | `7f4cc9a` | **Installed `v0.4.10-beta-op13`** (`Turnip` opt) |
| Merged/Archived (`11` wts) | `#534`/`#531`/`#480`/`#507`/`#537`/`#518`, `release/mali-0.4.{8..12}` | `c1a29d9`.. | Merged/closed/superseded |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **Tab S11 (SM-X730)** *(connected)* | Mali-G925 (`r49p1`) | **`v0.5.0-beta`** (`208746bb`, `v53`, `81aaf2cf`, stopped) | `2026-11-25/mali`: `66/66` saves, `4/5` prefs, `19643` `.spv`, `ac048f10`; clean |
| **OP13 (CPH2653)** *(unplugged)* | Adreno 830 (**Turnip V38**) | **`v0.4.10-beta-op13`** (`63a4195f`, `60fa2a5f`) | `2026-11-21`: `83/84` saves, `2707` `.spv`, `725a99243c7f`; stopped |

Rollback APKs (`.mali/apk/`): `v0.4.12-beta-mali` (`8eedca1a`), `pre-review-20261124` (`196c94c6`), `v0.4.11-beta-mali` (`881201b0`), `v0.4.10-op13` (`63a4195f`).

## Open PRs, Key Findings & Queue

- **Merged:** `#534`, `#531`, `#537`, `#480`, `#507`, `#504`, `#506`, `#508`, `#481`–`#485`. **Open (`MERGEABLE`):** `#582`, `#579`, `#545`, `#543`, `#529`.
- **Mali-G925 `r49p1` notes:** `vertexPipelineStoresAndAtomics=0` (FS+CS only); avoid periodic `mprotect` demotion; `dma-buf type 1` adds `+1.85 °C` AP / `+2.25 °C` Skin hardware floor over `copy-mode` (`PHASE4A_AUDIT.md`).

## Recent Milestones

- `[2026-11-25]` **Track 5 P1 PR #582 (`feat/mali-spirv-cleanup-discard` @ `7517817` atop `upstream/main` `208746b`):** Strictly `1:1` `CLEANUP` + `DISCARD` (P2 `FP16` parked); `-27.38%` `.spv` (`5094/5094` `spirv-val`), `5000` MSL `0` diffs, Release smoke A/B on SM-X730 (`-11.7%` scratch cache, `0` errors, `zora-plaza` `30.03 FPS`/`29.88 ms`, visual parity `PASS`).
- `[2026-11-25]` **Track 2 PR #579 (`feat/mali-bc-astc` @ `d63e3a0` atop `v0.4.13-beta`):** `BC3`/`BC5_UNORM` -> `ASTC 4x4` + Mali `r49+` `VK_EXT_astc_decode_mode` (`UNORM8`); `-127..134 MB (-15.5%..-16.2%)` peak tex VRAM, `0` streaming hitches, default-on `SM-X730` `PASS`.
- `[2026-11-25]` **Direct Heap P2–4A (`f856e65`):** `dma-heap` `RSS -1.68 GB` vs `copy`, `dma-buf type 1` `+1.85..2.25 °C` floor isolated (`DIRECT_HEAP=0`).
- `[2026-11-24]` **`v0.4.12-beta-mali` (`f87d238`):** Consolidated `79ba1909` + `#529`/`#531`/`#534`/`#543`/`#545` + thermals (`PASS`).
