# MALI Status

Last updated: `2026-11-21` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.10-beta`** (`2026-11-21T15:06:56Z`, `1c02adf9`; upstream `#401` CAS optimization & `versionCode 49`).
- **Fork `main`:** `1c02adf994194055f7d517fc99dc5686269e11ed` (`main == origin/main == upstream/main == v0.4.10-beta`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `1c02adf` | Clean `v0.4.10-beta` baseline & docs |
| `.mali/worktrees/mali-renderpass-default` | `feat/vk-mali-renderpass-default` | `9d5e732` | **OPEN PR #534** (`MERGEABLE`, Mali `r49+` classic RP + `rpcompat` hash) |
| `.mali/worktrees/bc-parallel-decode` | `feat/vk-bc-parallel-decode` | `122e11e` | **OPEN PR #531** (`MERGEABLE`, parallel CPU BC1–7 & BC1->ASTC decode) |
| `.mali/worktrees/warm-cache-pipe-fastpath` | `feat/vk-warm-cache-fastpath` | `edeb35e` | **OPEN PR #529** (`MERGEABLE`, inline warm cache + `pipelineCreationCacheControl`) |
| `.mali/worktrees/render-cpu-hotpath` | `feat/vk-render-cpu-hotpath` | `e54d6a8` | **HELD locally** (verified `9/9`, held due to `<0.5ms` isolated gain on `DR=on`) |
| `.mali/worktrees/tile-local-read` | `feat/vk-local-read` | `9e75f99` | **OPEN PR #518** (`MERGEABLE`, `VK_KHR_dynamic_rendering_local_read`) |
| `.mali/worktrees/mali-inplace-feedback` | `feat/mali-inplace-feedback` | `2e36682` | **OPEN PR #507** (`UNKNOWN` merge check, `7f5afab` rebase + 11-locale UI note) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **OPEN PR #480** (`3/3` starter heights verified) |
| `.mali/worktrees/mali-release-048` | `release/mali-0.4.8-beta` | `5d93424` | Historical `v0.4.8-beta-mali` build (`#504`, `#506`, `#507`) |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **Galaxy Tab S11 (SM-X730)** *(connected)* | Mali-G925 MC12 (`r49p1` stock Mali) | **Stock `v0.4.10-beta` (`1c02adf`)** (`9ae5c77a...0f6`, re-signed `60fa2a5f...`) | `2026-11-21`: `68/68` saves (`diffs=0`), `3/4` prefs (`ActivityThread.IDS.xml`), `10401` `.spv`, `vk_pipelines.bin=c2a8d11fdc66`; idle |
| **OnePlus 13 (CPH2653)** *(unplugged)* | Adreno 830 (**Turnip `Turnip_Gen8_V38_Patched`** only) | `v0.4.5-beta` Mali (`2229426f...7b23a`) | `2026-11-17`: `83/84` (`0` save/INI diffs); `1865` `.spv`; PRs `#518`/`#529`/`#531`/`#534` untested on Turnip |

Rollback APKs under `.mali/apk/`: `v0.4.10-beta` (`9ae5c77a...`), `v0.4.8-beta-mali`, `v0.4.7-beta-mali`, `v0.4.6-beta-mali`, `v0.4.5-beta-mali`. Evidence branches (`origin`): `evidence/pr-mali-renderpass-default-20261121` (`2a2354b`), `evidence/pr-bc-parallel-decode-20261121` (`6df6519`), `6b5d66c7` (`#529`), `evidence/pr-local-read-20261120` (`abf2ec4`).

## Open PRs & Active Priorities

- **Merged:** `#504` (`5df9dd9`), `#506` (`c02beea`), `#508` (`3d28f80`), `#481`–`#485` (`#315`, `#436`, `#461`, `#475`).
- **Open:** **`#534`** (`9d5e732`), **`#531`** (`122e11e`), **`#529`** (`edeb35e`), **`#518`** (`9e75f99`), **`#507`** (`2e36682`), **`#480`** (`c48d1b1`).
- **Immediate Queue:** Await `mamcjr` review on `#529`/`#531`/`#534`/`#518`/`#507`; optional consolidated `v0.4.10-beta-mali` APK (`#529`+`#531`+`#534`+`#507`+`#518`) for daily 40 FPS play on `SM-X730`; verify `CPH2653` Turnip when plugged.

## Recent Milestones

- `[2026-11-21]` **PR #534 Classic Render Pass Default (`9d5e732`) & Locked 40 FPS Validation:** Mali `r49+` classic RP default cut `render_thread_ms` by `-1.87..-2.40 ms` p50 (`kakariko 60p +3.46 fps`); held `2B-2` (`e54d6a8`); combined `ArmC_40p_combined_nodr` hit locked `40.00 FPS` (`75% SGSR1`, `shadows=1`) across all 3 scenes; restored stock `v0.4.10-beta` (`9ae5c77a...`, `68/68` saves).
- `[2026-11-21]` **PR #531 Parallel BC Decode (`122e11e`) & PR #529 Warm Cache Fastpath (`edeb35e`):** Cut BC decode `2.59x` (`-61.4%`, camera peaks `4.64x–4.85x` lower) & eliminated warm teleport Link vanish (`skipped 1478 -> 91`, `1020/1032` cache hits).
- `[2026-11-21]` **PR #518 Local Read (`9e75f99`) & PR #507 Update (`2e36682`):** Opened `VK_KHR_dynamic_rendering_local_read` PR (`fbcopy -70%/-90%`); rebased `#507` on `v0.4.9-beta`. See `MALI_PROGRESS.md`.
