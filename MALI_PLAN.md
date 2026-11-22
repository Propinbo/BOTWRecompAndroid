# BOTWRecompAndroid Mali Contribution Plan

Authoritative engineering roadmap for owner's hardware—Galaxy Tab S11 (SM-X730, Mali-G925) and OnePlus 13 (CPH2653, Adreno 830 + Turnip)—spanning graphics, performance, memory, display/UI, input, CPU/thermals, automated testing, and upstream contributions to `https://github.com/mamcjr/BOTWRecompAndroid`.

## 1. Governance & Workflow Contract

Governance, language policy, authority boundaries, safety invariants, and verification rules are defined in root [`AGENTS.md`](AGENTS.md) and `.agents/skills/*/SKILL.md`:
- **Owner:** Defines goals, priorities, requirements, and success criteria; decides major scope changes and consequential product/architecture trade-offs.
- **Agent:** Operates with full command autonomy across Git (`origin` pushes, rebases, `main` fast-forwards, `mali-workspace` checkpoints), GitHub CLI (`gh pr`/`gh issue`), builds, and isolated ADB testing while enforcing IP, save-safety, Turnip-only Adreno, NTFS junction, and mandatory pre-PR `2–3` `independent-verifier` multi-agent review invariants.
- **Documentation checkpoint branch:** `mali-workspace` on `Propinbo/BOTWRecompAndroid` tracks only `AGENTS.md`, `MALI_PLAN.md`, `MALI_PROGRESS.md`, `MALI_STATUS.md`, and `.agents/` with `[skip ci]`. Never merge `mali-workspace` into `main` or PR branches.

## 2. Rolling Upstream Baseline

The active development target tracks the latest *verified, published* `mamcjr/BOTWRecompAndroid` GitHub Release (betas included). Campaigns pin one exact SHA and rebase/migrate cleanly onto newer verified releases when appropriate.

| Checked (UTC) | Newest Published Release | Tag SHA | Fork `main` Baseline | Relevance / Notes |
|---|---|---|---|---|
| `2026-11-22` | `v0.4.11-beta` | `afd7b84299fd3492bbfd8f9ee35065dd60f950d9` | `v0.4.11-beta` (`afd7b84`) | Fast-forwarded `main` (`1c02adf9 -> afd7b842`, `versionCode 50`, `#520` OOM retry, `#511/#521` gyro axes); `upstream/main` `75bfc64c` merged **#507** (`2e36682`). Opened **#543** (`213c521`, Fixes `#539`); deployed verified `v0.4.11-beta-mali` (`1a836dd`) on `SM-X730`. |
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
- **Merged Upstream (`v0.4.5-beta`–`v0.4.11-beta` / `upstream/main`):** `#315` (`benchmark-v2`, `ac38c7a9c4`), `#461` (`689372b`, RT upload before `LOAD_OP_LOAD`; optional `copy_texture` `gpu_owned` remains a clean future follow-up), `#436` (`soft_linear`), `#475` (`pipeline_prewarm`), `#481` (`d303dfd`, urgent shader builds + `WD_R_BUILD_WAIT`), `#482` (`6840da4`, `.smeta` v3 cache), `#483` (`5c55081`, Turnip/ARM r49+ `dyn_vi`), `#485` (`43c199a`, Mali async texture pool slots), **`#508`** (`3d28f80`, `gcopy_mode` dispatch chaining), **`#506`** (`c02beea`, `cbwin_snap` & Mali copy-mode), **`#504`** (`5df9dd9`, narrow-landscape `Unlocked`), & **`#507`** (`2e36682`, strict `"On"` Mali in-place reads + 11-locale UI note).
- **OPEN PRs (`7/7` `MERGEABLE`):** **`#543`** (`fix/narrow-shadow-539` @ `213c521`, Fixes `#539` narrow Vert+ `mAngle` shadow fix), **`#537`** (`fix/vk-descriptor-plan-dynamic-ubo` @ `c27c1b7`, `descriptor_plan()` `UNIFORM_BUFFER_DYNAMIC` + `test_descriptor_plan.py` parity), **`#534`** (`feat/vk-mali-renderpass-default` @ `9d5e732`, classic `VkRenderPass` default on Mali `r49+` + O(1) `rpcompat` hash), **`#531`** (`feat/vk-bc-parallel-decode` @ `122e11e`, 4T work-stealing CPU BC decode), **`#529`** (`feat/vk-warm-cache-fastpath` @ `ea24f43`, rebased onto `75bfc64c` / `#520` OOM retry, bounded inline warm cache + `pipelineCreationCacheControl`), **`#518`** (`feat/vk-local-read` @ `9e75f99`, `VK_KHR_dynamic_rendering_local_read` on Mali `r49+`; `mamcjr` `2026-11-22` review asks for `PW_VERSION` bump / same-att self-overlap handling + Tab S11 FX check / sRGB+CLEAR+off-path refinements), & **`#480`** (`benchmark-scenes-v2` @ `c48d1b1`).
- **Held / Local Worktrees:** `feat/vk-render-cpu-hotpath` (`e54d6a8`: `RscaleDedupe`, `soft_linear` bitmask/LUT) passed `9/9` Bionic and was folded into `CPH2653` Turnip opt (`feat/op13-turnip-opt` @ `7f4cc9a`, which also proved `CopyScratch` + `framepace` atomic cache + `exec_frame_now()` wins).
- **Consolidated Mali Release (`release/mali-0.4.11-beta` @ `1a836dd`):** Deployed `BOTWRecompAndroid-0.4.11-beta-mali-release.apk` (`68a6f611...`, cert `60fa2a5f...`, `75bfc64c` + `#534+#531+#529+#518+#537+#543`) on `SM-X730` (`40.00 fps` across `3/3` scenes at `75% SGSR1` + `shadows=1` + `inplace_reads=1`; `16:10` shadow `126.52`; `68/68` saves intact).

### B. `v0.4.5-beta` Physical Mali Validation (Archived Reference)
1. **MediaTek 13 MHz Native-Mode Clock Fix (`SM-X730` PASS):** `LARECOMP_NATIVE=1` on Dimensity 9400+ scales 13 MHz timer reads to 19.2 MHz cleanly (`30.0 fps`, `0` errors).
2. **Mali Program Eviction (`#450`) & Reboot Free (`SM-X730` PASS):** `46` reboots freed up to `386` programs / `1,055` pipelines (`0` kept); `evict5m` freed `311` programs / `779` pipelines at flat `VmRSS ~3.50 GiB`.

### C. Owner Master To-Do Roadmap (Galaxy Tab S11 + OnePlus 13 Turnip-Only)
Covers owner-selected Spheres **1, 2, 3, 4, 5, 7, 8** ranked by comfortable gameplay impact (Sphere 6 audio/Amiibo intentionally excluded per owner).

#### Active Ranked Queue
1. **Fullscreen 16:10 for Galaxy Tab S11 (Sphere 4, `#474` -> MERGED PR `#504` `5df9dd9`):** Complete & merged upstream (`v0.4.9-beta`); pending `CPH2653` Turnip cross-check when attached.
2. **Mali Perf Pack Phase 1 & Tile-Local Decals Phase 2 (Sphere 2):** **Phase 1 COMPLETE** (`#508` merged `3d28f80`, `#506` merged `c02beea`, `#507` `2e36682` rebased onto `v0.4.9-beta` & updated per `mamcjr` UI note, `OPEN` / `MERGEABLE`). **Phase 2 IMPLEMENTED & OPEN as PR `#518`** (`feat/vk-local-read` @ `9e75f99`, cleanly mergeable onto `v0.4.9-beta` `7f5afab7`; verified on `SM-X730` Mali-G925 `r49p1`: `breaks=0`, `fbcopy -70%/-90%`, `+5.1%` Zora 60p, `.smeta` v4, 31m soak; **untested on `CPH2653` Turnip**).
3. **Graphics Accuracy & Visual Fixes (Sphere 1):** Distant low grass 50–130 m async mask write-back (`512² RGBA8` / `256² RG8`, `.mali/grass/AB.md`), real `SAMPLES_PASSED` occlusion queries (Kakariko fog / shrine rays `#486`), narrow 8/16-bit integer RT masks on Mali, `#244` Vulkan UB cleanup.
4. **CPU/Thermals & Input/UX (Spheres 7 & 5):** Heterogeneous thread affinity & ADPF thermal headroom tuning on Dimensity 9400+ / Snapdragon 8 Elite (`#376`); launcher UX / RU i18n polish (`#477` fingerprint fake-gamepad bug closed upstream).
5. **Benchmarking & Automated QA (Sphere 8):** Complete PR `#480` review, `11c` metamorphic invariant checker (`0/1/125` `git bisect`), and `11d` native camera hooks (`fixes[]` in `runtime/native.c`, keeping `tools/patches.py` / `GAMELIB_ID` untouched) strictly for `SM-X730` & `CPH2653` (Turnip).

#### Deferred / Parked per Owner (`2026-11-20`)
- **Background/HOME Survival & Memory Trimming (`12b–12d / 12f`, Sphere 3):** Frozen per owner (`2026-11-20`). Preserved scope for when resumed: `12b` LMKD/FastClear survival trials (`5/30/120/600s` HOME), `12c` safe owner-thread transient/staging trim on hide (zero per-frame `vkDeviceWaitIdle`, 20-cycle hide/resume stability), `12d` `compute_cache > 256` overflow fix (`.mali/memory-20261108/compute-cache-overflow.patch`), `12f` `<= 5%` `A/B` regression (`SM-X730` + `CPH2653` Turnip), and Mali 30-min freeze reports (`#401`, `#496`).
- **Mali Grass Row Striping (Sphere 1):** `2026-11-19` `SM-X730` isolation (`.mali/grass-stripes/`) found `hateno-day/night` & `field-grass` clean and localized `canyon-shore` rows strictly to medium billboard shader `477912f75d2e8819` (`vs=3db5d8500`, `fs=3db5d9100`); `c2..c7` flag bundles (incl. `LARECOMP_NO_FILTER_FALLBACK=1`) were neutral; `frame 928` dump in `c8_debug_frame/`. Resume when `CPH2653` Turnip is attached for 1:1 compare or owner provides target location/screenshot.
- **8BitDo Pro 2 Gyroscope (Sphere 5, `#338`):** Static audit of `runtime/hle/motion.c` identified silent tablet-gyro fallback in `D`/`X` modes, `S`-mode (`057e:2009`) axis permutation (`feed()` `w = {g[0], -g[2], g[1]}` + `m_accel` Mahony drift), exact-zero deadzone packets failing `alive`/`changing >= 20` 2s guard, and shared phone/pad calibration statics (`m_gyro_ts`, `bias[3]`, `still_since`). Resume when owner shares controller mode & live gyro test.

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
