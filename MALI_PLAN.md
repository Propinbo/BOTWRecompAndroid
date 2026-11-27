# BOTWRecompAndroid Engineering & Upstream Contribution Plan

Authoritative engineering roadmap for owner's hardware: **Rooted AYN Odin 3** (`SM8750`, Snapdragon 8 Elite Oryon `6+2` + Adreno 830 + Mesa Turnip `Gen8 V39 Patched`, **Primary Target**) and **Galaxy Tab S11** (`SM-X730`, Dimensity 9400+ / Mali-G925 `r49p1`, **Protected Non-Regression Tier**), plus OnePlus 13 (`CPH2653`) reference, targeting upstream contributions to `https://github.com/mamcjr/BOTWRecompAndroid`.

## 1. Governance & Workflow Contract

Governance, language policy, authority boundaries, safety invariants, and verification rules are defined in root [`AGENTS.md`](AGENTS.md) and `.agents/skills/*/SKILL.md`:
- **Owner:** Defines goals, priorities, requirements, and success criteria; decides major scope changes and consequential product/architecture trade-offs.
- **Agent:** Operates with full command autonomy across Git (`origin` pushes, rebases, `main` fast-forwards, `mali-workspace` checkpoints), GitHub CLI (`gh pr`/`gh issue`), builds, and isolated ADB testing while enforcing IP, save-safety, Turnip-only Adreno, NTFS junction, and mandatory pre-PR `2–3` `independent-verifier` multi-agent review invariants.
- **Documentation checkpoint branch:** `mali-workspace` on `Propinbo/BOTWRecompAndroid` tracks only `AGENTS.md`, `MALI_PLAN.md`, `MALI_PROGRESS.md`, `MALI_STATUS.md`, and `.agents/` with `[skip ci]`. Never merge `mali-workspace` into `main` or PR branches.

## 2. Rolling Upstream Baseline

The active development target tracks the latest *verified, published* `mamcjr/BOTWRecompAndroid` GitHub Release (betas included). Campaigns pin one exact SHA and rebase/migrate cleanly onto newer verified releases when appropriate.

| Checked (UTC) | Newest Published Release | Tag SHA | Fork `main` Baseline | Relevance / Notes |
|---|---|---|---|---|
| `2026-11-25` | `v0.5.0-beta` | `d68d698` (`upstream/main` `208746bb`) | `208746bb` (`upstream/main`) | Fast-forwarded `main` (`versionCode 53`). Opened **PR #582** (`feat/mali-spirv-cleanup-discard` @ `7517817`) atop `208746bb`; parked Track 5 Phase 2 (`RelaxedPrecision`) locally; restored clean `v0.5.0-beta` Release APK (`81aaf2cf...`) on `SM-X730`. |
| `2026-11-25` | `v0.4.13-beta` | `c5b19bd1892fae71a2e4fb897bd6ef2d59102328` | `v0.4.13-beta` (`c5b19bd`) | `#534` (`98dd516`) & `#531` (`5b78b37`) **MERGED** upstream (`versionCode 52`). Opened **PR #579** (`feat/mali-bc-astc` @ `d63e3a0`); executed Track 5 Phase 0–2 (`feat/mali-shader-opt`). |
| `2026-11-24` | `v0.4.12-beta` | `79ba1909f716d7761046a767651c62dbfa917ef1` | `v0.4.12-beta` (`79ba190`) | Fast-forwarded `main` (`versionCode 51`, `#480`/`#537` merged). Closed `#518` for `#534`; updated all 5 open PRs (**`#545`** `6f30e81`, **`#543`** `d27739c`, **`#534`** `c1a29d9`, **`#531`** `3abb640`, **`#529`** `87ba2a4`); deployed **`v0.4.12-beta-mali`** (`f87d238`, APK `8eedca1a`) on `SM-X730`. |
| `2026-11-22` | `v0.4.11-beta` | `afd7b84299fd3492bbfd8f9ee35065dd60f950d9` | `v0.4.11-beta` (`afd7b84`) | Fast-forwarded `main` (`versionCode 50`, `#520`, `#511/#521`); `upstream/main` `75bfc64c` (`#507` merged). Rebased `#529` & `#518`; opened **`#543`** & **`#545`** (`08963d5`); deployed **`v0.4.11-beta-mali`** (`072fc5c`, APK `9d2aaed2...`) on `SM-X730`. |
| `2026-11-21 15:06` | `v0.4.10-beta` | `1c02adf994194055f7d517fc99dc5686269e11ed` | `v0.4.10-beta` (`1c02adf`) | Fast-forwarded `main` (`7f5afab -> 1c02adf9`, `#401` CAS optimization & `versionCode 49`). Opened **#529** (`edeb35e`), **#531** (`122e11e`), **#534** (`9d5e732`), **#537** (`c27c1b7`); held `render-cpu-hotpath` (`e54d6a8`); rolled out verified `v0.4.10-beta-mali` (`7451bb8`, `40.00 fps`) on `SM-X730`. |
| `2026-11-21 04:30` | `v0.4.9-beta` | `7f5afab71f122f1622401c0f11775e40f78df0e2` | `v0.4.9-beta` (`7f5afab`) | Fast-forwarded `main` & `origin/main` (`2b71a10 -> 7f5afab7`); retired `fullscreen-narrow` & `cbwin-mali-snap` (`#504`/`#506` merged). Rebased **#507** (`2e36682`, 11-locale Mali UI note). **#518** (`9e75f99`) & **#507** (`2e36682`) both `MERGEABLE`. |
| `2026-11-20 11:51` | `v0.4.8-beta` | `2b71a10c934e52780fc769b6c8b50de3278a7c2c` | `v0.4.8-beta` (`2b71a10`) | Fast-forwarded `main` (`2c72f20 -> 2b71a10`). Merged PR **#508** (`3d28f80`, `gcopy_dchain`). Rebased & updated `#504` (`186ea04`), `#506` (`adbafd6`), `#507` (`fe2f659`); retired `gcopy-dchain` & `mali-release-047`; deployed `release/mali-0.4.8-beta` (`5d93424`). |
| `2026-11-20 05:31` | `v0.4.7-beta` | `2c72f20d542e44ef3350c2c05cb036d7fb1dfcb8` | `v0.4.7-beta` (`2c72f20`) | Fast-forwarded `main` (`0897fb8 -> 2c72f20`). Merged PRs `#481`, `#482`, `#483`, `#485`, plus `#452`, `#453` (`cbwin` copy), `#455`, `#456`, `#457` (Shadows), `#471/#478` (`MADV_PAGEOUT` map), & `#401` thread dump. Retired 5 merged/superseded worktrees. |
| `2026-11-17 17:04` | `v0.4.6-beta` | `0897fb889719d0489a7bcbd8dba320b997d377ce` | `v0.4.6-beta` (`0897fb8`) | Fast-forwarded fork/local `main` (`6113c61 -> 0897fb8`). Merged upstream `#436`, `#475`, `#401` freeze watchdog. Rebased PRs `#481–#483`; opened PR `#485`; built/verified `feat/mali-perf-046` & `release/mali-0.4.6-beta` (`SM-X730`). |
| `2026-11-16 18:54` | `v0.4.5-beta` | `6113c61a1d0e6f78532eb7fdde9cc9348613ad4b` | `v0.4.5-beta` (`6113c61`) | Restart shader free, `#450` Mali shader eviction >1000, MediaTek 13 MHz clock fix. |
| `2026-11-16 05:11` | `v0.4.4-beta` | `280048bc43b05d76af0659a51c828ff393d4434e` | `v0.3.2-beta` (`7bb5bc9`) | PR `#461` (`0230d2f`) branched here; `0780cb6` (`#450`) merged cleanly (`git merge-tree`). |
| `2026-11-15 10:39` | `v0.4.3-beta` | `5946454c9fbdc414e0ab025f751f9c2011d23c3b` | `v0.3.2-beta` (`7bb5bc9`) | Installed stock APK on `SM-X730` & `CPH2653` (`3d3ad8e7...`); PR `#436` (`ab828a1`) branched here. |
| `2026-11-14 08:34` | `v0.4.0-beta` | `d0bda062b382334d70f709fc65c2d80a669f643c` | `v0.3.2-beta` (`7bb5bc9`) | Used for `#294` pipeline prewarming `13a–13b.1` and initial Water Cube root-cause proof. |
| `2026-11-08 07:12` | `v0.3.2-beta` | `7bb5bc98fae6093da3c2dd8f5af3c756b59ee7d5` | `v0.3.2-beta` (`7bb5bc9`) | Fast-forwarded fork `main` after PR `#255` & `#278` merged. |
| `2026-11-04 15:08` | `v0.3.1-beta` | `c48606cfafaaa4f41083830bbec2fd8b9ab01b47` | `v0.3.1-beta` (`c48606c`) | Fast-forwarded fork `main`; used for `11a–11b`, PR `#255`, PR `#278`, ultrawide/16:10 work. |
| `2026-11-01 17:36` | `v0.2.10-beta` | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.10-beta` (`4bad6dc`) | Used for Milestones 5–10, PR `#183`, and initial Teleport Dissolve (`#210`). |

Full 83-row historical release-check polling log (2026-10-31 to 2026-11-16): `.mali/archive/release-checks-2026-10-to-11.md`.

## 3. Active & Parked Engineering Initiatives

### A. Upstream PR Follow-Through & Mali Perf Pack
- **Merged Upstream (`v0.4.5-beta`–`v0.5.0-beta` / `upstream/main`):** `#315`, `#461`, `#436`, `#475`, `#480` (`c48d1b1`), `#481` (`d303dfd`), `#482` (`6840da4`), `#483` (`5c55081`), `#485` (`43c199a`), `#504` (`5df9dd9`), `#506` (`c02beea`), `#507` (`2e36682`), `#508` (`3d28f80`), `#531` (`5b78b37`), `#534` (`98dd516`), & `#537` (`c27c1b7`).
- **Closed:** `#518` (`6bb6c66`, `feat/vk-local-read` retained in fork) closed in favor of `#534` classic render passes on Mali `r49+`.
- **OPEN PRs (`5/5` `MERGEABLE`):** **`#582`** (`feat/mali-spirv-cleanup-discard` @ `7517817`, atop `208746bb`), **`#579`** (`feat/mali-bc-astc` @ `d63e3a0`), **`#545`** (`fix/android-nintendo-gyro` @ `6f30e81`), **`#543`** (`fix/narrow-shadow-539` @ `d27739c`), & **`#529`** (`feat/vk-warm-cache-fastpath` @ `87ba2a4`).
- **Held / Local Worktrees:** `feat/vk-render-cpu-hotpath` (`e54d6a8`) passed `9/9` Bionic and was folded into `CPH2653` Turnip opt (`feat/op13-turnip-opt` @ `7f4cc9a`).
- **Consolidated Mali Release (`release/mali-0.4.12-beta` @ `f87d238`):** Deployed rebuilt `BOTWRecompAndroid-0.4.12-beta-mali-release.apk` (`8eedca1a...`, `versionCode 51`, cert `60fa2a5f...`, `79ba1909` + 5 updated PRs + 14 `feat/mali-cpu-thermals` commits) onto `SM-X730` (`10/10` host + `16/16` Bionic + `test_shader_meta_cache` `PASS`; live `30.00 fps` & `40.00 fps` `PASS`; `68/68` saves intact).

### B. `v0.4.5-beta` Physical Mali Validation (Archived Reference)
1. **MediaTek 13 MHz Native-Mode Clock Fix (`SM-X730` PASS):** `LARECOMP_NATIVE=1` on Dimensity 9400+ scales 13 MHz timer reads to 19.2 MHz cleanly (`30.0 fps`, `0` errors).
2. **Mali Program Eviction (`#450`) & Reboot Free (`SM-X730` PASS):** `46` reboots freed up to `386` programs / `1,055` pipelines (`0` kept); `evict5m` freed `311` programs / `779` pipelines at flat `VmRSS ~3.50 GiB`.

### C. Dual-Target Engineering Roadmap (Odin 3 Primary + SM-X730 Non-Regression)

- **Primary Daily-Play Profile (Rooted AYN Odin 3, `1080p @ 120.00 Hz` AMOLED):** **"Native `1080p` (`1920x1080`, `100%`, `1:1`) `40.00 FPS` Ultra + Low-Power (`< 6–7.5 W`) + `tu_autotune` GMEM R&D"** (`16px` sub-native `65/68/70/85%` `SGSR1` + `100%` native bypass, no `>100%` SSAA/bicubic, `nouboopt` stays OFF on `a8xx`, no custom Turnip driver builds).
- **Protected Secondary Tier (`SM-X730` Mali-G925 `r49p1` + `CPH2653` Turnip):** Every shared change must preserve `100%` validated Mali-G925 behavior via vendor/capability gates; `PR-A` & `PR-B` split cleanly out of `feat/mali-40fps-opt` (`109f6d7`) atop `v0.5.0-beta` (`208746b`).

#### Active Staged Queue (Owner Approved `2026-11-27`)
1. **[COMPLETE] Stage A — `PR-A`, `PR-B` & Track `S1` (`Oryon 6+2`, Dynamic `ADPF`, `WC` Ring & `Native`):** `PR-A` (`0f75d15`) + `PR-B` (`3b45216`) + `S1` (`_Alignas(64)`, `-O3`, `WC` > `Cached`, `Native` wins `-1.9..-2.9 W`, `nouboopt` off guard on `a830`).
2. **[COMPLETE] Stage B — Track `S3` (Pass Consolidation, Feedback-Loop & `LRZ`):** `PR #582` + `PR #586` generalization cut passes `183.4->161.96` (`perf 142.62`) -> `8.31/6.19/8.82 W`, `61.7 °C`; `S3.3` feedback barrier kept `=0` default.
3. **[COMPLETE] Stage C — Track `S2` (`1080p Native Ultra` Visuals):** Locked `game_dynres=0`, `no_game_fxaa=1`+pre-HUD `SMAA`, `16x AF`, `IGN` dither, `OLED Black`, `2x` shadows & `1.5x` LOD Hard-Gate PASS (`40.00 FPS`, `0 >26ms`, Kakariko `p95 20.08–20.35<=20.5ms`, `9.16–9.40 W`, `<=62.1 °C`).
4. **[COMPLETE] Stage D — Tracks `S4` + `S5` + Play-Ready Deploy (`6d3df97`):** Turnip GPL (`669–779` fast-links, `0` fallbacks, boot wait `1324–1374->20.5–30.5ms`, `945` prewarm pipes), `S5` Suspend/Resume triad (`0` faults), `14/14` Bionic + Python tests, signed APK installed (`adb install -r`) + live INI configured + `26/26` saves intact. Next: upstream PR audit/submission & parked `SM-X730` items (`12b–12d`, fog `SAMPLES_PASSED`, narrow integer RT masks, `11c`).

## 4. Completed Campaigns & Historical Index

Detailed historical milestone specifications are archived in `.mali/archive/MALI_PLAN_COMPLETED_2026_10_11.md`; raw evidence lives in `MALI_PROGRESS.md` and `.mali/`.

| Period | Milestone / Campaign | Pinned SHA | Outcome & Upstream Artifacts |
|---|---|---|---|
| `2026-10-30` | M1–M2: Fork setup & issue `#89`/`#105` triage | `4acbf40` (`0.2.5-beta`) | **COMPLETE.** Symptom matrix & `.mali/milestone-2/` |
| `2026-10-30..31` | M3, M3a, M3b: Toolchain, native deps & baseline APK | `3d66ac0` (`0.2.5-beta`) | **COMPLETE.** NDK r30, CMake 3.30.4, WSL `gen` deps, M3-v1 protocol |
| `2026-10-31..11-01` | M4: Physical device baseline (`SM-X730`, BOTW `1.9.0`) | `3d66ac0` (`0.2.5-beta`) | **COMPLETE.** Native teardown hang confirmed; 24-window GP/Hateno baseline |
| `2026-11-01` | Migrations `0.2.5 -> 0.2.8 -> 0.2.10` & M5, M5b | `b02302c`, `4bad6dc` | **COMPLETE.** Pipeline-cache state explained wait; 50 ms bursts proven vsync-phase effect |
| `2026-11-01..02` | M6–M9: Vsync-phase frame pacer (`VK_GOOGLE_display_timing`) | `9c6ab0e` (`4bad6dc`) | **COMPLETE / MERGED** as **PR #183** (60 Hz bursts eliminated; P2 120 Hz analyzed) |
| `2026-11-02` | M10: Residual 120 Hz `41.7/25 ms` background audit | `9c6ab0e` | **COMPLETE.** H-A supported (Display HAL / compositor side); factual comment on `#183` |
| `2026-11-02..03` | Teleport Dissolve noise fix + G925 audit (`P1–P3`) | `050fbc2`, `0c6908d` | **COMPLETE / MERGED** as **PR #210** (BC RT GPU-to-CPU shadow & lifetime fixes) |
| `2026-11-04` | `test_nvn_lifetime`, `format-probe`, Maintenance5/empty copy | `e2da27a`, `e9ef72c` | **COMPLETE / MERGED** as **PR #245** & **PR #255**; `#244` probe posted |
| `2026-11-04..05` | Steps `11a–11b`: Benchmark probe & MVP harness | `08fb8f5` (`c48606c`) | **COMPLETE / MERGED** as **PR #278** (3 scenes, scratch teleport, `ok`/`harness_error`) |
| `2026-11-05..08` | OnePlus setup (`Turnip V38`), Ultrawide UI & Loading aspect | `c48606c`, `761dcc0` | **COMPLETE.** Local fixes in `archive/ultrawide-fixes` & `loading-screen-aspect` |
| `2026-11-08` | Benchmark v2 (`f50e05a`, `346eaea`, `60813bb`) & `scenes` | `7bb5bc9`, `113ca2e` | **COMPLETE.** Published as **PR #315** (open); 13-scene set local |
| `2026-11-08` | Memory `12a` (`LARECOMP_MEMORY_TRACE=1`) + OnePlus audit | `7bb5bc9` | **COMPLETE locally.** `.mali/memory/12a/REPORT.md` |
| `2026-11-09..13` | Devices `0.3.4`, historical Mali-G720 test (retired), `benchmark-scenes-v2` | `3d9beeb`, `7b69caa` | **COMPLETE locally.** 11-scene set `7b69caa` |
| `2026-11-14` | Pipeline prewarming `#294` (`13a`, `13b`, `13b.1`) & Git cleanup | `d0bda06` (`0.4.0-beta`) | **NO-GO / PARKED** (`13c` skipped: ready before draw, only 1/3 stutter gain); `history.bundle` |
| `2026-11-15` | Dueling Peaks water cube root cause & `soft_linear` fix | `ab828a1` (`0.4.3-beta`) | **COMPLETE / OPEN PR #436** (`VK_FORMAT_R16G16B16A16_UNORM` fallback); devices updated to `v0.4.3-beta` |
| `2026-11-15` | Mali vs. Adreno+Turnip Entry RAM & 30-min Mali VMA audit | `5946454` (`0.4.3-beta`) | **COMPLETE.** No local patch needed; Turnip-only Adreno rule enacted |
| `2026-11-16` | Underwater walk `#296`/`#438` root cause & RT sync fix | `0230d2f` (`0.4.4-beta`) | **COMPLETE / OPEN PR #461** (Cases 1 & 2, `19/19` tests, live Mali & Turnip verified) |
| `2026-11-16` | Workspace cleanup, `main` FF & Antigravity migration | `6113c61a` (`0.4.5-beta`) | **COMPLETE.** `57.59 GiB` freed, legacy remnants purged, `main` FF'd, `.agents/` + English docs published to `mali-workspace` |
