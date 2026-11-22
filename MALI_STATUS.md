# MALI Status

Last updated: `2026-11-22` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.11-beta`** (`2026-11-21T23:30:59Z`, `afd7b842`, `versionCode 50`).
- **Fork `main`:** `afd7b84299fd3492bbfd8f9ee35065dd60f950d9` (`main == origin/main`; `upstream/main` `75bfc64c`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `afd7b84` | Clean `v0.4.11-beta` baseline & docs |
| `.mali/worktrees/mali-release-0411` | `release/mali-0.4.11-beta` | `072fc5c` | **Installed `v0.4.11-beta-mali`** (`75bfc64c` + `#534..#545`) |
| `.mali/worktrees/fix-android-nintendo-gyro` | `fix/android-nintendo-gyro` | `08963d5` | **OPEN PR #545** (`057e` axes/clock/bias, `MERGEABLE`) |
| `.mali/worktrees/fix-narrow-shadow-539` | `fix/narrow-shadow-539` | `213c521` | **OPEN PR #543** (`Fixes #539` Vert+ shadows) |
| `.mali/worktrees/op13-turnip-opt` | `feat/op13-turnip-opt` | `7f4cc9a` | **Installed `v0.4.10-beta-op13`** (`Turnip` opt) |
| `.mali/worktrees/mali-release-0410` | `release/mali-0.4.10-beta` | `7451bb8` | Archived `v0.4.10-beta-mali` |
| `.mali/worktrees/fix-descriptor-plan-dyn-ubo` | `fix/vk-descriptor-plan-dynamic-ubo` | `c27c1b7` | **OPEN PR #537** (UBO_DYNAMIC `descriptor_plan`) |
| `.mali/worktrees/mali-renderpass-default` | `feat/vk-mali-renderpass-default` | `9d5e732` | **OPEN PR #534** (Mali `r49+` classic RP + hash) |
| `.mali/worktrees/bc-parallel-decode` | `feat/vk-bc-parallel-decode` | `122e11e` | **OPEN PR #531** (4T parallel BC/ASTC decode) |
| `.mali/worktrees/warm-cache-pipe-fastpath` | `feat/vk-warm-cache-fastpath` | `ea24f43` | **OPEN PR #529** (inline warm cache + ctrl) |
| `.mali/worktrees/render-cpu-hotpath` | `feat/vk-render-cpu-hotpath` | `e54d6a8` | **HELD** (folded into `7f4cc9a`) |
| `.mali/worktrees/tile-local-read` | `feat/vk-local-read` | `6bb6c66` | **OPEN PR #518** (`local_read` + ROAA) |
| `.mali/worktrees/mali-inplace-feedback` | `feat/mali-inplace-feedback` | `2e36682` | **MERGED PR #507** (`2e36682`) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (`3/3` starter heights) |
| `.mali/worktrees/mali-release-048` | `release/mali-0.4.8-beta` | `5d93424` | Historical `v0.4.8-beta-mali` |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **Tab S11 (SM-X730)** *(connected)* | Mali-G925 (`r49p1`) | **`v0.4.11-beta-mali`** (`9d2aaed2...`, `60fa2a5f...`) | `2026-11-22-gyro-pr`: `68/68` saves, `15802` `.spv`, `1fefc662caaa`; stopped |
| **OP13 (CPH2653)** *(unplugged)* | Adreno 830 (**Turnip V38**) | **`v0.4.10-beta-op13`** (`63a4195f...`, `60fa2a5f...`) | `2026-11-21`: `83/84` saves, `2707` `.spv`, `725a99243c7f`; stopped |

Rollback APKs (`.mali/apk/`): `v0.4.11-beta-mali` (`9d2aaed2`, prev `aa13a014`/`881201b0`), `v0.4.10-op13` (`63a4195f`), `v0.4.10-mali` (`a6899a76`), `v0.4.10` (`9ae5c77a`), `v0.4.8`..`0.4.5`.

## Open PRs & Active Priorities

- **Merged:** `#507`, `#504`, `#506`, `#508`, `#481`–`#485`. **Open (`8/8` `CLEAN`):** `#545`, `#543`, `#537`, `#534`, `#531`, `#529`, `#518`, `#480`.
- **Queue:** Monitor 8 open `CLEAN` PRs; `CPH2653` Turnip testing (`unplugged`); graphics accuracy / CPU thermals backlog.

## Recent Milestones

- `[2026-11-22]` **PR `#545` (`08963d5`) & `v0.4.11-beta-mali` (`072fc5c`, `9d2aaed2`):** Switch 1 `(-y,z,-x)`, `1.96x` clock scaling, raw-frame `GyroBias`; `3/3` Round 2 verifiers PASS, `13/13` Bionic PASS (`15/15` `test_motion`), live 8BitDo Pro 2 (`0.000` turns drift, owner PASS) & tablet IMU (`0` drift).
- `[2026-11-22]` **PR `#518` Rebase (`6bb6c66`):** `ROAA`, 88B legacy `pw_id`, format/clear/off-path guards; `2/2` verifier, `17/17` Bionic, `4/4` `SM-X730` FX (`fbcopy -50%..-90%`), `30/40.00 fps`.
- `[2026-11-22]` **PR `#529` Rebase (`ea24f43`) & PR `#543` (`213c521`, `#539`):** Rebased `#529` over `#520`; fixed Vert+ `mAngle` shadows (`8/8` Bionic).
- `[2026-11-22]` **`v0.4.10-beta-op13` (`7f4cc9a`):** `CPH2653` Turnip fullscreen + `CopyScratch` + `framepace` (`30.00`/`47.62 fps`, `0` errors).
