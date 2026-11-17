# BOTWRecompAndroid Mali Contribution Plan

Authoritative engineering roadmap for Mali-G925 / Immortalis-G720 optimization, correctness, benchmarking, and upstream contributions to `https://github.com/mamcjr/BOTWRecompAndroid`.

## 1. Governance & Workflow Contract

Governance, language policy, authority boundaries, safety invariants, and verification rules are defined in root [`AGENTS.md`](AGENTS.md) and `.agents/skills/*/SKILL.md`:
- **Owner:** Defines goals, priorities, requirements, and success criteria; explicitly approves external actions (`git push`, PRs/Issues, `main` fast-forwards), protected-file edits (`tools/patches.py`, `GAMELIB_ID`, `.gitignore`), major scope changes, and physical device access/upgrades.
- **Agent:** Operates as an autonomous senior engineer and researcher owning technical investigation, architecture, implementation, local worktree commits, testing, debugging, and proportional situational verification.
- **Documentation checkpoint branch:** `mali-workspace` on `Propinbo/BOTWRecompAndroid` tracks only `AGENTS.md`, `MALI_PLAN.md`, `MALI_PROGRESS.md`, `MALI_STATUS.md`, and `.agents/` with `[skip ci]`. Never merge `mali-workspace` into `main` or PR branches.

## 2. Rolling Upstream Baseline

The active development target tracks the latest *verified, published* `mamcjr/BOTWRecompAndroid` GitHub Release (betas included). Campaigns pin one exact SHA and finish on it; migrations require owner approval.

| Checked (UTC) | Newest Published Release | Tag SHA | Fork `main` Baseline | Relevance / Notes |
|---|---|---|---|---|
| `2026-11-16 18:54` | `v0.4.5-beta` | `6113c61a1d0e6f78532eb7fdde9cc9348613ad4b` | `v0.4.5-beta` (`6113c61`) | Fast-forwarded fork/local `main` (`7bb5bc9 -> 6113c61`). Restart shader free, `#450` Mali shader eviction >1000, MediaTek 13 MHz Native clock fix, full-span copy texture hash, vertex cache retention. PRs `#461` (`6f83821`), `#436` (`f92454e`), & `#315` (`d928ccc`) updated locally onto `6113c61`. |
| `2026-11-16 05:11` | `v0.4.4-beta` | `280048bc43b05d76af0659a51c828ff393d4434e` | `v0.3.2-beta` (`7bb5bc9`) | PR `#461` (`0230d2f`) branched here; `0780cb6` (`#450`) merged cleanly (`git merge-tree`). |
| `2026-11-15 10:39` | `v0.4.3-beta` | `5946454c9fbdc414e0ab025f751f9c2011d23c3b` | `v0.3.2-beta` (`7bb5bc9`) | Installed stock APK on `SM-X730` & `CPH2653` (`3d3ad8e7...`); PR `#436` (`ab828a1`) branched here. |
| `2026-11-14 08:34` | `v0.4.0-beta` | `d0bda062b382334d70f709fc65c2d80a669f643c` | `v0.3.2-beta` (`7bb5bc9`) | Used for `#294` pipeline prewarming `13a–13b.1` and initial Water Cube root-cause proof. |
| `2026-11-08 07:12` | `v0.3.2-beta` | `7bb5bc98fae6093da3c2dd8f5af3c756b59ee7d5` | `v0.3.2-beta` (`7bb5bc9`) | Fast-forwarded fork `main` after PR `#255` & `#278` merged. |
| `2026-11-04 15:08` | `v0.3.1-beta` | `c48606cfafaaa4f41083830bbec2fd8b9ab01b47` | `v0.3.1-beta` (`c48606c`) | Fast-forwarded fork `main`; used for `11a–11b`, PR `#255`, PR `#278`, ultrawide/16:10 work. |
| `2026-11-01 17:36` | `v0.2.10-beta` | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.10-beta` (`4bad6dc`) | Used for Milestones 5–10, PR `#183`, and initial Teleport Dissolve (`#210`). |

Full 83-row historical release-check polling log (2026-10-31 to 2026-11-16): `.mali/archive/release-checks-2026-10-to-11.md`.

## 3. Active & Parked Engineering Initiatives

### A. Upstream PR Follow-Through & Benchmark Scene Set
- **PR #461 (`fix/rt-load-upload-296`, local `6f83821` on `6113c61`, `6` files):** Rebased and narrowed strictly to `#296`/`#438` write-back RT sync (`LOAD_OP_LOAD` + `undiscard()`). `13/13` Bionic checks PASS (`10/13` fail on unpatched `6113c61`), `SM-X730` Run A verified (`30.0 fps`, `93` write-backs, `0` errors). Patch + English reply (`.mali/pr-reviews-20261117/reply-pr461.md`) ready for owner push approval.
- **PR #436 (`fix/soft-linear-filter`, local `f92454e` on `6113c61`, `6` files):** Updated with lazy `vk_any_format_no_linear()`, `CLAMP`/`REPEAT` wrap support (`soft_linear_rep_u`/`soft_linear_rep_v`, required because BOTW `64x64 f0x2a` water clipmaps bind `wrap 1`), `soft_linear_mip`, and `last_prog` pre-population. `test_soft_linear.py` + `SM-X730` `#461 + #436` Run B eliminate phantom water cleanly (`30.0 fps`, `0` errors). Patch + English reply (`.mali/pr-reviews-20261117/reply-pr436.md`) ready for owner push approval.
- **PR #315 (`pr315-benchmark-v2`, local `d928ccc` on `6113c61`, `16` files):** Rebased cleanly with `seq_wait_done` + `BP_GPU_WAIT_NS`, restored top-of-file headers, and `test_nvn_lifetime.c` low-RAM stubs. `6/6` Bionic + `assembleDebug` PASS. Patch + English reply (`.mali/pr-reviews-20261117/reply-pr315.md`) ready for owner push approval.
- **Benchmark Scenes v2 (`benchmark-scenes-v2`, `7b69caa`):** Audited in `.mali/benchmark/scenes-v2/CANDIDATE_ANALYSIS_2026_11_17.md`: found `4` defective views (`lake-hylia-shore`, `faron-woods-pond`, `faron-canopy`, `lurelin-rain`) plus biome redundancy and DVFS `cap30` compression (`26.56-28.06 ms`); proposed upgraded `12`-scene suite incorporating verified clean shrine-free candidates (`zora-plaza`, `faron-jungle`, `akkala-foliage`, `lake-hylia-shore v3`, `lurelin-shore`) plus candidates needing single-scene validation (`korok-plaza`, `dueling-peaks-river` / `gerudo-gate-day`) and pairing `fps_mode=2`/`0` runs.

### B. `v0.4.5-beta` Physical Mali Validation Candidates
1. **MediaTek 13 MHz Native-Mode Clock Fix (`Verified active on SM-X730`):** During `2026-11-17` `LARECOMP_NATIVE=1` hardware verification on `SM-X730` (`Dimensity 9400+`), `v0.4.5-beta` logged `counter 13000000 Hz: not the Switch's 19.2 MHz - its reads are scaled to it` and `sdk: 58 svc, 129 TLS, 43 counter reads rewritten (scaled to 19.2 MHz)`, running smoothly at `30.0 fps` (`max 35ms`, `0` errors). Paired formal stock `v0.4.5-beta` rollout (`SM-X730` + `CPH2653`) awaits owner go-ahead.
2. **Long-Session Mali Shader Eviction (`#450`, Awaiting Owner Go-Ahead):** Re-run the 30-min Mali diagnostic protocol (`.mali/memory-long-mali-20261115/`) on `v0.4.5-beta` vs the `v0.4.3-beta` baseline (`+625.06 MiB` `VmRSS`, `1,214` pipelines evicted, `576 MB` driver `scudo` retention) to measure real-world `VmRSS`/program/pipeline impact and check for eviction-induced stutter.

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
