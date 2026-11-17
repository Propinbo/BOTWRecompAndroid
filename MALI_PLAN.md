# BOTWRecompAndroid Mali Contribution Plan

Authoritative engineering roadmap for Mali-G925 / Immortalis-G720 optimization, correctness, benchmarking, and upstream contributions to `https://github.com/mamcjr/BOTWRecompAndroid`.

## 1. Governance & Workflow Contract

Governance, language policy, authority boundaries, safety invariants, and verification rules are defined in root [`AGENTS.md`](AGENTS.md) and `.agents/skills/*/SKILL.md`:
- **Owner:** Defines goals, priorities, requirements, and success criteria; decides major scope changes and consequential product/architecture trade-offs.
- **Agent:** Operates with full command autonomy across Git (`origin` pushes, rebases, `main` fast-forwards, `mali-workspace` checkpoints), GitHub CLI (`gh pr`/`gh issue`), builds, and isolated ADB testing while enforcing IP, save-safety, Turnip-only Adreno, NTFS junction, and mandatory pre-PR `2–3` `independent-verifier` multi-agent review invariants.
- **Documentation checkpoint branch:** `mali-workspace` on `Propinbo/BOTWRecompAndroid` tracks only `AGENTS.md`, `MALI_PLAN.md`, `MALI_PROGRESS.md`, `MALI_STATUS.md`, and `.agents/` with `[skip ci]`. Never merge `mali-workspace` into `main` or PR branches.

## 2. Rolling Upstream Baseline

The active development target tracks the latest *verified, published* `mamcjr/BOTWRecompAndroid` GitHub Release (betas included). Campaigns pin one exact SHA and rebase/migrate cleanly onto newer verified releases when appropriate.

| Checked (UTC) | Newest Published Release | Tag SHA | Fork `main` Baseline | Relevance / Notes |
|---|---|---|---|---|
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

### A. Upstream PR Follow-Through & Benchmark Scene Set
- **PR #315 (`MERGED` `ac38c7a9c4`):** `benchmark-v2` merged into `upstream/main`; `pr315-benchmark-v2` worktree retired.
- **PR #461 (`MERGED` `689372b`, head `7077d56` on `26da3fa`):** Upstream merged `#461` (`#296`/`#438` write-back RT upload before `LOAD_OP_LOAD`); retired `.mali/worktrees/fix-rt-load-upload-296` safely; optional `copy_texture` `gpu_owned` remains a clean future follow-up.
- **PR #480 (`OPEN`, `benchmark-scenes-v2`, `c48d1b1` on `26da3fa`, `5` files):** Rebased & opened after `#315` merge; adds Link `Y_TOLERANCE = 2.0` height verification (`y_settle`, `y_window_start`, `y_window_end`) and 12 shrine-free MainField scenes (`24/24 ok` `cap30`, `4/4 ok` `cap60`, `6/6` Bionic PASS); proactively verified `3/3` `tools/benchmark-scenes.txt` starter scenes on `SM-X730` (`plateau` `250.21`, `hateno-lab` `356.02`, `hateno-village` `242.61`, `<= 0.79` u delta) in reply to `mamcjr`.
- **PR #436 (`MERGED` into `0897fb8` / `v0.4.6-beta`):** Soft-linear format fallback merged upstream; worktree retired.
- **PR #475 (`MERGED` into `0897fb8` / `v0.4.6-beta`):** Pipeline prewarming (`#294`) merged upstream; worktree retired.
- **Shader-Stutter 3-Way PR Split (`OPEN`, rebased on `0897fb8`):** Rebased past `#436`/`#475`/`#401`, Round-3 reviewed, verified across WASM + `SM-X730` Bionic + `:app:assembleDebug`, and force-pushed (`MERGEABLE`/`CLEAN`):
  - **PR #481 (`feat/urgent-shader-builds`, `eaec823`, `5` files):** `job_queue.h`, `LARECOMP_URGENT_SHADERS`, condvar wakeups, work-stealing, `build_pipeline_urgent`, `pthread_once` pools, `jobs_drain()`, and `test_shader_job_queue.c`.
  - **PR #482 (`feat/shader-meta-cache`, `ce13c72`, `5` files):** Binary `MWSMET01` `.smeta` stage cache (`shader_meta_cache.h`, `LARECOMP_META_CACHE`), `_WIN32` rename, standalone `pthread_once` + `jobs_drain()`, cached `gmem_miss_log_on()`, and `test_shader_meta_cache.c`.
  - **PR #483 (`feat/vk-dyn-vertex-input`, `3383e1e`, `7` files):** `VK_EXT_vertex_input_dynamic_state` (`dyn_vi.h`, `LARECOMP_DYN_VI`), `SS_VI` + `vi_gen`, `ss_forget()` in internal static-VI passes, and `test_dyn_vertex_input.c`.
- **PR #485 (`OPEN`, `feat/mali-async-tex-slots`, `5eea75f` on `0897fb8`, `4` files):** Enables async texture pool slot updates by default on proprietary Mali (`vk_driver_is_mali()`, `slot_retire_pending()`, `slot_old_reset()`), verified via `test_nvn_lifetime` and `86/86` `0`-drain 2s windows on `SM-X730` (`MERGEABLE`/`CLEAN`).
- **Mali `v0.4.6-beta` Performance Optimization Pack (`feat/mali-perf-046` `83bc58c`, stacked on `5eea75f`):** Implements `cbwin_snap.h` (`CbwinBaseCache` + active-16B `CbpatchDedupe` + atomic async flush), Mali automatic `inplace_reads`, and Android copy-mode `dchain` dispatch barrier elision; verified via `12/12` Bionic tests + cooled 60 FPS `ABBA` on `SM-X730` (`+6.6%..+11.1%` FPS, `upload -83.5%`, `6/6` clean screenshots). Prepare upstream PR splits after `#482`/`#485` land.

### B. `v0.4.5-beta` Physical Mali Validation
1. **MediaTek 13 MHz Native-Mode Clock Fix (`Verified active on SM-X730`):** Under `LARECOMP_NATIVE=1` on `SM-X730` (`Dimensity 9400+`), `v0.4.5-beta` logged `counter 13000000 Hz: not the Switch's 19.2 MHz - its reads are scaled to it` (`sdk: 58 svc, 129 TLS, 43 counter reads rewritten`), running at `30.0 fps` (`0` errors).
2. **Mali Program Eviction (`#450`) & Reboot Free (`Verified 5-min + 46-reboot pass on SM-X730`):** In `set12-cap30`, `46` `[vk] reboot:` events cleanly freed up to `386` programs / `1,055` pipelines (`0` still building kept). In `evict5m` (`LARECOMP_PROG_KEEP=150, LARECOMP_PROG_AGE=20`), `7` eviction passes freed `311` programs and `779` pipeline keys with flat `VmRSS ~3.50 GiB` and `0` errors. Full 30-min baseline comparison and paired stock `v0.4.5-beta` rollout await `CPH2653` attachment & owner go-ahead.

### C. Parked Initiatives (Resumable on Owner Priority)
- **Fullscreen 16:10 (`archive/fullscreen-16x10` on `v0.3.1-beta`):**
  - *Done:* Phase 1 (world aspect projection + fit-to-width UI on `SM-X730`).
  - *Remaining Goals & Success Criteria:* Phase 2 vertical HUD anchors (`E = 360*(1/g - 1)` layout formula, consistent pane/text/scissor alignment, touch-shortcut verification, zero unintended shift in central windows/menus); Phase 3 map/menus/Inventory Link scaling and ultrawide cross-device regression on `CPH2653` (Turnip).
- **Memory & Background Retention (`12b–12d / 12f`, formerly `codex/memory-background`):**
  - *Updated by upstream `v0.4.5-beta`:* Parked **`12e` (in-process reset shader/pipeline free) is now addressed upstream in `v0.4.5-beta`**, eliminating duplicate local reset work.
  - *Remaining Goals & Success Criteria (if resumed on updated upstream):* Complete `12b` background survival trials (`5/30/120/600s` HOME/Settings, LMKD vs ColorOS FastClear separation); evaluate `12c` safe owner-thread transient/staging trim on hide (zero `vkDeviceWaitIdle` per frame, verified 20-cycle hide/resume stability); evaluate `12d` dynamic compute-cache index above 256 keys (`.mali/memory-20261108/compute-cache-overflow.patch`); run `12f` paired `A/B` regression (`CPH2653` Turnip + `SM-X730`, `<= 5%` frame-time envelope).
- **Benchmark Check Layer & Native Hooks (`11c–11d`):**
  - *11c:* Automated `TEST_STRATEGY.md` check loop (`T1/T3/L1/P1` metamorphic invariants with red negative controls and `0/1/125` `git bisect run` exit codes).
  - *11d:* Native-only absolute camera / fly-through hooks in `runtime/native.c` (`fixes[]`), leaving `tools/patches.py` and `GAMELIB_ID` untouched.

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
| `2026-11-09..13` | Devices `0.3.4`, `SM-X926B` setup, `benchmark-scenes-v2` | `3d9beeb`, `7b69caa` | **COMPLETE locally.** 11-scene set `7b69caa`; `SM-X926B` ready |
| `2026-11-14` | Pipeline prewarming `#294` (`13a`, `13b`, `13b.1`) & Git cleanup | `d0bda06` (`0.4.0-beta`) | **NO-GO / PARKED** (`13c` skipped: ready before draw, only 1/3 stutter gain); `history.bundle` |
| `2026-11-15` | Dueling Peaks water cube root cause & `soft_linear` fix | `ab828a1` (`0.4.3-beta`) | **COMPLETE / OPEN PR #436** (`VK_FORMAT_R16G16B16A16_UNORM` fallback); devices updated to `v0.4.3-beta` |
| `2026-11-15` | Mali vs. Adreno+Turnip Entry RAM & 30-min Mali VMA audit | `5946454` (`0.4.3-beta`) | **COMPLETE.** No local patch needed; Turnip-only Adreno rule enacted |
| `2026-11-16` | Underwater walk `#296`/`#438` root cause & RT sync fix | `0230d2f` (`0.4.4-beta`) | **COMPLETE / OPEN PR #461** (Cases 1 & 2, `19/19` tests, live Mali & Turnip verified) |
| `2026-11-16` | Workspace cleanup, `main` FF & Antigravity migration | `6113c61a` (`0.4.5-beta`) | **COMPLETE.** `57.59 GiB` freed, legacy remnants purged, `main` FF'd, `.agents/` + English docs published to `mali-workspace` |
