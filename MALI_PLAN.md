# BOTWRecompAndroid Mali contribution plan

Approved in chat on 2026-10-30. This is the authoritative milestone plan. The original nine milestone definitions remain preserved. On 2026-10-30 the user approved splitting the remaining baseline preparation into milestones 3a and 3b after the completed milestone-3 investigation. Actual execution status, evidence, decisions, blockers, and relevant commit SHAs belong in MALI_PROGRESS.md.

Upstream: https://github.com/mamcjr/BOTWRecompAndroid

Current source baseline after the explicitly authorized bounded synchronization on 2026-10-30: **`3d66ac0482ae9d99546a579170675dd6eb3c047c`**, upstream `v0.2.5-beta` / main. Fork main and local source were advanced fast-forward from `4acbf40c9312618e16e1153f3696c64e125d2a2f`. Completed milestones 1-3 and 3a remain complete; historical evidence and 3a environment/validation are preserved. The synchronization authorizes neither 3b nor 4.

Planned test hardware: MediaTek Dimensity 9400+, Arm Immortalis-G925 MC12. Hardware and device results must be verified from real device evidence during the authorized device milestones.

Execution authorization: `Приступай к шагу N` authorizes only milestone N. Dependencies and completing a milestone do not authorize other milestones. Do not automatically advance. Project/source commits, source pushes, publishing raw evidence, and PR submission require separate explicit authorization. The documentation-only checkpoint exception below does not authorize project milestones. Preserve existing work. Use only lawfully provided game data; never acquire or redistribute proprietary game assets.

## Adaptive planning and manual approval

Reassess remaining milestones as evidence emerges, prioritizing the project objective over an outdated hypothesis or redundant work. Minor clarifications within the currently authorized scope may be handled directly. Material changes to scope, priorities, dependencies, or milestone order (including adding, removing, splitting, or merging milestones) must be explained and proposed for user approval before execution. After approval, update this plan, preserve completed milestones, and record the reason in MALI_PROGRESS.md. A proposed or revised plan never authorizes execution of additional milestones.

## Documentation persistence authorization

On 2026-10-30 the user authorized a documentation/checkpoint task only, plus standing documentation-only updates after future explicitly authorized milestones: commit and publish exactly `AGENTS.md`, `MALI_PLAN.md`, and `MALI_PROGRESS.md` to `codex/mali-workspace` in the existing fork https://github.com/Propinbo/BOTWRecompAndroid. No project milestone is authorized by that task. Review the three files for credentials, proprietary assets, sensitive logs, and private information before publication. Raw evidence and other working notes remain local.

Do not push to upstream, open a PR, merge branches, change the default branch, force-push, or publish source changes under this authorization. Keep checkpoint documentation out of future upstream PRs: never merge this branch into a contribution branch; carry only the independently authorized source patch. On source branches keep the three documentation files locally excluded from staging, along with `.mali/`. A clean clone of the checkpoint branch contains the documentation; a clone of `main` needs explicit recovery. See MALI_PROGRESS.md for retrieval steps and verified persistence limits.

The bounded synchronization separately authorized importing upstream commits into fork main only by fast-forward. It did not expand standing source-publication permission. The documentation branch intentionally retains its historical application tree and is updated with documents only; its application tree is not the current build baseline. Recover documents into a current main checkout without merging checkpoint history into source.

## 1. Prepare the fork, workspace, and checkpoints.

**Scope:** After `Приступай к шагу 1`, recheck existing work and applicable instructions; identify your authenticated GitHub account; reuse a suitable fork or create one through available authorized tooling. Prepare the checkout with `origin` pointing to your fork and `upstream` to `mamcjr/BOTWRecompAndroid`, preserving existing work.

**Deliverables:** `MALI_PLAN.md`, `MALI_PROGRESS.md`, recorded remotes, branch, base SHA, working-tree state, and applicable contributor rules. Keep planning files and evidence locally excluded from upstream changes.

**Acceptance criteria:** Verify fork ancestry, remotes, and preservation of existing work. The planning inspection found the current workspace empty, without a Git checkout or applicable local `AGENTS.md`; upstream's inspected tree also contained no `AGENTS.md`. Recheck after checkout and follow [CONTRIBUTING.md](https://github.com/mamcjr/BOTWRecompAndroid/blob/4acbf40c9312618e16e1153f3696c64e125d2a2f/CONTRIBUTING.md).

**Dependencies:** Explicit authorization for this milestone and sufficient GitHub permissions. Authentication failures are blockers, not permission to bypass access controls.

**Stopping condition:** Workspace and initial checkpoint are ready; no build, commit, or push.

**Execution rules for every milestone:** Start only after `Приступай к шагу N`; check prerequisites, existing progress, and repository state without repeating completed investigations unnecessarily. Record the relevant SHA, completed work, findings with references, validation evidence, changed files/state, blockers, and next action awaiting authorization. Report the checkpoint location and persistence status. Verify checkpoint existence and recorded hashes when resuming; consult MALI_PROGRESS.md for actual persistence tests and their limits. Preserve a user-controlled backup of local raw evidence and any uncommitted patch; only the three reviewed documentation files may be published under the standing checkpoint authorization. If retrieval is unavailable or persistence uncertain, report the blocker and provide a portable checkpoint. Stop after the authorized milestone; if it grows too large, document a coherent checkpoint and propose separately authorized substeps. No automatic advance.

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

**Scope:** Establish actual device access through authorized ADB or your supplied test results. Record phone model, Android/build, reported GPU, Vulkan driver, application SHA/APK hash, game version, settings, and test conditions. Run the selected scenarios and compare Native/Recompiled modes where available.

**Deliverables:** Reproduction steps, raw application logs/logcat, available crash evidence, visual captures, measured frame times/FPS, and supported memory/thermal observations. Preserve saves through the project's scratch-save mechanism where applicable.

**Acceptance criteria:** Evidence identifies a repeatable symptom and its frequency, or documents bounded unsuccessful reproduction attempts and missing information. Distinguish cold shader compilation, sustained slowdown, visual corruption, and crashes. Every claimed device result must have corresponding real evidence.

**Dependencies:** Completed milestone-3 protocol, a usable baseline APK produced by milestone 3b after verified milestone 3a, device access or supplied results, and your lawful game data. A recorded build blocker does not satisfy the APK prerequisite.

**Stopping condition:** Baseline evidence package is complete, or a specific access/data blocker is recorded. No inferred hardware results or fix implementation.

## 5. Investigate one symptom with a bounded diagnostic experiment.

**Scope:** Trace the selected symptom through relevant code. Follow evidence into NVN command translation, Vulkan allocation/cache management, synchronization and resource lifetime, shader/texture compatibility, or Native/Recompiled differences as warranted. Start with existing diagnostics; isolate any necessary temporary instrumentation.

**Deliverables:** Referenced code paths, an explicit hypothesis, a discriminating experiment, results, and rejected explanations.

**Acceptance criteria:** Complete one bounded hypothesis/experiment cycle. Clearly separate verified defects from plausible explanations. Diagnostic changes and measurements must be identifiable and reproducible.

**Dependencies:** Milestone 2 and usable baseline evidence from milestone 4.

**Stopping condition:** Reach a supported diagnosis, reject the hypothesis, or identify the next missing measurement. Further investigation requires another explicitly authorized scope.

## 6. Select one narrowly scoped contribution.

**Scope:** Choose a fix supported by the investigation and recheck overlapping upstream changes.

**Deliverables:** A short proposal describing the defect, intended behavior, affected code, exclusions, regression risks, and validation criteria.

**Acceptance criteria:** The proposal addresses one independently reviewable problem, explains why the change should work, and defines measurable success before coding. If evidence is insufficient or upstream already resolves it, record that outcome without inventing a fix.

**Dependencies:** Milestone 5 provides adequate evidence.

**Stopping condition:** One concrete proposal is ready for authorization; no production implementation.

## 7. Implement the selected fix and perform local verification.

**Scope:** Implement only the proposal selected in milestone 6, following surrounding conventions and contributor rules. Remove unrelated diagnostic changes. Add a meaningful regression test when feasible; otherwise document why device validation is necessary.

**Deliverables:** Focused uncommitted patch, build results, relevant test results, and a reproducible candidate APK with its hash.

**Acceptance criteria:** The application builds, applicable checks pass, and the diff contains only the intended change and necessary tests. Respect reset-hook requirements where applicable. Exclude planning files, logs, generated game code, and proprietary assets.

**Dependencies:** Milestone 6.

**Stopping condition:** Candidate patch is ready for device validation. No claim of a verified rendering fix, commit, or push.

## 8. Validate the candidate on your device.

**Scope:** Compare unchanged baseline and candidate under the recorded protocol. Repeat the reproducer, relevant mode comparisons, focused rendering checks, and the predefined stability run.

**Deliverables:** Paired raw evidence, comparison results, crash/reproduction counts, visual observations, and limitations.

**Acceptance criteria:** Demonstrate the selected fix against its predefined criteria, with no observed relevant regression. Performance claims must account for run variability and thermal/cache conditions. Report crash-free testing as a measured duration, not proof that crashes are impossible. Follow the project's requirement to observe rendering changes working in game.

**Dependencies:** Milestones 4 and 7; real device evidence.

**Stopping condition:** Record pass, failure, or inconclusive results. A failure does not authorize another implementation cycle automatically.

## 9. Prepare a reviewable upstream PR package locally.

**Scope:** Recheck current upstream for overlapping fixes and review the final patch. Prepare the PR title/body, reproduction instructions, explanation, validation summary, and limitations.

**Deliverables:** Clean proposed diff, local PR draft, evidence references suitable for sharing, and a portable resumption checkpoint.

**Acceptance criteria:** The contribution addresses one problem; claims match actual evidence; required checks and device validation are documented; planning/checkpoint files, proprietary assets, generated game code, and unrelated changes are absent. If upstream movement requires adaptation, record that as separately authorized work.

**Dependencies:** Successful milestone 8 and completed local verification.

**Stopping condition:** Stop with the package ready for your review. Committing, pushing a branch, publishing evidence, and submitting the upstream PR each require explicit authorization for that action.
