# Mali contribution execution checkpoint

Checkpoint ID: MALI-M3-2026-10-30 (milestone 3; previous checkpoint: MALI-WORKFLOW-2026-10-30).

Updated: 2026-10-30, Europe/Moscow (UTC+03:00).

## Authorization and status

Current authorization: the user explicitly commanded `Приступай к шагу 3`, asked for only milestone 3, an updated checkpoint, and a stop. Milestones 1-2 remain complete. Milestone 3 reached its acceptance alternative of a precise build blocker and a prepared measurement protocol; no APK was produced. Milestones 4-9 remain unauthorized. Project/source commits and pushes still require separate authorization; the standing exception covers only the three reviewed checkpoint documents on the fork's checkpoint branch. Historical sections below describe their own earlier tasks and do not override this current status.

| Milestone | Status |
| --- | --- |
| 1. Prepare the fork, workspace, and checkpoints | COMPLETE; preserved and reverified |
| 2. Triage the reports against current upstream | COMPLETE; stopped after triage |
| 3. Build an unchanged baseline and prepare the measurement protocol | COMPLETE via precise-build-blocker alternative; protocol ready, APK gate unresolved |
| 4. Collect reproducible baseline evidence from your physical device | Not started; awaiting explicit authorization |
| 5. Investigate one symptom with a bounded diagnostic experiment | Not started; awaiting explicit authorization |
| 6. Select one narrowly scoped contribution | Not started; awaiting explicit authorization |
| 7. Implement the selected fix and perform local verification | Not started; awaiting explicit authorization |
| 8. Validate the candidate on your device | Not started; awaiting explicit authorization |
| 9. Prepare a reviewable upstream PR package locally | Not started; awaiting explicit authorization |

All nine approved milestone definitions and their order remain unchanged in MALI_PLAN.md. The earlier documentation task added workflow/persistence guidance; milestone 3 has not revised the plan. The original complete milestone-1 checkpoint and its manifest remain at `.mali/milestone-1/MALI_PROGRESS.md` and `.mali/milestone-1/checkpoint-manifest.json`. The pre-documentation-task plan, milestone-2 progress, manifest, and exclusion rules are preserved under `.mali/workflow-persistence/`. These local historical records are not included on GitHub; their manifests describe the historical file versions.

## Milestone 3 outcome and current stopping point

- Baseline source remains `4acbf40c9312618e16e1153f3696c64e125d2a2f`, branch `main`; live origin/main and upstream/main matched at resume. Checkpoint branch before this update: `ad9225fea60ccdea474379da900d7378423bcbee`. All 52 entries of the previous local manifest matched byte count and SHA-256. Prior progress/manifest were archived under `.mali/milestone-3/` before editing.
- Tracked source and index are unchanged. The pre-existing untracked `BOTWRecompAndroid-checkpoint/` is a clean recovery clone at `ad9225f`; it was preserved. The three guidance documents and `.mali/` remain locally excluded on the source checkout. No source commit, source publication, upstream write, PR, branch switch/merge, or device operation was performed.
- Build attempted once from `android/botwrecompandroid`: `./gradlew.bat assembleDebug --no-daemon --console=plain --stacktrace`. Wrapper downloaded Gradle 8.14.3 successfully. Exit 1 after Gradle's reported 12 seconds: `java.lang.IllegalArgumentException: 25.0.4` in Kotlin's `JavaVersion.parse`, during Kotlin DSL initialization before Android/native compilation. Actual JVM is Android Studio JBR 25.0.4; Gradle embeds Kotlin 2.0.21. No APK exists; APK SHA-256 is **N/A**, not a successful baseline build.
- Pins read from unchanged source: AGP 8.13.0, Gradle 8.14.3, compile/target SDK 36, min SDK 30 (probe 31), NDK 30.0.16248370, CMake 3.30.4, arm64-v8a. Runtime CMake specifies `-O2 -g` even for debug; no compilation confirmed it. SDK 36 and Build Tools 35.0.0/36.0.0 are installed. NDK 30 and CMake 3.30.4 are missing; installed NDKs are 27.2.12479018/28.2.13676358 and SDK CMake is 3.22.1 (PATH CMake 4.4.3). A compatible JDK 17/21 was not found in checked standard locations. JDK patch is not pinned by the project. [AGP compatibility](https://developer.android.com/build/releases/agp-8-13-0-release-notes) gives JDK 17 minimum/default; [Gradle 8.14.3 release notes](https://docs.gradle.org/8.14.3/release-notes.html) document Java 24 support. Proposed recovery uses a compatible JDK 17 without updating source pins.
- Further prerequisites found missing by inventory, rather than reached as compiler errors: uninitialized libadrenotools gitlink `8fae8ce254dfc1344527e05301e43f37dea2df80` and all `android/deps/out` (SDL 3.4.16, shaderc 2026.3, Vulkan-Headers/volk SDK 1.4.350.1, VMA 3.3.0, DejaVu 2.37). Scripts include expected archive SHA-256; none of those archives was downloaded or hash-verified in this stage. FFmpeg 9.0.2 and bundled builder dependencies are also absent. Source permits omission of these optional components, but such an APK has different features and cannot silently represent a full baseline: FFmpeg supplies movies/launcher video; the builder enables on-device Recompiled setup. Windows FFmpeg/Unicorn preparation uses WSL/Linux NDK, whose readiness was not tested. No package/submodule installation was performed because preparation is substantial, triggering this milestone's checkpoint/stop condition.
- Game-code generation is separate from APK build. `android/build_gamelib.sh` was inspected, not run; no game data or generated game code was needed or acquired. Building open-source Python/Zig/etc. builder dependencies is distinct from generating the user's game library.
- Existing hooks inspected before adding any instrumentation: intent `env`, `LARECOMP_SAVE_DIR`, `LARECOMP_SETTINGS`, `LARECOMP_LOG_FILE`, LOG_PACE, VK_PROF, TIMING_TRACE, BENCHMARK_CONFIG; SPIR-V and pipeline caches; overlay and exit-info logging; Android memory/Vulkan probe. Probe can report real driver properties/capabilities and host-import checks, not game performance. Neither it nor ADB device tests were built/run; ADB was queried only for its executable version. No new runtime tooling or instrumentation introduced.
- Inspected relevant existing Vulkan/cache/timing/upload tests and Android regression runner. Three unchanged host tests were attempted with the already installed **NDK 28.2.13676358 / clang 19.0.1**, Python 3.14.7, Node 24.20.0. All exited 1 before assertions: GPU timing test had unresolved memset/memcpy, shader cache test unresolved memcpy, upload cache test undeclared ga_wrote. These are observed host-harness failures under a non-pinned compiler, not verified Android/Mali defects; no tests/source were repaired. Device runner, Vulkan capability tests requiring missing prepared headers, and game tests were not run.

Local deliverables (not published): `.mali/milestone-3/BASELINE_BUILD.md`, `DEVICE_PROTOCOL.md`, `assembleDebug.log`, `build-attempt.json`, `environment.json`, `inspect-environment.ps1`, `host-tests.json`, per-test/compiler logs, resume/state/publication records and updated manifest. The build report records exact commands, versions, missing inputs and source entry points. The protocol contains no device results.

Protocol M3-v1, fixed before measurements:

- Confirm physical model/SoC/GPU/OS/Vulkan driver, actual APK hash/signature/composition, game version/library ID and complete settings before collecting data. Use cloned seed saves through scratch-save and separate settings; preserve personal saves and hashes. Driver is system Vulkan. Native/Recompiled are paired only if both work from the same lawful data and baseline.
- Main settings: 30 FPS cap, fixed 1x internal resolution, fixed 1080-line output, 16:9, SGSR1, no dynamic resolution; record all other defaults, AA/heap/import/sync/ADPF. Screen target 60 Hz, fixed 50% brightness, ordinary power mode, no recording/boost/cleaner. Verify availability rather than assume the phone accepts each setting.
- S0 launcher/title/load control (each load bounded at 180 s); S1 fixed Great Plateau viewpoint at Shrine of Resurrection exit; S2 same seed with four 30-s phases: still, full camera rotation, fixed marked walk, still. Fix seed/camera/weather/game-time/reference frames before data; unavailable seed requires a documented protocol revision before measurements.
- S1/S2 × Native/Recompiled × app-cold/warm × 3 repetitions, each 120 s; alternate mode order R→N / N→R / R→N. Cold means separately archived/removed app SPIR-V and pipeline caches with app stopped. Warm uses one complete preparatory scene pass, saved caches, cooldown/reload and 60 s settling outside the window. Driver cache is uncontrolled/unknown unless independently established; never call app-cold fully cold. Cache manifests and compiler activity remain labelled.
- Unplugged battery 50–80%, ambient target 22±2 C, at least 5 min rest, start battery temperature <=35 C and paired starts within 1 C, normal/light thermal state. Record actual supported sensors; severe+ stops the run, preserves evidence and requires cooldown. Missing readings are N/A.
- Performance: ordinary hooks off and consistent overlay off, native app/logcat logging plus capability-checked low-overhead FrameTimeline actual-present buffer timestamps for the game layer. Validate trace loss/coverage and overhead in a separate sanity check. Fallback is verified SurfaceFlinger latency; if neither is reliable, report only supported interval FPS and no invented percentiles. UI gfxinfo/overlay snapshots do not establish Vulkan frame-time distributions. Report per-run elapsed frames/FPS, median/p95/p99 inter-present time and >50/>100 ms counts only from trustworthy timestamps; give all repetitions and variability, no selective deletion of slow frames.
- Diagnostic runs are separate IDs/tables, max 120 s × 2 per mode, using existing LOG_PACE/VK_PROF/TIMING_TRACE/scale hooks as needed; their timings are not pooled with performance. Probe/capability reports are separate. Memory/thermal before/after and supported 10-s samples are labelled by source and overhead; CPU wait is not GPU utilization.
- Stability: one 20-min warm-cache run per mode (max 40 min gameplay total), route loops and menu every 5 min, then one return to launcher/reboot, transitions bounded at 180 s. Freeze candidate is 30 s without new game frames/input response; evidence collection bounded by another 60 s. Preserve crash/ANR/logcat/exit info and separate teardown, gameplay, OS/cleaner exits and operator force-stops. Report actual duration/frequency, including incomplete runs, rather than promises of crash freedom. Visual corruption needs separately timed captures outside performance windows.

### Current blockers, proposed plan adjustment, and next authorization

The exact build blocker satisfies milestone 3's alternative acceptance criterion, but **the usable baseline APK prerequisite for milestone 4 remains unresolved**. No device tests or measurements may be claimed from this checkpoint.

Proposal only, not approved or inserted into MALI_PLAN.md: split the remaining baseline preparation into separately authorized **3a** (compatible JDK 17, exact SDK NDK/CMake and pinned submodule, Gradle configuration verification) and **3b** (hash-verified native dependencies and explicitly recorded full APK composition including FFmpeg/builder, unchanged APK/hash/signature, applicable checks). If a pinned package is unavailable, agree on any substitution first; source/test fixes are not implicitly authorized. This preserves completed milestones 1-2 and keeps step 4 behind the actual APK gate. No proposal action has been executed. Wait for approval of a plan revision and explicit execution instructions; do not automatically proceed to step 4.

Checkpoint persistence: publish only sanitized AGENTS.md, MALI_PLAN.md, MALI_PROGRESS.md on `Propinbo/BOTWRecompAndroid:codex/mali-workspace` under standing authorization. AGENTS.md and MALI_PLAN.md contents are unchanged; this update changes only MALI_PROGRESS.md within that three-file checkpoint. The containing commit SHA and independent readback evidence are recorded locally in `.mali/milestone-3/publication.json` and the final task report (the document cannot contain its own commit hash). The final local manifest covers this stage and prior retained evidence; local backup location/readback are recorded separately. Raw logs, detailed protocol and build outputs remain local. Do not infer persistence of raw evidence from remote documentation recovery.

**STOP after milestone 3.** Later stages and proposed substeps require the user's decision. The sections below are preserved historical records.

## Verified repository state at milestone 2 (historical)

- Workspace: the repository root (personal absolute path omitted from the published checkpoint).
- Branch: `main`, tracking `origin/main`.
- HEAD/base: `4acbf40c9312618e16e1153f3696c64e125d2a2f`.
- Local origin/main and upstream/main remain at that SHA; live upstream/main was checked at start and again after triage and matched. No fetch, checkout, reset, branch change or source mutation was needed in milestone 2.
- Origin fetch/push: https://github.com/Propinbo/BOTWRecompAndroid.git
- Upstream fetch/push: https://github.com/mamcjr/BOTWRecompAndroid.git
- Fork owner: Propinbo; repository ID 1440100758; parent/source mamcjr/BOTWRecompAndroid. Verified in milestone 1; sanitized metadata remains in `.mali/fork-metadata.json`.
- Tracked files and index match HEAD. No visible untracked files after local exclusions.
- Uninitialized submodule `third_party/libadrenotools` remains pinned to `8fae8ce254dfc1344527e05301e43f37dea2df80`. No build-dependency preparation performed.
- No applicable AGENTS.md in the checked workspace/ancestor paths; no nested AGENTS.md found in the current checkout. CONTRIBUTING.md was read again. Its no-game-files, scratch-save, rendering validation and reset-hook requirements remain applicable to future work.
- Local exclusions remain `/MALI_PLAN.md`, `/MALI_PROGRESS.md`, `/.mali/` in `.git/info/exclude`; tracked .gitignore unchanged.
- No application build, device connection, benchmark, instrumentation, implementation, new commit, push, issue/comment/PR mutation or evidence publication in milestone 2.

## Milestone 2 deliverables

Primary report in Russian: [.mali/milestone-2/MALI_TRIAGE.md](.mali/milestone-2/MALI_TRIAGE.md).

It contains:
- Source-linked matrix of performance loss, visual corruption, hangs, gameplay crashes, teardown crashes and external cleaner exits.
- Reporter configurations separated from evidence in logs, maintainer explanations, actual code changes and unverified hypotheses.
- Explicit treatment of 056a9a0 (0.2.3 coherent allocations) and 4a1aebd (0.2.4 cached memory plus flush/invalidate).
- Current release/tag history, post-release changes, all open PRs at inspection time, and candidate code entry points fixed to the base SHA.
- Missing evidence and limits; no result is claimed for the user's Dimensity 9400+ / Immortalis-G925 MC12.

Key findings:
1. [#89](https://github.com/mamcjr/BOTWRecompAndroid/issues/89) and [#105](https://github.com/mamcjr/BOTWRecompAndroid/issues/105) remain open. The older 0.2.3 slowdown is already addressed by [4a1aebd](https://github.com/mamcjr/BOTWRecompAndroid/commit/4a1aebd12ce14c04d10def9d78fe41278c8b4418), included in v0.2.4-beta. The attached gameplay logs already show cached type 0xb with manual maintenance.
2. Two #89 logs are different failures: a Recompiled gameplay SIGSEGV at 0x14c, and a Native-session crash after start-screen teardown, with vk_wait_seq/vk_frame_begin/launcher_run in its host stack. These are candidate problems, not one proven cause.
3. #105 has three byte-identical copies of one previous log, plus a separate current log in its ZIP. The current log reports FORCE STOP by single-cleaner for the preceding process (RSS 9177 MiB). This is not proof that the reported freeze was caused by the cleaner.
4. A direct new follow-up from #89's reporter, [#124](https://github.com/mamcjr/BOTWRecompAndroid/issues/124), reports further instability and Game Hub observations; its attached startup-only log records OneKeyClean, without the gameplay failure.
5. Sustained low-FPS samples include intervals with no shader compiles and large transient upload accounting. This supports later profiling, not a confirmed bottleneck or leak. Screenshots show low FPS, but do not establish the purple corruption/freeze event itself.
6. [#71](https://github.com/mamcjr/BOTWRecompAndroid/issues/71), [#94](https://github.com/mamcjr/BOTWRecompAndroid/issues/94), [#99](https://github.com/mamcjr/BOTWRecompAndroid/issues/99) and [#109](https://github.com/mamcjr/BOTWRecompAndroid/issues/109) were reviewed as related context. Their hardware and historical workarounds were not generalized to the user's device.
7. Latest inspected release: v0.2.4-beta, tag SHA `1e727f2903d9a2b00c4227c576fbcd1b7a66ea7b`. Cache-fix ancestry checked with git merge-base. The only open PR in the current collection is draft [#96](https://github.com/mamcjr/BOTWRecompAndroid/pull/96), AAudio recovery. Post-release merged #97 (no-FFmpeg build) and #103 (Turnip Adreno automatic heap) provide no verified G925 graphics fix.

## Milestone 2 evidence and validation (historical)

- All six files listed in the milestone-1 manifest matched their SHA-256 and byte count on resume. Machine record: `.mali/milestone-2/resume-integrity.json`.
- GitHub connector snapshots: `source-snapshot.json`, `related-snapshot.json`, `supplemental-snapshot.json`, `issue-109-comments.json` under `.mali/milestone-2/`. Comment body_html with temporary signed image URLs was excluded from saved snapshots.
- Nine attachments downloaded successfully from the exact issue URLs: four ZIPs, two standalone TXT files, three JPEGs. Manifest records their URL, SHA-256, size, content type and retrieval time. Connector cannot return these binary files, so direct URL downloads were used.
- ZIP entries were inspected before extraction; only named application logs were extracted, never executed. Entry hashes and sizes in `archive-manifest.json`.
- Seven log files on disk represent five unique contents by SHA-256. Three images visually inspected. Reproducible offline parsing: `analyze-logs.ps1`; output `log-analysis.json` and `log-samples.csv` (duplicate files remain explicitly labelled).
- Existing code/history inspected read-only. Relevant Git object IDs, release ancestry and exact code entry points appear in the report. No diagnostic hypothesis/experiment cycle was executed.
- Final milestone-2 working-tree and exclusion checks, remote-head result and checkpoint hashes were saved in `.mali/milestone-2/workspace-state.json` and the then-current `.mali/checkpoint-manifest.json` (archived under `.mali/workflow-persistence/` by this task).
- No tests were run: this milestone consists of triage and excluded local notes; no code or build inputs changed.

## Milestone 2 changed local state and persistence (historical)

- MALI_PLAN.md, original fork evidence, source files, index, Git branch/remotes and .git/info/exclude preserved.
- MALI_PROGRESS.md updated to this checkpoint.
- Added `.mali/milestone-1/` historical copies and `.mali/milestone-2/` report, source snapshots, attachments, manifests and offline parsing/download scripts. Updated the root checkpoint-manifest.json.
- The old `.mali/workspace-state.json` remains a historical milestone-1 snapshot. The milestone-2 snapshot is the current one.
- Checkpoint files survived into this separate chat and all six original hashes matched before any changes. This verifies survival between these chats in the same local environment. It does not demonstrate survival after an app restart, OS failure, or migration.
- Milestone-2 files and SHA-256 manifest were read back from a separate process before completion. That manifest covered the then-current plan, progress, evidence, historical copies and local exclusion rules; it does not hash itself and is not a signed authenticity record.
- Keep a user-controlled local/offline backup of MALI_PLAN.md, MALI_PROGRESS.md and .mali/ together. No code patch exists. No backup or evidence was uploaded automatically; the GitHub fork alone does not retain excluded files.
- Resume at this exact path by verifying the current manifest, HEAD/remotes/index/working tree and latest authorized milestone. Never force-reset an existing checkout to match this checkpoint.

## Blockers and next action (historical milestone 2)

No milestone-2 blocker remains. The report lists evidence limitations (exact reporter APK/SHA, phone/OS/build, fixed settings/scenes/cache/thermal conditions and crash/freeze traces) for later authorized work. No defect is verified on the user's actual device.

Stop after this documentation task. The next eligible project command is `Приступай к шагу 3`: only then prepare the unchanged baseline build and measurement protocol. Device testing, diagnostic experiments, implementation, and source publication remain outside current authorization. Documentation checkpoint publication is the specific standing exception below.

## Workflow and documentation-persistence agreement, 2026-10-30 (historical documentation task)

- This task is documentation and checkpoint persistence only, not execution or repetition of milestones 1-9. All 35 entries in the pre-task milestone-2 manifest matched SHA-256 and byte count; the historical milestone-1 checkpoint, milestone-2 report, and recorded repository state support preserving milestones 1-2 as COMPLETE. Milestones 3-9 remain Not started and unauthorized. No completion evidence was invented.
- Added repository-root AGENTS.md; no existing root or ancestor instructions were present to replace. MALI_PLAN.md remains authoritative. MALI_PROGRESS.md records actual progress, evidence, decisions, blockers, relevant SHAs, and the next action awaiting authorization. New sessions must consult these files and verify state without repeating completed investigations unnecessarily.
- The plan is adaptive. Handle minor clarifications within an authorized scope directly. Explain and obtain user approval before material changes to scope, priorities, dependencies, or milestone order. After approval, update MALI_PLAN.md and record the reason here, preserving completed work and historical decisions. No milestone definition/order was revised in this task. A plan proposal, revision, dependency, or completed milestone never authorizes further execution.
- Execute milestone N only after `Приступай к шагу N`; stop after that milestone, report results, and wait. Project/source commits, source pushes, and PR submission require separate authorization.
- Explicit standing exception: create and publish documentation-only checkpoints of exactly AGENTS.md, MALI_PLAN.md, and MALI_PROGRESS.md on `codex/mali-workspace` in https://github.com/Propinbo/BOTWRecompAndroid, including updates after future explicitly authorized milestones. Review these files for credentials, proprietary assets, sensitive logs, and private information first. This is a public fork; only sanitized workflow and progress summaries may be published.
- No upstream push, PR creation, branch merge, default-branch change, force-push, or source publication is authorized. Keep documentation and `.mali/` out of future upstream PRs. Never merge the checkpoint branch into a contribution branch; publish only the separately authorized source patch on a source branch.
- Before editing, archived the original plan/progress/manifest/exclusion rules under `.mali/workflow-persistence/`. The original findings and historical decisions remain intact locally. Published progress omits the personal absolute workspace path; raw logs, attachments, screenshots, metadata snapshots, and detailed working notes remain local and were not authorized for publication.

## Persistence verification and limits

- The local checkpoint files are actually available in this session; all 35 prior manifest entries matched. This supports file survival across chats in this local environment, not guaranteed survival after deletion, restart, or migration.
- Official OpenAI documentation states that Codex discovers repository AGENTS.md at session startup from the project root toward the current directory. The file must already be present before starting that session: [Custom instructions with AGENTS.md](https://learn.chatgpt.com/docs/agent-configuration/agents-md).
- Publication target: https://github.com/Propinbo/BOTWRecompAndroid/tree/codex/mali-workspace. Authenticated account and fork owner are Propinbo; the fork has push permission, upstream parent is mamcjr/BOTWRecompAndroid, and default branch is main. No checkpoint branch existed at initial inspection.
- Published first documentation revision `db0a5d2b3ce89ff19362cf6ec61c0855f3aa8e4d` on the checkpoint branch through authenticated GitHub Git-data/ref operations (equivalent remote Git storage, without a shell push). Its sole parent is the recorded application baseline; comparison against that baseline contains exactly AGENTS.md, MALI_PLAN.md, and MALI_PROGRESS.md. No application source change was published. Commit messages use `[skip ci]`; no application build was invoked.
- Independently cloned the checkpoint branch from GitHub into a new local directory without copying the original workspace or its evidence. All three Git blob IDs and text contents matched the originals (Git's Windows checkout line endings were normalized for comparison). Recovery evidence: `.mali/workflow-persistence/retrieval-first.json`, retained locally.
- Ran a genuinely fresh, ephemeral Codex CLI session (`codex-cli 0.166.0-alpha.5.1`, no resumed chat history) against that clone with the read-only sandbox. It exited successfully, read the three files, identified root AGENTS.md as active guidance, preserved complete milestones 1-2 and unauthorized milestones 3-9, and correctly stated the manual milestone command, material-plan-change approval rule, and documentation-only publication destination. No milestone, build, device operation, edit, commit, push, or network mutation was performed by the verification session. Private transcript/result and command audit remain under `.mali/workflow-persistence/` and are not published.
- During hash diagnostics an isolated clone line-ending setting temporarily made Git flag existing CRLF checkouts as modified. Restoring the inherited setting cleared those flags without writing application source files. Both the recovery clone and the original source checkout then passed working-tree/index diff checks. The first session's checkout caveat is thus resolved; its instruction-loading and status results remain valid.
- This establishes remote documentation recovery and instruction access in an independent CLI session, independent of this chat's memory and of the original local files. It does not test a desktop-app or OS restart, guarantee future GitHub/network availability, or establish persistence of the excluded raw evidence. A default-branch-only clone will not automatically contain these documents; recover the checkpoint before starting Codex.
- Only these three documentation files will be stored remotely. `.mali/` evidence, local manifests, patches, and exclusions do not accompany a fresh clone. Retain a user-controlled local/offline backup. Missing raw evidence must be disclosed, not fabricated or treated as automatic grounds to repeat a completed milestone.

## Resume in a new Codex session

Recover documentation before starting the session so root AGENTS.md can load. Use a new, nonexistent local directory for a clean recovery:

```powershell
git clone --single-branch --branch codex/mali-workspace https://github.com/Propinbo/BOTWRecompAndroid.git BOTWRecompAndroid-checkpoint
Set-Location BOTWRecompAndroid-checkpoint
git rev-parse HEAD
Get-Content AGENTS.md
Get-Content MALI_PLAN.md
Get-Content MALI_PROGRESS.md
```

Open that repository root in a new Codex session. The exact next-milestone prompt, only when the user chooses to authorize it, is:

> Read AGENTS.md, MALI_PLAN.md, and MALI_PROGRESS.md from Propinbo/BOTWRecompAndroid, checkpoint branch codex/mali-workspace. Verify the checkpoint and existing work. Milestones 1-2 are complete; preserve their findings. Приступай к шагу 3. Execute only milestone 3 and stop afterward. Do not commit or publish source changes. Update only the three checkpoint documents on codex/mali-workspace under the standing documentation authorization.

For an existing source checkout, preserve uncommitted work and compare any newer local notes before recovery. Fetch only the checkpoint branch from the verified fork, then restore only the three documentation paths into the working tree without staging them:

```powershell
git fetch --no-tags https://github.com/Propinbo/BOTWRecompAndroid.git refs/heads/codex/mali-workspace:refs/remotes/origin/codex/mali-workspace
git restore --source=origin/codex/mali-workspace --worktree -- AGENTS.md MALI_PLAN.md MALI_PROGRESS.md
```

The restore assumes these paths have no newer local content or staged changes; if they do, preserve and reconcile first. On a source branch, locally exclude `/AGENTS.md`, `/MALI_PLAN.md`, `/MALI_PROGRESS.md`, and `/.mali/` via `.git/info/exclude` (or the path reported by `git rev-parse --git-path info/exclude`). These exclusions are local and must be recreated in a new source checkout. Keep documentation untracked there; stage source paths explicitly and never cherry-pick documentation commits into a source PR or merge the checkpoint branch into a source branch. Before a source PR, verify its complete diff contains none of the checkpoint files.

Source baseline remains `4acbf40c9312618e16e1153f3696c64e125d2a2f`; a checkpoint commit records documentation changes, not a new application baseline. Verify branch, remotes, source SHA, existing patch state, and available local evidence before work. Reading/recovering this checkpoint and the example prompt above are not themselves user authorization to execute milestone 3.

## Documentation task outcome (historical)

- Created AGENTS.md and updated only the workflow/persistence portions of MALI_PLAN.md and the progress/checkpoint record here. Verified all nine milestone definitions/order, all nine status rows, and the prior key findings against the archived originals. The adaptive-planning policy and restricted documentation-publication exception are the user-approved reasons for these updates.
- Original source checkout remains on main at `4acbf40c9312618e16e1153f3696c64e125d2a2f`, with unchanged tracked files and index. No project milestone was executed; no application build, device test, source edit/commit/push, upstream write, PR, merge, default-branch change, or force-push was performed. Root AGENTS.md and the two Mali notes are untracked and locally excluded there; `.mali/` remains excluded. Tracked .gitignore is unchanged.
- Only the three reviewed documents are checkpoint changes in the fork. Historical originals, evidence, fresh-session transcripts, and verification artifacts stay local. The updated local `.mali/checkpoint-manifest.json` covers current documents/evidence after publication; the pre-task manifest remains archived. Do not compare new workflow notes against the historical milestone-2 hashes.
- The initial verified remote SHA is recorded above. This verification record is published as a subsequent documentation-only fast-forward on the same branch; obtain the current containing commit SHA with `git rev-parse origin/codex/mali-workspace` after fetching, or from this task's final report. A document cannot contain its own final commit hash. Final remote readback and local integrity results are retained in `.mali/workflow-persistence/publication.json`.
- Stop here. Milestone 3 and every later milestone still require their own explicit command. Future documentation-only updates to this same branch remain authorized after future explicitly authorized milestones; source publication remains separately controlled.
