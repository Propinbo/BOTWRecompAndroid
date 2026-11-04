# BOTWRecompAndroid Mali contribution plan

Approved in chat on 2026-10-30. This is the authoritative milestone plan. The original nine milestone definitions remain preserved. On 2026-10-30 the user approved splitting the remaining baseline preparation into milestones 3a and 3b after the completed milestone-3 investigation. On 2026-11-01 the user approved adding milestone 5b (the 50 ms pacing bursts) between milestones 5 and 6. Actual execution status, evidence, decisions, blockers, and relevant commit SHAs belong in MALI_PROGRESS.md.

Upstream: https://github.com/mamcjr/BOTWRecompAndroid

Current active source baseline after the third authorized upstream migration phase (2026-11-04): **`06111e4c981a101ea7e8d2b1b9e709667f3e057b`**, upstream `v0.3.0-beta`. No APK was built or installed for this baseline: the tablet still runs the previous test APK `9a141dc5…` (0.2.13-based). Fork main and local `main` were fast-forwarded `3d66ac0` (`v0.2.5-beta`, the milestone 3b/4 baseline) → `b02302c` (`v0.2.8-beta`) → `4bad6dc` (`v0.2.10-beta`) → `06111e4`; the 0.2.5 baseline itself came by fast-forward from `4acbf40c9312618e16e1153f3696c64e125d2a2f` on 2026-10-30. Completed milestones and phases keep the SHAs they used. See "Rolling upstream baseline" for the last release check.

Planned test hardware: MediaTek Dimensity 9400+, Arm Immortalis-G925 MC12. Hardware and device results must be verified from real device evidence during the authorized device milestones.

Execution authorization: `Приступай к шагу N` authorizes only milestone N. Dependencies and completing a milestone do not authorize other milestones. Do not automatically advance. Project/source commits, source pushes, publishing raw evidence, and PR submission require separate explicit authorization. The documentation-only checkpoint exception below does not authorize project milestones. Preserve existing work. Use only lawfully provided game data; never acquire or redistribute proprietary game assets.

## Adaptive planning and manual approval

Reassess remaining milestones as evidence emerges, prioritizing the project objective over an outdated hypothesis or redundant work. Minor clarifications within the currently authorized scope may be handled directly. Material changes to scope, priorities, dependencies, or milestone order (including adding, removing, splitting, or merging milestones) must be explained and proposed for user approval before execution. After approval, update this plan, preserve completed milestones, and record the reason in MALI_PROGRESS.md. A proposed or revised plan never authorizes execution of additional milestones.

## Documentation persistence authorization

The active checkpoint branch is `mali-workspace` in the existing fork https://github.com/Propinbo/BOTWRecompAndroid. On 2026-10-30 the user authorized a documentation/checkpoint task only, plus standing documentation-only updates after future explicitly authorized milestones: commit and publish exactly `AGENTS.md`, `MALI_PLAN.md`, and `MALI_PROGRESS.md` to the checkpoint branch, then named `codex/mali-workspace`. On 2026-11-04 the user added `MALI_STATUS.md` (a short current-state file read at session start) to this list, so the checkpoint is now four files. On 2026-10-31 the user separately authorized its official GitHub rename to `mali-workspace` and migration of active recovery references. This changes the branch name, not milestone scope, order, status, or execution authorization. No project milestone is authorized by either maintenance task. Review the four files for credentials, proprietary assets, sensitive logs, and private information before publication. Raw evidence and other working notes remain local.

Do not push to upstream, open a PR, merge branches, change the default branch, force-push, or publish source changes under this authorization. Keep checkpoint documentation out of future upstream PRs: never merge this branch into a contribution branch; carry only the independently authorized source patch. On source branches keep the four documentation files locally excluded from staging, along with `.mali/`. A clean clone of the checkpoint branch contains the documentation; a clone of `main` needs explicit recovery. See MALI_PROGRESS.md for retrieval steps and verified persistence limits.

The bounded synchronization separately authorized importing upstream commits into fork main only by fast-forward. It did not expand standing source-publication permission. The documentation branch intentionally retains its historical application tree and is updated with documents only; its application tree is not the current build baseline. Recover documents into a current main checkout without merging checkpoint history into source.

## Rolling upstream baseline

User-approved on 2026-11-01; the workflow rule is in `AGENTS.md`. The active target is the latest *verified, published* upstream release (GitHub Releases, betas included). Releases are checked at session start, before each milestone, before selecting or implementing a fix, before preparing the PR, and about every 6 h in long sessions. A campaign pins one SHA and finishes on it. A new release never triggers an automatic rebuild, restart, or retest loop; migration is proposed at the next safe checkpoint boundary and needs authorization. There is no background monitoring.

Release-check record (newest first):

| Checked (UTC) | Newest published release | Tag SHA | Active baseline | Note |
|---|---|---|---|---|
| 2026-11-04T19:55Z | `v0.3.1-beta` (published 14:03:45Z) | `c48606cfafaaa4f41083830bbec2fd8b9ab01b47` | `v0.3.1-beta` `c48606c` | Step 11b refinements; tag = `upstream/main`, no newer release. |
| 2026-11-04T18:38Z | `v0.3.1-beta` (published 14:03:45Z) | `c48606cfafaaa4f41083830bbec2fd8b9ab01b47` | `v0.3.1-beta` `c48606c` | End of step 11b, pinned to this SHA; tag = `upstream/main`, no newer release. |
| 2026-11-04T15:58Z | `v0.3.1-beta` (published 14:03:45Z) | `c48606cfafaaa4f41083830bbec2fd8b9ab01b47` | `v0.3.1-beta` `c48606c` | Start of step 11a; campaign pinned to this SHA. Tag = `upstream/main`. |
| 2026-11-04T15:08Z | `v0.3.1-beta` (published 14:03:45Z) | `c48606cfafaaa4f41083830bbec2fd8b9ab01b47` | `v0.3.1-beta` `c48606c` | Tag = upstream main. Migration authorized by the user; fork and local main fast-forwarded from `06111e4`, no force. Campaign `pr-maintenance5` pinned to it, rechecked unchanged before the push. |
| 2026-11-04T07:39Z | `v0.3.0-beta` (published 02:47:32Z) | `06111e4c981a101ea7e8d2b1b9e709667f3e057b` | `v0.3.0-beta` `06111e4` | Tag = upstream main. Migration authorized by the user; fork and local main fast-forwarded from `4bad6dc`. Rechecked unchanged at the end of the port. |
| 2026-11-03T08:30Z | `v0.2.13-beta` | `5403f006905343792cc1692a3ea9c4fca89c1506` | Fork main `4bad6dc`, review port based on `5403f00` | Session-start release check for review of `050fbc2`. |
| 2026-11-03T06:40Z | `v0.2.13-beta` (published 03:40:51Z; 0.2.12 at 2026-11-02T20:47Z) | `5403f006905343792cc1692a3ea9c4fca89c1506` | `v0.2.10-beta` `4bad6dc` (teleport-dissolve patch campaign finished on it) | v0.2.13 touches `exec.c` `begin_pass` (DEAD VIEW check), `wb_eligible`, `vk_collect_dead`/`vk_image_view`; the teleport-dissolve diff applies cleanly in a dry run. The user then authorized a local commit of the patch and a port to v0.2.13 (see MALI_PROGRESS.md); this changes no fork `main`. |
| 2026-11-02T17:28Z | `v0.2.11-beta` (published 15:55:35Z) | `3556864abc50b1bb7f0e89705f8364167ee0d272` | `v0.2.10-beta` `4bad6dc` (no migration authorized) | 0.2.11 = 3 commits over 4bad6dc: stick dead-zone floor 8% (`runtime/hle/input.c`), in-place mods toggle (`runtime/mods.c`), version bump. No rendering, Vulkan, pacing or texture changes; does not address #174 and does not touch #183 files (#183 merges onto 0.2.11 without conflicts). Maintainer replied on #174: could not reproduce on Adreno 830 Native (10.5 min standing); asks for Native `LARECOMP_BC1_ASTC=0` and/or 0.2.9 on the same route, plus a log of a run showing the defect. #183: no maintainer response yet. |
| 2026-11-02T08:41Z, rechecked 10:03Z | `v0.2.10-beta` (published 15:14:53Z) | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.10-beta` `4bad6dc`; milestone 10 pinned `pace-vsync-phase` `9c6ab0e` | No newer release or main commit during milestone 10; PR #183 open, no comments. |
| 2026-11-02T06:33Z | `v0.2.10-beta` (published 15:14:53Z) | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.10-beta` `4bad6dc` | No newer release or main commit during milestone 9. |
| 2026-11-02T05:13Z | `v0.2.10-beta` (published 15:14:53Z) | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.10-beta` `4bad6dc` | No newer release or main commit at end of milestone 8. |
| 2026-11-01T18:56Z, rechecked 19:47Z | `v0.2.10-beta` (published 15:14:53Z) | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.10-beta` `4bad6dc` (milestone 7 pinned) | No newer release or main commit during milestone 7. |
| 2026-11-01T18:47Z, rechecked 18:54Z | `v0.2.10-beta` (published 15:14:53Z) | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.10-beta` `4bad6dc` (milestone 6 pinned) | No newer release or main commit during milestone 6. |
| 2026-11-01T18:15Z, rechecked 18:39Z | `v0.2.10-beta` (published 15:14:53Z) | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.10-beta` `4bad6dc` (milestone 5b pinned) | No newer release or main commit during milestone 5b. |
| 2026-11-01T17:44Z, rechecked 18:07Z | `v0.2.10-beta` (published 15:14:53Z) | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.10-beta` `4bad6dc` (milestone 5 pinned) | No newer release or main commit during milestone 5. |
| 2026-11-01T17:36Z | `v0.2.10-beta` (published 15:14:53Z) | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.10-beta` `4bad6dc` (migrated; campaign pinned) | No newer release or untagged main commit after the 0.2.10 regression. |
| 2026-11-01T15:17Z, rechecked 15:18Z before publication | `v0.2.10-beta` (published 15:14:53Z); `v0.2.9-beta` `b753dcd` published 12:24:22Z | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.8-beta` `b02302c` (campaign pinned) | Both appeared during the pinned 0.2.8 regression; not switched mid-campaign. Migration proposed below. |
| 2026-11-01T09:53Z | `v0.2.8-beta` (published 2026-10-31T21:53:54Z) | `b02302c73fc2a0df7bd70b2e51a56d9d99af9421` | `v0.2.5-beta` `3d66ac0` → migrated to `b02302c` | No newer untagged commits at that time. |

## 4→5. Upstream migration phase: 0.2.5-beta → 0.2.8-beta (COMPLETE, 2026-11-01)

**Authorization:** The user's pasted migration-phase instruction of 2026-11-01. It authorizes a fast-forward-only update of local and fork `main` to the verified release, an unchanged rebuild with the verified 3a/3b environment, an in-place install, a bounded device regression with read-only root sampling, the rolling-upstream policy, and a three-document checkpoint. It does not authorize milestone 5, source changes, or an upstream PR.

**Outcome:** Main was fast-forwarded to `b02302c`, the APK rebuilt and installed in place, and the bounded regression completed. Results and evidence are in MALI_PROGRESS.md. The game library was not rebuilt because the ABI ID was unchanged.

## 4→5b. Upstream migration phase: 0.2.8-beta → 0.2.10-beta (COMPLETE, 2026-11-01)

**Authorization:** The user's "Migrate to 0.2.10" in answer to the proposal recorded at the 0.2.8 checkpoint. Same shape and limits as the 0.2.8 phase: FF-only main update, unchanged rebuild, in-place install, bounded regression with read-only root sampling, three-document checkpoint. During the phase the user also instructed skipping the recharge before the Recompiled long session, and approved filing an upstream issue for a new visual defect (filed from the user's account via Claude in Chrome). It does not authorize milestone 5, source changes, or an upstream PR.

**Outcome:** Main fast-forwarded to `4bad6dc`, APK rebuilt and installed in place (ABI unchanged), regression completed with one truncated session. Results and evidence are in MALI_PROGRESS.md.

## Upstream migration phase: 0.2.10-beta → 0.3.0-beta (COMPLETE, 2026-11-04)

**Authorization:** The user's 2026-11-04 instruction: FF of fork main under the `AGENTS.md` rules and a record here, with no rebuild and no install on the tablet. Then local ports of `audit-fixes-0213` and `vuid-fixes-0213` as `audit-fixes-030` and `vuid-fixes-030`, without `diag:` commits and without push or PR.

**Outcome:** Main fast-forwarded to `06111e4`; the two ports exist locally. No device regression was run on 0.3.0, because none was authorized. Results are in MALI_PROGRESS.md.

## Reassessment of milestones 5-9 after the 0.2.10 regression (proposal; not approved)

Superseded proposal: the earlier "migrate to 0.2.10 before milestone 5" item is done.

- **Measured on 0.2.10 and changed vs earlier baselines:**
  - GPU memory halved (1,440 MB vs 2,914 MB);
  - no thermal escalation in either mode (Android thermal status stayed 0, against SEVERE 2/2 for Recompiled on 0.2.5);
  - a new Native-only terrain/grass defect (upstream issue mamcjr/BOTWRecompAndroid#174).

  The Recompiled thermal symptom is therefore no longer a milestone 5 candidate unless it recurs.
- **Milestone 5 candidates on 0.2.10, re-ranked:**
  1. The Native terrain/grass sinking defect. It is reproducible within ~10 min on this route, Native only, and absent on 0.2.5. Upstream now knows (#174) and ships quickly, so investigate only if #174 stays unfixed in the next release. Otherwise a cheap step is a bisect between 0.2.8 and 0.2.10 or a flag test (`LARECOMP_BC1_ASTC=0`) to give upstream a precise cause.
  2. The Native teardown hang: 1 of 18 on 0.2.5, 0 of 8 Native returns since (4 on 0.2.8, 4 on 0.2.10). Not reproducible cheaply; keep only as an observation.
  3. Pipeline-creation wait regression: a fixed small batch waits 104-167 ms on 0.2.10 (174-182 ms on 0.2.8, 13-34 ms on 0.2.5). Small, measurable, Mali-specific candidate. *Resolved by milestone 5:* this is a pipeline-cache state effect (15 ms on 0.2.10 with the batch cached), not a regression.
  4. 50 ms pacing bursts at the 30 FPS cap (still present, up to ~1,400 intervals in a segment).
  5. Upload volume of ~930 MB per 2 s (#89, #161). The maintainer has said heap import for Mali/Xclipse is the next big piece of work, so this has a high risk of duplicating upstream.
- **Process proposal (not approved):** with upstream releasing every few hours, use tiered per-release checks:
  - a short smoke check (install, one short cell per mode, visual check), about 30 min;
  - long thermal sessions only when a release touches power, thermal or memory paths;
  - milestone work on a pinned SHA, with overlap rechecked at fix selection and before the PR.

  An AC-side smart plug would remove the manual charger steps; see `.mali/charging-feasibility/REPORT.md`.
- **Milestones 6-9:** definitions unchanged; compare against the active baseline at that time.

## 1. Prepare the fork, workspace, and checkpoints.

**Scope:** After `Приступай к шагу 1`, recheck existing work and applicable instructions; identify your authenticated GitHub account; reuse a suitable fork or create one through available authorized tooling. Prepare the checkout with `origin` pointing to your fork and `upstream` to `mamcjr/BOTWRecompAndroid`, preserving existing work.

**Deliverables:** `MALI_PLAN.md`, `MALI_PROGRESS.md`, recorded remotes, branch, base SHA, working-tree state, and applicable contributor rules. Keep planning files and evidence locally excluded from upstream changes.

**Acceptance criteria:** Verify fork ancestry, remotes, and preservation of existing work. The planning inspection found the current workspace empty, without a Git checkout or applicable local `AGENTS.md`; upstream's inspected tree also contained no `AGENTS.md`. Recheck after checkout and follow [CONTRIBUTING.md](https://github.com/mamcjr/BOTWRecompAndroid/blob/4acbf40c9312618e16e1153f3696c64e125d2a2f/CONTRIBUTING.md).

**Dependencies:** Explicit authorization for this milestone and sufficient GitHub permissions. Authentication failures are blockers, not permission to bypass access controls.

**Stopping condition:** Workspace and initial checkpoint are ready; no build, commit, or push.

**Execution rules for every milestone:** Start only after `Приступай к шагу N`; check prerequisites, existing progress, and repository state without repeating completed investigations unnecessarily. Record the relevant SHA, completed work, findings with references, validation evidence, changed files/state, blockers, and next action awaiting authorization. Report the checkpoint location and persistence status. Verify checkpoint existence and recorded hashes when resuming; consult MALI_PROGRESS.md for actual persistence tests and their limits. Preserve a user-controlled backup of local raw evidence and any uncommitted patch; only the four reviewed documentation files may be published under the standing checkpoint authorization. If retrieval is unavailable or persistence uncertain, report the blocker and provide a portable checkpoint. Stop after the authorized milestone; if it grows too large, document a coherent checkpoint and propose separately authorized substeps. No automatic advance.

## 2. Triage the reports against current upstream.

**Scope:** Review [#89](https://github.com/mamcjr/BOTWRecompAndroid/issues/89), [#105](https://github.com/mamcjr/BOTWRecompAndroid/issues/105), their available attachments, related changes, releases, and relevant pending PRs. Extend the planning inspection where necessary.

**Deliverables:** A source-linked symptom matrix covering performance loss, visual corruption, freezes/crashes, reported configurations, existing fixes, and missing evidence.

**Acceptance criteria:** Account explicitly for the existing Mali caching change, [commit `4a1aebd`](https://github.com/mamcjr/BOTWRecompAndroid/commit/4a1aebd12ce14c04d10def9d78fe41278c8b4418), released in 0.2.4-beta. Separate reporter observations, maintainer explanations, code changes, and unverified hypotheses. Treat the reports' hardware as related evidence, without assuming equivalence to your Dimensity 9400+/Immortalis-G925 MC12.

**Dependencies:** Milestone 1.

**Stopping condition:** Document remaining candidate problems and relevant code entry points; no implementation or device testing.

## 3. Build an unchanged baseline and prepare the measurement protocol.

**Historical completion:** The original investigation is complete through its precise-build-blocker acceptance alternative, with protocol M3-v1 and evidence retained under `.mali/milestone-3/`. No baseline APK was produced. The original definition below is preserved; the approved remaining preparation is now 3a then 3b. Completing the investigation does not satisfy the actual APK gate for device testing.

**Scope:** Check the pinned Android toolchain and dependencies, then build the unchanged application at a recorded SHA. Inspect existing diagnostic hooks, the Android probe, and relevant tests before introducing tooling. Keep game-code generation separate and perform it only if necessary using your lawfully supplied data.

**Deliverables:** Build commands, toolchain/dependency versions, build log, APK hash, and a concise device-testing protocol.

**Acceptance criteria:** Produce a traceable baseline APK or identify the precise build blocker. Define scenes, settings, Native/Recompiled comparisons, shader-cache state, repetitions, measurement methods, thermal conditions, and bounded stability-test duration before measuring. Separate diagnostic runs from performance runs.

**Dependencies:** Milestones 1-2; available build dependencies.

**Stopping condition:** Baseline build and protocol are ready. If dependency preparation becomes substantial, checkpoint it and propose smaller substeps before continuing.

## 3a. Prepare and verify the pinned build environment.

**Approval and authorization:** On 2026-10-30 the user approved the proposed 3a/3b split and explicitly instructed execution of milestone 3a only. This English execution instruction is explicit authorization for 3a; approval of the split alone does not authorize 3b or any later milestone.

**Scope:** Select a compatible JDK 17 for the build process without changing the user's global Java settings or the project's Gradle/AGP pins. Prepare exactly Android NDK 30.0.16248370 and SDK CMake 3.30.4, verify SDK 36 and required Build Tools, and initialize/verify libadrenotools at the recorded gitlink with its required pinned submodules. Preserve existing installations and source work. Verify Gradle configuration without assembling an APK or building native dependencies.

**Deliverables:** Reproducible setup/configuration commands and logs; JDK distribution/version and archive verification; exact SDK package versions; submodule SHAs and cleanliness; Gradle configuration result; updated checkpoint with local raw evidence retained separately.

**Acceptance criteria:** A compatible JDK 17 and exact pinned NDK/CMake can be invoked, SDK prerequisites and submodule pins are verified, and unchanged-project Gradle configuration succeeds, or a precise environment/configuration blocker is documented. Inspect actual package availability before substitution. If an exact pinned package is unavailable, stop and seek agreement before any replacement or source-pin change; no silent use of an installed alternative. Distinguish package/network/access failures from application defects.

**Dependencies:** Preserved completed milestones 1-3; explicit execution authorization for 3a; access to official toolchain distributions.

**Stopping condition:** Environment/configuration verification is ready, or the exact blocker is checkpointed. Do not run dependency-build scripts, assemble/package the APK, generate game code, run device tests, repair source/tests, or proceed to 3b. Source commits/publication still need separate authorization.

## 3b. Build native dependencies and the unchanged baseline APK.

**Synchronization addendum, 2026-10-30:** Use the new baseline `3d66ac0482ae9d99546a579170675dd6eb3c047c` (0.2.5-beta). Milestone 3a remains COMPLETE and is not reopened: Gradle/AGP/JDK/SDK/NDK/CMake/dependency-script/submodule requirements are unchanged, and existing installation records match. No environment delta or reinstall is required by this source update. New-source configuration/native-build validation belongs to 3b; the historical 3a Gradle success remains tied to its original SHA. Keep the agreed full FFmpeg/builder composition and previously planned WSL/Linux build prerequisites. Preserve M3-v1 and add current-baseline/cache/known-issue metadata when preparing the new APK, without repeating completed triage. In later device/investigation stages, account for upstream GPU-memory-table/reset/exit changes linked to #89, surface-recovery fix #108 and shader material-ID rounding #112; do not assume a Mali result or select a duplicate fix. This addendum is planning only, not execution authorization.

**Scope:** After separate explicit execution authorization, prepare open-source native dependencies using the pinned scripts and verified archive SHA-256 values. Record a full baseline composition including FFmpeg and the on-device Recompiled builder; distinguish building that open-source builder from generating game code. Use the verified 3a environment to build the unchanged application at the recorded source SHA. Reuse the completed milestone-3 protocol and diagnostic inspection, updating minor practical details only with recorded reasons.

**Deliverables:** Dependency versions/hashes and build commands/logs, explicit APK composition, baseline APK with SHA-256 and signing-certificate identity, applicable check results and limitations, preserved protocol/evidence and updated checkpoint.

**Acceptance criteria:** Produce a traceable unchanged-source APK with the agreed full composition and applicable verification, or document the precise dependency/build blocker. Recheck existing host-test blockers with the pinned toolchain; source/test repairs and pinned-package substitutions require separate agreement. Keep any necessary game-code generation separate, using only lawfully supplied data; do not perform device measurements in this milestone.

**Dependencies:** Milestone 3a provides a usable verified environment; separately authorized execution of 3b and available dependency sources. A checkpointed blocker in 3a does not imply its environment is ready.

**Stopping condition:** Baseline APK/hash/signature and protocol are ready, or an exact blocker is recorded. No automatic advance to milestone 4, device test, implementation, source commit/push, or PR.

## 4. Collect reproducible baseline evidence from your physical device.

**Approved preparation addendum, 2026-10-31:** After the initial device/data-blocker checkpoint, the user explicitly instructed doing everything needed to prepare the supplied game and identified their local PC container folder. Milestone 4 may now prepare isolated open-source decompression/extraction tools, use the user’s existing local key material without publishing it, extract the user-supplied base/update/DLC into a separate local output, validate the actual supported game build and save compatibility, and transfer the prepared data for the existing device protocol. Preserve original containers/saves and completed evidence. Use no externally acquired game assets or keys. This authorizes resolving the data prerequisite and continuing step 4 only; it does not authorize source/test fixes, source publication or milestone 5.

**Approved comparison preparation, 2026-10-31:** After device setup the user explicitly selected “Подготовить Great Plateau и Recompiled для полного сравнения”. Preserve the original Great Plateau S1/S2 target and both-mode comparison: prepare a lawful scratch seed at that location through the supplied save/game, and generate/build the matching 1.9.0 Recompiled game library using the existing unchanged project workflow and pinned toolchain. Record generator/dependency versions, flags, ABI ID and library hash. Do not substitute the proposed Native-only alternative scene matrix or a shortened check. Existing setup/capability captures remain preparation evidence; no controlled baseline window has completed. This is still milestone 4 preparation/execution, not source-fix or milestone-5 authorization.

**Approved first-pair exception, 2026-10-31:** After the resumed S1/app-cold first pair measured a 1.4 C start-temperature difference, the user explained that the tablet had been brought indoors from outside and explicitly instructed accepting the tolerance. Accept this first pair with that documented exception, preserving actual temperatures and the original failed <=1 C check. Later pairs retain the original <=1 C criterion; do not retroactively change measured values or claim strict compliance.

**Scope:** Establish actual device access through authorized ADB or your supplied test results. Record phone model, Android/build, reported GPU, Vulkan driver, application SHA/APK hash, game version, settings, and test conditions. Run the selected scenarios and compare Native/Recompiled modes where available.

**Deliverables:** Reproduction steps, raw application logs/logcat, available crash evidence, visual captures, measured frame times/FPS, and supported memory/thermal observations. Preserve saves through the project's scratch-save mechanism where applicable.

**Acceptance criteria:** Evidence identifies a repeatable symptom and its frequency, or documents bounded unsuccessful reproduction attempts and missing information. Distinguish cold shader compilation, sustained slowdown, visual corruption, and crashes. Every claimed device result must have corresponding real evidence.

**Dependencies:** Completed milestone-3 protocol, a usable baseline APK produced by milestone 3b after verified milestone 3a, device access or supplied results, and your lawful game data. A recorded build blocker does not satisfy the APK prerequisite.

**Stopping condition:** Baseline evidence package is complete, or a specific access/data blocker is recorded. No inferred hardware results or fix implementation.

## 5. Investigate one symptom with a bounded diagnostic experiment.

**Status (2026-11-01): COMPLETE** on `4bad6dc`. The selected symptom was the small-batch pipeline-creation wait. It is explained by pipeline-cache state, not by slower compiles, and a small unsaved-session-tail defect was verified. Details are in MALI_PROGRESS.md. Milestone 6 is not authorized.

**Scope:** Trace the selected symptom through relevant code. Follow evidence into NVN command translation, Vulkan allocation/cache management, synchronization and resource lifetime, shader/texture compatibility, or Native/Recompiled differences as warranted. Start with existing diagnostics; isolate any necessary temporary instrumentation.

**Deliverables:** Referenced code paths, an explicit hypothesis, a discriminating experiment, results, and rejected explanations.

**Acceptance criteria:** Complete one bounded hypothesis/experiment cycle. Clearly separate verified defects from plausible explanations. Diagnostic changes and measurements must be identifiable and reproducible.

**Dependencies:** Milestone 2 and usable baseline evidence from milestone 4.

**Stopping condition:** Reach a supported diagnosis, reject the hypothesis, or identify the next missing measurement. Further investigation requires another explicitly authorized scope.

## 5b. Investigate the 50 ms pacing bursts at the 30 FPS cap.

**Approval (2026-11-01):** after milestone 5, the user selected this symptom and approved adding this milestone ("Одобряю 5b"). Approval is not execution authorization: execution needs `Приступай к шагу 5b`.

**Status (2026-11-01): COMPLETE** on `4bad6dc`. The bursts are a phase effect between the runtime's free-running 33.333 ms present grid and the display's vsync latch, confirmed causally by a phase-sweep experiment at 60 and 120 Hz. Details are in MALI_PROGRESS.md. Milestone 6 is not authorized.

**Symptom:** at the 30 FPS cap on a 60 Hz panel, the mean holds 30 FPS but frames go to the screen in bursts that alternate 50.0 / 16.7 ms (one vsync late, then one early) instead of 33.3 ms. Examples: 906 of 3,600 intervals were 50 ms in GP-R-S2-cold-1; bursts occurred in 7 of 24 M4 windows; long runs of 515 intervals (0.2.8 Native) and 1,398 intervals (0.2.10 Recompiled). The cause is not established. All measurements so far were at 60 Hz; the device default is 120 Hz.

**Scope:** pin the newest verified release at the start (currently v0.2.10-beta `4bad6dc`). Trace the frame path from game-frame readiness to present: frame limiter (`runtime/framepace.h`), async render and present (`runtime/nvn/vulkan/async.c`, `device.c`), swapchain present mode, and ADPF. Start with existing diagnostics (`timing_trace.c`, `LARECOMP_LOG_POWER`). Add temporary instrumentation only if they are insufficient, isolated and identifiable. Form one explicit hypothesis, for example "frame readiness sits near a vsync boundary". Test it with a discriminating device experiment comparing frame readiness with actual present (SurfaceFlinger) at 60 and 120 Hz, Native and Recompiled where needed. Diagnostic runs stay separate from performance runs. USB power is allowed as a recorded deviation, as in milestone 5. Restore the user's display settings afterwards.

**Deliverables:** referenced code paths, the hypothesis, the experiment, results, rejected explanations, and local raw evidence under `.mali/milestone-5b/`.

**Acceptance criteria:** one bounded hypothesis/experiment cycle. Verified defects stay clearly separated from plausible explanations. Diagnostic changes and measurements are identifiable and reproducible.

**Estimated duration:** about 2-2.5 h in total: code reading 30-45 min, device runs 1-1.5 h (4-8 windows of 2-5 min, about 5 min of cooling before each), analysis and recording 15-20 min.

**Dependencies:** milestone 4 evidence; milestone 5 complete.

**Stopping condition:** a supported diagnosis, a rejected hypothesis, or the next missing measurement identified. No source fix, commit or PR. Milestone 6 then selects one contribution using the evidence from milestones 5 and 5b.

## 6. Select one narrowly scoped contribution.

**Status (2026-11-01): COMPLETE** on `4bad6dc`. Selected: vsync-phase-locked present pacing for the 50 ms bursts (candidate (b)), using `VK_GOOGLE_display_timing` past presentation timing to keep the queue at about half a refresh period before the latch. The pipeline-cache save (candidate (a)) is deferred. The proposal is in MALI_PROGRESS.md; local copy at `.mali/milestone-6/M6_PROPOSAL.md`. Milestone 7 is not authorized; its first step is feasibility gate G0.

**Scope:** Choose a fix supported by the investigation and recheck overlapping upstream changes.

**Deliverables:** A short proposal describing the defect, intended behavior, affected code, exclusions, regression risks, and validation criteria.

**Acceptance criteria:** The proposal addresses one independently reviewable problem, explains why the change should work, and defines measurable success before coding. If evidence is insufficient or upstream already resolves it, record that outcome without inventing a fix.

**Dependencies:** Milestone 5 provides adequate evidence. Milestone 5b's evidence (pacing-burst cause) is also available.

**Stopping condition:** One concrete proposal is ready for authorization; no production implementation.

## 7. Implement the selected fix and perform local verification.

**Status (2026-11-01): COMPLETE** on `4bad6dc`. Gate G0 passed. The candidate patch (4 files, +133/−8) is uncommitted in the working tree. Candidate APK `f4482815…` is built and not installed. Device smoke runs found and fixed two steering defects. Evidence-driven parameter changes against the milestone 6 proposal stay within its mechanism. Details are in MALI_PROGRESS.md. Milestone 8 is not authorized.

**Scope:** Implement only the proposal selected in milestone 6, following surrounding conventions and contributor rules. Remove unrelated diagnostic changes. Add a meaningful regression test when feasible; otherwise document why device validation is necessary.

**Deliverables:** Focused uncommitted patch, build results, relevant test results, and a reproducible candidate APK with its hash.

**Acceptance criteria:** The application builds, applicable checks pass, and the diff contains only the intended change and necessary tests. Respect reset-hook requirements where applicable. Exclude planning files, logs, generated game code, and proprietary assets.

**Dependencies:** Milestone 6.

**Stopping condition:** Candidate patch is ready for device validation. No claim of a verified rendering fix, commit, or push.

## 8. Validate the candidate on your device.

**Status (2026-11-02): COMPLETE with one failed criterion (P2).** On `4bad6dc` the candidate meets P1, P3, P4, P5, S1 and S2 (60 Hz bursts removed, long session 37379 frames without a burst run above 4) but fails P2 at 120 Hz (max block 0.253 vs limit 0.16; residual background attributed to the compositor, criterion not relaxed). S3 is partial: 60 FPS in-world and movie smoke checks remain open. Failure does not authorize another implementation cycle; milestone 9 is not authorized. Details in MALI_PROGRESS.md.

**Scope:** Compare unchanged baseline and candidate under the recorded protocol. Repeat the reproducer, relevant mode comparisons, focused rendering checks, and the predefined stability run.

**Deliverables:** Paired raw evidence, comparison results, crash/reproduction counts, visual observations, and limitations.

**Acceptance criteria:** Demonstrate the selected fix against its predefined criteria, with no observed relevant regression. Performance claims must account for run variability and thermal/cache conditions. Report crash-free testing as a measured duration, not proof that crashes are impossible. Follow the project's requirement to observe rendering changes working in game.

**Dependencies:** Milestones 4 and 7; real device evidence.

**Stopping condition:** Record pass, failure, or inconclusive results. A failure does not authorize another implementation cycle automatically.

## 9. Prepare a reviewable upstream PR package locally.

**Status (2026-11-02): COMPLETE** on `4bad6dc`. The package (clean 4-file patch, PR draft with title, body, reproduction steps and limitations) is local; no overlap with upstream found, claims in the draft match the M8 evidence, including the failed P2 and the open smoke checks. Nothing was committed, pushed or submitted. Details in MALI_PROGRESS.md.

**Scope:** Recheck current upstream for overlapping fixes and review the final patch. Prepare the PR title/body, reproduction instructions, explanation, validation summary, and limitations.

**Deliverables:** Clean proposed diff, local PR draft, evidence references suitable for sharing, and a portable resumption checkpoint.

**Acceptance criteria:** The contribution addresses one problem; claims match actual evidence; required checks and device validation are documented; planning/checkpoint files, proprietary assets, generated game code, and unrelated changes are absent. If upstream movement requires adaptation, record that as separately authorized work.

**Dependencies:** Successful milestone 8 and completed local verification.

**Stopping condition:** Stop with the package ready for your review. Committing, pushing a branch, publishing evidence, and submitting the upstream PR each require explicit authorization for that action.

**Submission (2026-11-02):** on the user's "Отправлять PR как есть": commit `9c6ab0e` on fork branch `pace-vsync-phase`, upstream PR mamcjr/BOTWRecompAndroid#183.

## 10. Investigate the residual 120 Hz 41.7 / 25 ms background.

**Approval (2026-11-02):** the user approved adding this milestone ("Да, сформируй новый план для этой ошибки"). Approval is not execution authorization: execution needs `Приступай к шагу 10`.

**Status (2026-11-02): COMPLETE** on `9c6ab0e`. H-A supported: FrameTimeline shows the display HAL presenting one vsync later than SurfaceFlinger expects on ~90% of display frames and on time on the rest; the 25 / 41.7 ms pairs are those on-time presents, while app queue, SF arrival and latch are identical across interval classes. V1 and V3 were excluded by mechanism after step 2; at the user's choice only V2 (`setFrameRate(30)`) was run against V0, and it made the background worse. Details in MALI_PROGRESS.md. No further milestone is authorized.

**Symptom:** at the 30 FPS cap on the 120 Hz panel (device default), with the PR #183 pacing, about 17% of intervals per 150-frame block are 41.7 / 25 ms instead of 33.3 ms (M8: mean block share 0.175, max 0.253 with sweep; 0.176 / 0.333 without). It does not depend on the grid phase. The late-shown frames were queued and GPU-ready on time. Misses are isolated, in runs of 2-4. Working attribution: compositor side. Not proven. This is a different mechanism from the phase bursts fixed by #183.

**Hypotheses (one bounded cycle discriminates them):**
- **H-A, compositor/display-side misses** that the app cannot influence. Prediction: no app-side lever changes the background. FrameTimeline classifies the misses as SurfaceFlinger or display-HAL jank, not app jank.
- **H-B, latch-target ambiguity:** with `desiredPresentTime = 0`, SurfaceFlinger latches "the first vsync after the queue". At 120 Hz, its scheduling or prediction picks the wrong vsync. Prediction: a `desiredPresentTime` set to the intended vsync (from the #183 anchor and period) removes most of the background.
- **H-C, refresh-rate selection:** without a frame-rate vote, SurfaceFlinger or Samsung's adaptive refresh heuristics make the 120 Hz grid irregular for a 30 fps layer. Prediction: `ANativeWindow_setFrameRate(30, FIXED_SOURCE)` or a periodic refresh change in the SurfaceFlinger dump coincides with the misses and changes the background.

**Scope:**
1. Release and PR check at the start: newest upstream release, the state of #183 and maintainer comments. Pin the SHA, which is `pace-vsync-phase` `9c6ab0e` on `4bad6dc` unless upstream has moved. A move means proposing an adaptation first.
2. **Cheap, no code (about 30 min):** on the #183 candidate APK, 120 Hz, Native S1 with warm cache, capture one 180 s window. Record SurfaceFlinger `--latency` together with FrameTimeline jank classification (Perfetto `android.surfaceflinger.frametimeline` or `dumpsys SurfaceFlinger --frametimeline`, whichever the device supports) and the active display mode over time. If the misses classify clearly as SurfaceFlinger or display jank and no lever looks applicable, the cycle can end here with H-A supported.
3. **Discriminating experiment:** one temporary, isolated, env-gated diagnostic patch on top of the candidate, kept local and never part of a PR:
   - V0, candidate as is;
   - V1, `desiredPresentTime` = intended vsync (H-B);
   - V2, `setFrameRate(30, FIXED_SOURCE)` (H-C);
   - V3, lead of 2 ms and 6 ms instead of r/2 (confirms phase-independence).

   Each variant: 120 Hz, Native S1, warm cache, 60 s settle + 180 s window, 2 windows, about 5 min cooling between windows, USB power (recorded deviation, as in 5b/8). Any variant that removes the background also gets one 60 Hz window, to check P1 still holds.
4. Analyse with the M8 block-share tooling, the same 150-frame blocks and burst definitions. Restore afterwards: baseline APK `e4a62881…` reinstalled and hash-verified, refresh keys deleted, brightness 88 and auto mode, ADB over USB.

**Success / decision criteria (fixed before measuring):**
- A lever "works" if both its windows reach mean block share ≤ 0.05 and max ≤ 0.16 at 120 Hz (P2), with FPS within ±0.05 of V0 and P1 still met at 60 Hz.
- "No effect" if every variant stays within the V0 window-to-window range. Then H-A is supported: record compositor-side, outside the app, and offer a factual comment on #183.

**Deliverables:** referenced code and system paths, the hypotheses, the FrameTimeline classification, the variant table, rejected explanations, local raw evidence under `.mali/milestone-10/`, and the diagnostic patch with its SHA-256.

**Estimated duration:** about 2.5-3 h in total. Start checks and FrameTimeline capture 30-40 min. Diagnostic patch and build 30-40 min. Device runs about 1.5 h (up to 9 windows of about 4 min plus about 5 min cooling each). Analysis and recording 20 min.

**Dependencies:** milestone 8 evidence; PR #183 branch.

**Stopping condition:** a supported diagnosis (a working lever, or H-A supported), or the next missing measurement identified. There is no production fix, commit, push or PR in this milestone. A working lever becomes a separately proposed implementation milestone and a separate PR, or an update to #183 if the maintainer prefers. Each needs its own authorization. The open S3 checks from milestone 8 (60 FPS in-world, movie) are not part of this milestone.

## 11. Built-in benchmark and test suite.

**Approval (2026-11-04):** the user approved adding this milestone and its split after the maintainer's answer in #214: scenes with teleport, time and weather; memory writes behind the benchmark switch on a scratch save; hooks that change `tools/patches.py` or `GAMELIB_ID` only Native for the prototype or held for a release that already rebuilds; 1.9.0 only, with `unsupported version` on others; fixed cap through env for measurement. Each substep runs only on its own command. Background: `.mali/benchmark/FEASIBILITY.md`, `.mali/benchmark/TEST_STRATEGY.md`.

### 11a. Device probe.

**Status (2026-11-04): COMPLETE** on `c48606c`. Teleport works only through the scratch save (with an in-place reboot); camera read, closed-loop stick, weather, time, confirmed fast travel and game function calls work; the clock relation holds, but both its controls stayed green (`control_missing`). Results are in `.mali/benchmark/FEASIBILITY.md` and MALI_PROGRESS.md. Step 11b is not authorized.

**Scope:** about 2.5 h on the tablet, base `v0.3.1-beta` `c48606c`, scratch copies only. Commands instead of input; where input is unavoidable, each step is confirmed by game state. Temporary patches stay outside git history. Items (a)-(k) as in the user's instruction: `LARECOMP_PLACE` and `LARECOMP_DOORJUMP_TEST`; teleport; camera read and closed-loop stick; time and weather held for 45 s with Blood Moon reset; fast travel by confirmed input; `cap30` and `fps_mode=4` passes with display rate and actual FPS target (`LARECOMP_FPS=40` not used); three runs of one scene; Native vs Recompiled; clock speed independent of FPS; game function call without patch changes, stopped at the first failure.

**Done when:** each item in FEASIBILITY.md reads confirmed, not confirmed or fallback only, with evidence; real durations are in TEST_STRATEGY.md; the tablet runs stock 0.3.1 and that is verified; the result is in MALI_PROGRESS.md.

### 11b. Benchmark MVP, no hooks in `tools/patches.py`.

**Status (2026-11-04): COMPLETE** on branch `benchmark-mvp`, head `6a808ef` after refinements (base `c48606c`), local only. The user's 11b command narrowed the contract below: `cap30` only (no `fps_mode=4`), statuses `ok`/`cache_cold`/`harness_error` (no `pass`/`fail`), no headroom field (derivable from `frames.csv`). All three scenes ran 3 repeats in R and N with no `harness_error`; 13 of 18 windows `cache_cold`. Details in MALI_PROGRESS.md. Steps 11c and 11d are not authorized.


**Done when:** behind the benchmark switch, off by default and with no change in behaviour when off, a run on 1.9.0 goes from start to report on a scratch save without user input and rejects other versions with `unsupported version`. Scenes come from the methods 11a confirmed, with arrival checked by Link's coordinates. The report gives per-frame CPU and GPU time, p50/p95/p99, headroom against budget, repeats with median and spread, a passport without serial numbers, and the `pass`/`fail`/`harness_error` statuses. `cap30` and `fps_mode=4` are both reported with display rate and actual target. `GAMELIB_ID` does not change. Upstream PR only on separate authorization.

### 11c. Check layer per TEST_STRATEGY.md.

**Done when:** the minimal loop from TEST_STRATEGY section 5 runs on the 11b machinery (T1, T3, L1, P1, with statuses and report); thresholds come from A/A runs; every binding test has seen its negative control go red on the device, or is reported as `control_missing`; `git bisect run` exit codes work on at least one known defect. Real durations replace the estimates.

### 11d. What needs hooks, Native only.

**Done when:** absolute camera (and the path without gravity and collisions, if taken up) works behind the switch in Native through `fixes[]` in `native.c`, with `tools/patches.py` and `GAMELIB_ID` unchanged and Recompiled reporting the feature as unavailable. Stop criterion from FEASIBILITY.md holds: if Link cannot complete 60 s without death or state change within the spike, the path is dropped and scenes stay on the pan.
