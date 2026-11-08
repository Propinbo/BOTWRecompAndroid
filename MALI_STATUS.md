# MALI status (read this first; keep it short)

Updated: 2026-11-08 (Europe/Moscow). History: `MALI_PROGRESS.md`, select the needed heading.

## Flag-off overhead follow-up — COMPLETE
- Audit/source commit `60813bb`: atomic updates and normal diagnostic clocks were already off. Fixed eager argument evaluation (extra BC clock only when separate TEXTURE_TRACE enabled) with one lazy bool guard; decode predicates use bool without clock. Tests/build PASS; old helper rejected by negative control.
- ABBAAB, A=stock032, B=60813bb without benchmark flag;3 boots each, one Hateno scene,100s, last900 frames/boot. Stock CPU p50:18.690/19.430/19.820ms, median19.430, range1.130ms(5.82%). V2:19.180/19.540/18.980ms, median19.180, range0.560ms(2.92%). Difference−0.250ms(−1.29%) is smaller than both ranges. Stable overhead was not detected; previous single-pair+2.76% is consistent with noise, not established counter cost.
- All starts thermal0; first5 ends0, finalB ended1. Zero periods>50ms, zero benchmark logs/artifacts. This comparison cannot resolve a small bool-branch cost or prove zero overhead; B was the corrected60813bb, not a repeat series of oldf50e05a.
- All41 files match today's verified backup; original APK9e11940d…54fe restored with -r-d, app stopped, all6 scratch roots removed. Player/save/settings/cache guards matched throughout.
- Evidence `.mali/benchmark/off-abba/REPORT.md`, `summary.json`, `series.json`, per-run logs/screenshots/guards, tests/build/negative logs, `restored.json`, `cleanup.json`. Local commits only, no push. Release check08:00UTC unchanged v0.3.2-beta/7bb5bc9.

## Base and checkouts
- Active release baseline: `v0.3.2-beta`, tag = upstream/main = fork main = local main `7bb5bc98fae6093da3c2dd8f5af3c756b59ee7d5`. GitHub Releases checked 2026-11-08 07:12 and 07:29 UTC; published 04:57:53Z. Both main refs advanced from c48606c after ancestor checks; ordinary FF push to fork main only.
- Current managed worktree: `C:/Users/propinbo/.codex/worktrees/benchmark-v2/BOTWRecompAndroid-Mali`, branch `benchmark-v2`, clean, HEAD `60813bb` (three local source commits). No benchmark branch push or PR. `.mali` points to the original workspace's local evidence.
- Original checkout remains `grass-masks-ab` at c48606c with its two experimental source edits. Existing `.mali/worktrees/ultrawide/` and `fullscreen-16x10/` with uncommitted work preserved. Fullscreen HUD work was not executed by this task.

## Benchmark v2 — COMPLETE
- User-authorized scope complete: per-frame raw counters for pipeline build/wait, GPU wait, texture upload/BC/ASTC decode and guest file reads; likely causes for periods > budget; window summaries in report.json; thermal_max/thermal_wait_s before each scene/repeat; CPU/GPU headroom; separate game and actual presentation metrics. SavageWindRecomp 257b1f3 MIT attribution + full notice included.
- Local commits: dbb1d3a implementation; f50e05a clocks with flag off reused and missing GPU samples kept unknown. Base pinned 7bb5bc9. Patch `.mali/benchmark/v2/benchmark-v2.patch`.
- Final APK f50e05a, R, three scenes × two repeats: 6/6 ok, each 30.00 FPS, zero game frames >50 ms. Game p99 33.541–33.803 ms; headroom p50 5.988–6.487 ms. Display data separately shows occasional late presentation; zero missing feedback in all six windows. CSV→JSON independent recomputation PASS.
- Cold start (empty separate scratch cache): 1529 completed builds, 5385.880 ms blocking build wait across the trace. Confirmed frame 1430.459 ms with 1379.183 ms pipeline wait; BC-decode stalls also visible. Excluding the first incomplete interval: 14 frames >50 ms attributed to pipeline wait. After warmup, measured window 30 FPS. Raw loading/warmup kept in frames.csv, window=1 selects the measured period.
- Historical single-pair flag off vs stock 0.3.2 (superseded for overhead interpretation by the follow-up above), 100 s each / last900 frames: period p50 33.33/33.33 ms; CPU 19.715/19.185 ms (+2.76%); GPU 27.395/27.350 ms (+0.16%); zero >50ms, zero benchmark logs or report files. Limited regression check, not proof of zero branch cost.
- APK build, Android host tests, counter/classifier/display-ID/missing-data/cooling policy tests PASS. Broken-classifier negative control rejected. Invalid thermal_max rejected as bad_scene_file. Thermal status was 0 at all six scene starts; hot timeout was tested synthetically, not forced on the tablet.
- Causes are a measured-counter heuristic: background compilation is marked correlation, unknown is retained. GPU samples are the existing asynchronous busy estimates; actual presentation uses GOOGLE feedback, never substituted CPU timestamps.
- Full evidence: `.mali/benchmark/v2/REPORT.md`, `summary.json`, `runs/`, build/test logs. First FULL-R on dbb1d3a is supplementary; mandatory FULL2-R + COLD-R ran on final f50e05a, passport clean.

## Tablet and save safety
- Backup `.mali/user-backup/2026-11-08/benchmark-v2/`: 41 files, archive a4eb18df…12fe, all hashes verified. Player save/settings and original active cache unchanged after every run. Only scratch saves/settings/caches were used.
- Exact original installed APK 0.3.1 debug `9e11940d…54fe` restored with adb downgrade without data removal; installed hash verified, all41 save/settings hashes matched backup, app stopped. Scratch cleanup recorded in `.mali/benchmark/v2/cleanup.json`.
- OnePlus's last known separate menulr package and save state were untouched by this task.

## PRs, workflow and next actions
- PR #278 MERGED (2026-11-08 02:42:45Z), merge 681be27. Benchmark code, hooks, scene file and commit stamping identical to benchmark-mvp 08fb8f5; other merge-tree differences are independent upstream changes. PR #255 also MERGED in 0.3.2.
- `mali-workspace` remains documentation-only, separate from contribution branches. This task's source and documentation stay local; no checkpoint push under the explicit no-push instruction.
- No further task or milestone started. Historical 11c/11d still require separate user commands. For previous work and decisions, use the appropriate headings in MALI_PROGRESS.md and MALI_PLAN.md.
