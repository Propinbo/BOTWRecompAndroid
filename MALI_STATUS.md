# MALI Status

Last updated: `2026-11-24` (`Europe/Moscow`)

## Upstream & Repository Baseline

- **Newest upstream release:** `mamcjr/BOTWRecompAndroid` **`v0.4.12-beta`** (`2026-11-24T13:33:45Z`, `79ba1909`, `versionCode 51`).
- **Fork `main`:** `79ba1909f716d7761046a767651c62dbfa917ef1` (`main == origin/main == upstream/main`).

## Active Worktrees

| Worktree | Branch | HEAD | Purpose |
|---|---|---|---|
| Root `.` | `main` | `79ba190` | Clean `v0.4.12-beta` baseline & docs |
| `.mali/worktrees/mali-release-0412` | `release/mali-0.4.12-beta` | `f87d238` | **Installed `v0.4.12-beta-mali`** (`79ba1909` + 5 PRs + thermals) |
| `.mali/worktrees/mali-cpu-thermals` | `feat/mali-cpu-thermals` | `0a89336` | CPU/thermals P1+P2 (`300s` `40fps` `PASS`) |
| `.mali/worktrees/fix-android-nintendo-gyro` | `fix/android-nintendo-gyro` | `6f30e81` | **OPEN PR #545** (axes/clock/accel/bias) |
| `.mali/worktrees/fix-narrow-shadow-539` | `fix/narrow-shadow-539` | `d27739c` | **OPEN PR #543** (`Fixes #539` Vert+ shadows) |
| `.mali/worktrees/mali-renderpass-default` | `feat/vk-mali-renderpass-default` | `c1a29d9` | **OPEN PR #534** (Mali `r49+` RP + `dyn_render`) |
| `.mali/worktrees/bc-parallel-decode` | `feat/vk-bc-parallel-decode` | `3abb640` | **OPEN PR #531** (`min(4,big)` BC/ASTC decode) |
| `.mali/worktrees/warm-cache-pipe-fastpath` | `feat/vk-warm-cache-fastpath` | `87ba2a4` | **OPEN PR #529** (FPS-scaled fastpath + peek) |
| `.mali/worktrees/tile-local-read` | `feat/vk-local-read` | `6bb6c66` | **CLOSED PR #518** (superseded by `#534`) |
| `.mali/worktrees/fix-descriptor-plan-dyn-ubo` | `fix/vk-descriptor-plan-dynamic-ubo` | `c27c1b7` | **MERGED PR #537** (dynamic UBO plan) |
| `.mali/worktrees/benchmark-scenes-v2` | `benchmark-scenes-v2` | `c48d1b1` | **MERGED PR #480** (`3/3` starter heights) |
| `.mali/worktrees/op13-turnip-opt` | `feat/op13-turnip-opt` | `7f4cc9a` | **Installed `v0.4.10-beta-op13`** (`Turnip` opt) |
| `.mali/worktrees/render-cpu-hotpath` | `feat/vk-render-cpu-hotpath` | `e54d6a8` | **HELD** (folded into `7f4cc9a`) |
| `.mali/worktrees/mali-inplace-feedback` | `feat/mali-inplace-feedback` | `2e36682` | **MERGED PR #507** (`2e36682`) |
| `.mali/worktrees/mali-release-0411` | `release/mali-0.4.11-beta` | `072fc5c` | Archived `v0.4.11-beta-mali` |
| `.mali/worktrees/mali-release-0410` | `release/mali-0.4.10-beta` | `7451bb8` | Archived `v0.4.10-beta-mali` |
| `.mali/worktrees/mali-release-048` | `release/mali-0.4.8-beta` | `5d93424` | Archived `v0.4.8-beta-mali` |

## Physical Devices & Backup Guards

| Device | GPU / Driver | Installed App | Verified Backup & Caches |
|---|---|---|---|
| **Tab S11 (SM-X730)** *(connected)* | Mali-G925 (`r49p1`) | **`v0.4.12-beta-mali`** (`8eedca1a`, `v51`, `60fa2a5f`, stopped) | `2026-11-24/mali`: `68/68` saves, `19004` `.spv`, `ac048f10`; clean |
| **OP13 (CPH2653)** *(unplugged)* | Adreno 830 (**Turnip V38**) | **`v0.4.10-beta-op13`** (`63a4195f`, `60fa2a5f`) | `2026-11-21`: `83/84` saves, `2707` `.spv`, `725a99243c7f`; stopped |

Rollback APKs (`.mali/apk/`): `v0.4.12-beta-mali` (`8eedca1a`, `v51`), `pre-review-20261124.apk` (`196c94c6`, `v50`), `v0.4.11-beta-mali` (`881201b0`), `v0.4.10-op13` (`63a4195f`), `v0.4.10-mali`, `v0.4.8`..`0.4.5`.

## Open PRs & Active Priorities

- **Merged:** `#537`, `#480`, `#507`, `#504`, `#506`, `#508`, `#481`–`#485`. **Closed:** `#518`. **Open (`5/5` `MERGEABLE`):** `#545`, `#543`, `#534`, `#531`, `#529`.
- **Queue:** Watch upstream merges on the 5 open PRs + slice CPU/thermals PRs (`0a89336`); `Background/HOME Survival` (`Sphere 3`); `CPH2653` Turnip (`unplugged`).

## Recent Milestones

- `[2026-11-24]` **Consolidated `v0.4.12-beta-mali` Rollout (`f87d238`, APK `8eedca1a`):** Stacked `79ba1909` + 5 updated PRs (`#529`/`#531`/`#534`/`#543`/`#545`) + 14 `mali-cpu-thermals` commits; `10/10` host + `16/16` Bionic + `test_shader_meta_cache` `PASS`; live `SM-X730` `30.00 fps` (`29.00–29.50` low1) and `40.00 fps` (`37.96–39.10` low1, `25.00 ms`), clean PNGs, `68/68` saves & `19004` `.spv` verified.
- `[2026-11-24]` **Upstream `v0.4.12-beta` PR Review Fixes:** Merged `#480`/`#537`, closed `#518` for `#534`; rebased & fixed `#545` (`6f30e81`), `#531` (`3abb640`), `#543` (`d27739c`), `#534` (`c1a29d9`), `#529` (`87ba2a4`, `#534+#529` `-64.4%` skip / `97.8%` fastpath).
- `[2026-11-22]` **CPU & Thermals (`0a89336`, `196c94c6`):** `8/8` host + `18/18` Bionic `PASS`; `300s` soak `40.0 fps` (`-31.1 pts` late sched, `4.06` vs `4.34 W`).
