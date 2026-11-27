# BOTWRecompAndroid Platform Status

Last updated: `2026-11-27` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.5.1-beta`** (`da30d45dac467ce1b7d65e034cc33560ff67ae94`, `versionCode 54`; `upstream/main`).
- **Fork `main`:** `da30d45dac467ce1b7d65e034cc33560ff67ae94` (`main == origin/main == upstream/main`).
- **Pre-pivot safety tags:** `backup/pre-odin3-pivot` (`208746b`), `backup/pre-odin3-pivot-mali-40fps` (`109f6d7`).

## Active Worktrees & Branches

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `da30d45` | Clean `v0.5.1-beta` baseline & `.agents/`/`.mali/` docs |
| `.mali/worktrees/warm-cache-pipe-fastpath` | `feat/vk-warm-cache-fastpath` | `63c9e1a` | **OPEN PR #529** (rebased `da30d45`, FPS-scaled fastpath + peek) |
| `.mali/worktrees/fix-narrow-shadow-539` | `fix/narrow-shadow-539` | `6bccdbf` | **OPEN PR #543** (rebased `da30d45`, `#539` 1.9.0 frustum fovy) |
| `.mali/worktrees/mali-bc-astc` | `feat/mali-bc-astc` | `9a4e8a6` | **OPEN PR #579** (rebased `da30d45`, `BC3`/`BC5` -> ASTC 4x4) |
| `.mali/worktrees/pr-mali-renderpass-opt` | `feat/mali-rp-merge-store-opt` | `40ccf68` | **OPEN PR #586** (rebased `da30d45`, `r49+` dclear + `STORE_OP_NONE`) |
| `.mali/worktrees/pr-mali-shader-cleanup` | `feat/mali-spirv-cleanup-discard` | `53dd248` | **CLOSED PR #582** (no cold/60 FPS win on `SM-X730`; archived) |
| `.mali/worktrees/fix-android-nintendo-gyro` | `fix/android-nintendo-gyro` | `d038d38` | **MERGED PR #545** (in `v0.5.1-beta`) |
| `.mali/worktrees/pr-a-40fps-120hz-pacer` | `feat/android-40fps-120hz-pacer` | `0f75d15` | **Stage A `PR-A`** (`40 FPS + 120 Hz` pacer + dynamic `ADPF` + `16px`) |
| `.mali/worktrees/pr-b-cpu-affinity` | `feat/android-cpu-affinity-worker-isolation` | `3b45216` | **Stage A `PR-B`** (Oryon `6+2` `mode=3` + Mali `4+3+1` affinity) |
| `.mali/worktrees/odin3-stage-ab` / `odin3-stage-cd` | `feat/odin3-stage-*` | `0898253`/`6d3df97` | Odin 3 Stage A-D play-ready stack (`SM8750` defaults) |
| `.mali/worktrees/mali-40fps-opt` | `feat/mali-40fps-opt` | `109f6d7` | Consolidated Mali 40 FPS + Track 6 source (`SM-X730` stock) |
| Legacy dev wts (`mali-renderpass-opt`, `mali-shader-opt`, `mali-direct-heap`, `mali-cpu-thermals`, `op13-turnip-opt`, archived `11`) | Parked | `f856e65`.. | Historical dev branches |

## Physical Devices & Backup Guards

| Device | Role | GPU / Driver | Installed App & Verified Backup |
|---|---|---|---|
| **Odin 3 (`SM8750`)** *(USB, rooted)* | **Primary** | Adreno 830 (`Turnip Gen8 V39 Patched`) | **`odin3-stage-cd` (`6d3df97`, play-ready)**; `2026-11-27`: `26/26` saves (`0` diffs), INI active, `2686` `.spv` |
| **Tab S11 (`SM-X730`)** | Protected Secondary | Mali-G925 (`r49p1`) | **`40fps-final`** (`109f6d7`, `98f4649c` restored); `2026-11-27`: `67/67` saves, `23903` `.spv`, `vk_pipelines` `82790ffe` |
| **OP13 (`CPH2653`)** | Reference | Adreno 830 (`Turnip V38`) | **`v0.4.10-op13`** (`63a4195f`); `2026-11-21`: `83/84` saves, `2707` `.spv` |

Rollback APKs (`.mali/apk/`): `v0.4.12-mali`, `v0.4.11-mali`, `v0.4.10-op13`; `.mali/user-backup/2026-11-27/mali/` (`98f4649c`), Odin 3 stock (`81aaf2cf`) + `odin3-stage-cd-final-signed.apk`.

## Stage A-D & PR Review Progress

- **Open upstream (`MERGEABLE / CLEAN` on `v0.5.1-beta`):** `#529` (`63c9e1a`), `#543` (`6bccdbf`), `#579` (`9a4e8a6`), `#586` (`40ccf68`); `#545` **MERGED**; `#582` **CLOSED** after `53dd248` `SM-X730` A/B (`cold 45.32->44.77 s`, `plat 40.32->40.08`, `zora 42.17->42.30 FPS`, `dye 67.0% >4` tonal shift).
- **Evidence branch:** `origin/evidence/pr-reviews-2026-11-27` @ `41a4243` (`pr579/*`, `pr586/*`).
- **Odin 3 Stage C+D (`6d3df97`):** `40.00 FPS` (`low1 39.75-39.85`), `0 >26ms`, `6.79-9.16 W`, `<=62.1 C`, `26/26` saves intact.
- **Latest ledger (`MALI_PROGRESS.md`):** `2026-11-27` `v051-beta-pr-review-fixes`; `2026-11-26` `mali-40fps-opt`; `2026-11-25` `track5-mali-shader-cleanup-pr582`; `2026-11-24` `v0412-beta-mali-rollout`.
