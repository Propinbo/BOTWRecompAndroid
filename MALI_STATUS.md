# MALI Status

Last updated: `2026-11-22` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.11-beta`** (`2026-11-21T23:30:59Z`, `afd7b842`, `versionCode 50`).
- **Fork `main`:** `afd7b84299fd3492bbfd8f9ee35065dd60f950d9` (`main == origin/main`; `upstream/main` `75bfc64c`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `afd7b84` | Clean `v0.4.11-beta` baseline & docs |
| `.mali/worktrees/mali-release-0411` | `release/mali-0.4.11-beta` | `1a836dd` | **Installed `v0.4.11-beta-mali`** (`75bfc64c` + `#534..#543`) |
| `.mali/worktrees/fix-narrow-shadow-539` | `fix/narrow-shadow-539` | `213c521` | **OPEN PR #543** (`Fixes #539` narrow Vert+ shadows) |
| `.mali/worktrees/op13-turnip-opt` | `feat/op13-turnip-opt` | `7f4cc9a` | **Installed `v0.4.10-beta-op13`** (`Turnip` opt + `CopyScratch`) |
| `.mali/worktrees/mali-release-0410` | `release/mali-0.4.10-beta` | `7451bb8` | Archived `v0.4.10-beta-mali` release |
| `.mali/worktrees/fix-descriptor-plan-dyn-ubo` | `fix/vk-descriptor-plan-dynamic-ubo` | `c27c1b7` | **OPEN PR #537** (`descriptor_plan` UBO_DYNAMIC) |
| `.mali/worktrees/mali-renderpass-default` | `feat/vk-mali-renderpass-default` | `9d5e732` | **OPEN PR #534** (Mali `r49+` classic RP + hash) |
| `.mali/worktrees/bc-parallel-decode` | `feat/vk-bc-parallel-decode` | `122e11e` | **OPEN PR #531** (4T parallel BC/ASTC decode) |
| `.mali/worktrees/warm-cache-pipe-fastpath` | `feat/vk-warm-cache-fastpath` | `ea24f43` | **OPEN PR #529** (inline warm cache + ctrl) |
| `.mali/worktrees/render-cpu-hotpath` | `feat/vk-render-cpu-hotpath` | `e54d6a8` | **HELD** (folded into `7f4cc9a` Turnip validation) |
| `.mali/worktrees/tile-local-read` | `feat/vk-local-read` | `9e75f99` | **OPEN PR #518** (`dynamic_rendering_local_read`) |
| `.mali/worktrees/mali-inplace-feedback` | `feat/mali-inplace-feedback` | `2e36682` | **MERGED PR #507** (`2e36682`) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (`3/3` starter heights) |
| `.mali/worktrees/mali-release-048` | `release/mali-0.4.8-beta` | `5d93424` | Historical `v0.4.8-beta-mali` |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **Galaxy Tab S11 (SM-X730)** *(connected)* | Mali-G925 (`r49p1`) | **`v0.4.11-beta-mali`** (`68a6f611...`, `60fa2a5f...`) | `2026-11-22`: `68/68` saves (`diffs=0`), `12973` `.spv`, `c008b3285d18`; stopped |
| **OnePlus 13 (CPH2653)** *(unplugged)* | Adreno 830 (**Turnip V38**) | **`v0.4.10-beta-op13`** (`63a4195f...`, `60fa2a5f...`) | `2026-11-21`: `83/84` (`0` save/INI diffs), `2707` `.spv`, `725a99243c7f`; stopped |

Rollback APKs (`.mali/apk/`): `v0.4.11-beta-mali` (`68a6f611...`), `v0.4.10-beta-op13` (`63a4195f...`), `v0.4.10-beta-mali` (`a6899a76...`), `v0.4.10-beta` (`9ae5c77a...`), `v0.4.8`..`0.4.5`. Evidence (`origin`): `12a76bd`, `a1c84ac`, `2a2354b`, `6df6519`, `6b5d66c7`.

## Open PRs & Active Priorities

- **Merged:** `#507`, `#504`, `#506`, `#508`, `#481`–`#485`. **Open (7/7 `CLEAN`):** `#543`, `#537`, `#534`, `#531`, `#529`, `#518`, `#480`.
- **Queue:** Address `mamcjr` review on **PR #518** (`PW_VERSION`, same-att overlap check, off-path guards); upstream cross-GPU CPU opts (`7f4cc9a`).

## Recent Milestones

- `[2026-11-22]` **PR #529 Rebase (`ea24f43`) & PR Audit:** Rebased onto `75bfc64c` (`#520` OOM intact); `9/9` Bionic, `2/2` verifier PASS, `7/7` PRs `CLEAN`; triaged `mamcjr` #518 review.
- `[2026-11-22]` **v0.4.11-beta-mali Rollout (`1a836dd`, APK `68a6f611...`):** Stacked `#534+#531+#529+#518+#537+#543` on `75bfc64c` (`v0.4.11-beta` + `#507` + `#520` + gyro axes); `12/12 + 3/3` Bionic PASS, `30/40.00 fps` across `3/3` scenes, `16:10` shadow `126.52`, `68/68` saves intact.
- `[2026-11-22]` **PR #543 (`213c521`, Fixes #539) & Owner Triage:** Fixed narrow Vert+ `mAngle` (`+0xA0`) mismatch (`16:10` `103.11 -> 131.33`, `4:3` `134.24`, `0` FOV creep, `8/8` Bionic); checked Nov 21 crash (`0` tombstones, OS bg CPU kill at `7.1 GB` RSS, `log_file=1`).
- `[2026-11-22]` **OnePlus 13 Turnip V38 Opt (`7f4cc9a`):** `3168x1440` modes, `+1.01 fps` / `+3.85 low1` (`-73.2%` skips, `63a4195f...`).
