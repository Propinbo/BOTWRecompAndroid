# MALI Status

Last updated: `2026-11-21` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.10-beta`** (`2026-11-21T15:06:56Z`, `1c02adf9`, `versionCode 49`).
- **Fork `main`:** `1c02adf994194055f7d517fc99dc5686269e11ed` (`main == origin/main == upstream/main`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `1c02adf` | Clean `v0.4.10-beta` baseline & docs |
| `.mali/worktrees/mali-release-0410` | `release/mali-0.4.10-beta` | `7451bb8` | **Installed `v0.4.10-beta-mali`** (`#534+#531+#529+#507+#518+#537`) |
| `.mali/worktrees/fix-descriptor-plan-dyn-ubo` | `fix/vk-descriptor-plan-dynamic-ubo` | `c27c1b7` | **OPEN PR #537** (`MERGEABLE`, `descriptor_plan` UBO_DYNAMIC) |
| `.mali/worktrees/mali-renderpass-default` | `feat/vk-mali-renderpass-default` | `9d5e732` | **OPEN PR #534** (`MERGEABLE`, Mali `r49+` classic RP + `rpcompat` hash) |
| `.mali/worktrees/bc-parallel-decode` | `feat/vk-bc-parallel-decode` | `122e11e` | **OPEN PR #531** (`MERGEABLE`, 4T parallel CPU BC/ASTC decode) |
| `.mali/worktrees/warm-cache-pipe-fastpath` | `feat/vk-warm-cache-fastpath` | `edeb35e` | **OPEN PR #529** (`MERGEABLE`, inline warm cache + cache control) |
| `.mali/worktrees/render-cpu-hotpath` | `feat/vk-render-cpu-hotpath` | `e54d6a8` | **HELD locally** (verified `9/9`, held for `<0.5ms` isolated `DR=on` gain) |
| `.mali/worktrees/tile-local-read` | `feat/vk-local-read` | `9e75f99` | **OPEN PR #518** (`MERGEABLE`, `VK_KHR_dynamic_rendering_local_read`) |
| `.mali/worktrees/mali-inplace-feedback` | `feat/mali-inplace-feedback` | `2e36682` | **OPEN PR #507** (`7f5afab` rebase + 11-locale Mali UI note) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (`3/3` starter heights) |
| `.mali/worktrees/mali-release-048` | `release/mali-0.4.8-beta` | `5d93424` | Historical `v0.4.8-beta-mali` build |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **Galaxy Tab S11 (SM-X730)** *(connected)* | Mali-G925 MC12 (`r49p1` stock Mali) | **`v0.4.10-beta-mali` (`7451bb8`)** (`a6899a76...6b6`, cert `60fa2a5f...`) | `2026-11-21-b`: `68/68` saves (`diffs=0`), `3/4` prefs (`ActivityThread.IDS.xml`), `10401` `.spv`, `vk_pipelines.bin=c2a8d11fdc66`; stopped |
| **OnePlus 13 (CPH2653)** *(unplugged)* | Adreno 830 (**Turnip V38** only) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `83/84` (`0` save/INI diffs); `#518/#529/#531/#534/#537` untested on Turnip |

Rollback APKs under `.mali/apk/`: `v0.4.10-beta-mali` (`a6899a76...`), `v0.4.10-beta` (`9ae5c77a...`), `v0.4.8-beta-mali`, `v0.4.7-beta-mali`, `v0.4.6-beta-mali`, `v0.4.5-beta-mali`. Evidence branches (`origin`): `evidence/pr-descriptor-plan-dyn-ubo-20261121` (`a1c84ac`), `evidence/pr-mali-renderpass-default-20261121` (`2a2354b`), `evidence/pr-bc-parallel-decode-20261121` (`6df6519`), `6b5d66c7`, `abf2ec4`.

## Open PRs & Active Priorities

- **Merged:** `#504` (`5df9dd9`), `#506` (`c02beea`), `#508` (`3d28f80`), `#481`–`#485`.
- **Open:** **`#537`** (`c27c1b7`), **`#534`** (`9d5e732`), **`#531`** (`122e11e`), **`#529`** (`edeb35e`), **`#518`** (`9e75f99`), **`#507`** (`2e36682`), **`#480`** (`c48d1b1`).
- **Immediate Queue:** Await `mamcjr` review on `#507`/`#518`/`#529`/`#531`/`#534`/`#537`; cross-test on `CPH2653` Turnip when attached.

## Recent Milestones

- `[2026-11-21]` **PR #537 (`c27c1b7`) & `v0.4.10-beta-mali` Rollout (`7451bb8`):** Fixed `descriptor_plan()` `UNIFORM_BUFFER_DYNAMIC` drop/crash (`31,040` equiv, `SM-X730` black-screen/SIGSEGV -> `30.00 fps` `0` errors); built, verified (`15/15` Bionic, `40.00 fps` across all 3 scenes at `75% SGSR1` + `shadows=1` + `inplace_reads=1`), and installed `v0.4.10-beta-mali` (`a6899a76...`, `68/68` saves intact).
- `[2026-11-21]` **PR #534 Classic RP (`9d5e732`) & PR #531/#529 (`122e11e`/`edeb35e`):** `-1.87..-2.40 ms` render CPU (`kakariko 60p +3.46 fps`), `2.59x` faster BC decode, `-93.8%` warm teleport skips.
- `[2026-11-21]` **PR #518 Local Read (`9e75f99`) & PR #507 Rebase (`2e36682`):** Added Mali local-read path & 11-locale `#507` UI note. See `MALI_PROGRESS.md`.
