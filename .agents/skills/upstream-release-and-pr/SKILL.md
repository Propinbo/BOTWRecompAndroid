---
name: upstream-release-and-pr
description: >-
  Guides upstream GitHub Release checks, campaign SHA pinning, isolated worktree setup with Windows NTFS junctions, pre-PR cleanliness audits, and local PR packaging for mamcjr/BOTWRecompAndroid. Use when checking releases, starting a feature worktree, rebasing, or preparing an upstream PR.
---

# Upstream Release Check and PR Workflow

## 1. Rolling Upstream Release Check

1. Check releases when starting upstream-bound source work, evaluating/syncing baselines, or rebasing/packaging a PR (skip on unrelated local tasks or mid-campaign turns pinned to a chosen SHA):
   - `gh release list -R mamcjr/BOTWRecompAndroid -L 5`
   - `git ls-remote upstream refs/heads/main refs/tags/<newest_tag> refs/tags/<newest_tag>^{}`
2. Inspect release notes (`gh release view <tag> -R mamcjr/BOTWRecompAndroid`) and open/merged PRs (`gh pr list -R mamcjr/BOTWRecompAndroid --author Propinbo`) to check for overlapping upstream fixes or regressions.
3. Pin an exact commit SHA at the start of any bounded experiment or PR branch; never switch baselines mid-experiment.
4. Record any checked release in `MALI_STATUS.md` (and `MALI_PLAN.md` if baseline/active targets change). Keep active PR branches pinned to their chosen SHA until rebasing; fast-forward fork/local `main` to verified upstream releases when syncing baselines.

## 2. Isolated Feature Worktrees (Windows NTFS Junctions)

1. Never edit source directly on root `main` when preparing a feature or PR; create an isolated worktree under `.mali/worktrees/<slug>` and initialize nested submodules (`third_party/libadrenotools/lib/linkernsbypass`) before Android builds:
   ```powershell
   git worktree add -b <branch> .mali/worktrees/<slug> <pinned_sha>
   cmd /c mklink /J "<abs_worktree>\android\deps\out" "<abs_repo_root>\android\deps\out"
   git -C .mali/worktrees/<slug> submodule update --init --recursive
   ```
   The Android Gradle wrapper lives at `<abs_worktree>\android\botwrecompandroid\gradlew.bat` (use JDK 17).
2. Keep strictly one editing/building agent per worktree.
3. To remove a finished/superseded worktree safely on Windows:
   - First unlink the junction (`cmd /c rmdir "<abs_worktree>\android\deps\out"` — NEVER `Remove-Item -Recurse` on a junction!).
   - Verify `root\android\deps\out\lib\libshaderc_shared.so` still exists and zero `ReparsePoint`s remain inside `<abs_worktree>`.
   - Then delete the directory (`cmd /c rd /s /q "\\?\<abs_worktree>"`) and run `git worktree prune`.

## 3. Mandatory Pre-PR Multi-Agent Code Review Gate

1. Before running `gh pr create` or pushing substantive code/rebase changes to an open upstream PR, dispatch **2–3 parallel `invoke_subagent(TypeName="independent-verifier", ...)` calls** (strictly read-only) covering `<base>...HEAD` across orthogonal lenses:
   - **Lens 1 (`Concurrency, Lifetimes & Reset Safety`):** Threading, lock hierarchy, C11 atomics, worker drain vs. `vk_exec_reset` / `plat_reset_guest` / `map_placed`, `RT_ON_RESET`, and teardown cleanup.
   - **Lens 2 (`Vulkan Spec, Cache Integrity & Mali/Turnip Divergence`):** VUIDs, barriers, dynamic state (`ss_forget`/`SS_VI`), cache keys/corruption guards, Mali (`BC 0`, fmt `91`, LRU eviction) vs. Adreno+Turnip (`driver-id 18`, `placed maps`, `!g_inplace_copy`).
   - **Lens 3 (`Upstream Style, Benchmark/Flag-Off Overhead & Test Coverage`):** `CONTRIBUTING.md` comment style (*why*, not *what*), `#315` metrics / zero flag-off cost, `#ifdef __ANDROID__` portability, unit/negative-control coverage, and hygiene.
2. For wide multi-module PRs where per-file fan-out is cleaner, `run_workflow` (`type_name="self"` with explicit read-only instructions and structured JSON output schema) may supplement `independent-verifier`.
3. Resolve every `CRITICAL` and `MAJOR` finding (plus valid `MINOR` defects), re-verify tests on the final diff, and archive the review record in `.mali/<topic>/upstream-pr/CODE_REVIEW.md`.

## 4. Pre-PR Audit and Local Packaging (`CONTRIBUTING.md`)

1. Run [workspace_check.py](../../scripts/workspace_check.py) (`python -B -S .agents/scripts/workspace_check.py pr <worktree> --base upstream/main`) for instant validation of working-tree cleanliness, `git merge-tree --write-tree`, forbidden paths (`Dump/`, `extracted/`, `generated*`, `.bfttf`, `.mali/`, `.agents/`, `AGENTS.md`, `MALI_*.md`), protected files (`tools/patches.py`, `GAMELIB_ID`, `.gitignore`), and English-only commits/diffs.
2. Audit `git diff --stat <base>..HEAD` and `git log -n 5` to confirm one focused change per PR (`CONTRIBUTING.md`).
3. Export a clean `.patch` file and draft English `PR_DRAFT.md`, `CODE_REVIEW.md`, + `REPORT.md` locally under `.mali/<topic>/upstream-pr/`.
4. Run relevant host/Bionic/device tests after final post-review edits, then push the branch to `origin` and create or update the PR via `gh pr`.

## 5. Upstream Maintainer (`mamcjr`) Review Patterns & Required PR Evidence

Distilled from all `Propinbo` PRs (`#183`–`#508`) and peer PR reviews (`#189`, `#251`, `#417`, `#452`–`#458`, `#466`–`#468`) on `mamcjr/BOTWRecompAndroid`:

### A. Recurring Code-Level Blockers & Nits
1. **Zero cost / zero behavior change when off or on other GPUs (`#315`, `#436`, `#468`, `#475`):** Hot paths when a flag is off or on GPUs that do not need a fallback (especially Adreno 830 + Turnip, which `mamcjr` tests on POCO F7 Ultra) must pay at most a single cheap flag check — no clock reads, no atomics, no per-draw `vkGetPhysicalDeviceFormatProperties`, no unguarded `vk_exec_reset` side effects (e.g., `g_scene_frame`), and no tracking overhead in default mode.
2. **Conservative driver gating + env rollback (`#183`, `#483`, `#485`, `#468`):** Never turn on a risky Vulkan extension or behavior change across all drivers just because an extension is advertised (Qualcomm stock and older Mali `< r49` often have driver bugs). Enable defaults strictly on tested drivers (e.g., Mali `r49+`, Turnip), keep an explicit `LARECOMP_*` env switch to force on/off or roll back, and verify vendor-ID checks do not misclassify PanVK vs. proprietary Mali vs. Xclipse (`#466`).
3. **Strict single-concern scope & safe failure modes (`#461`, `#436`, `#417`, `#189`):** Split unrelated hardening/table/layout tweaks into separate PRs with their own evidence (`#461`). Avoid silent skips that can turn textures black or drop draws permanently: fall back to the base program on variant compile failure (`#436`), and prefer logging/clamping over silent discard. Do not duplicate shared helpers across parallel PRs — stack cleanly on the base PR (`#481` -> `#482`).
4. **Render-thread watchdog, non-blocking waits, and existing globals (`#481`, `#475`, `#452`, `#315`):** Wrap every new render-thread wait in a freeze-watchdog stage (`runtime/nvn/vulkan/watchdog.h`, e.g., `WD_R_BUILD_WAIT` for `#401` diagnostics). Never put `usleep`/blocking spinloops on the render thread (`#475`). Reuse existing globals like `g_exec_s` (`#452`) and keep `main`'s newer branches/metrics intact during rebases (`seq_fences_on` GPU waits in `#315`).
5. **Cache invalidation & struct-key completeness (`#482`):** Every persistent disk cache key must mix a translator/build ID so app updates cleanly invalidate stale `.spv`/metadata, and hash full zero-padded structs (not field-by-field lists that silently miss future fields).
6. **In-place reboot, reset & lifetime hygiene (`#461`, `#467`, `#481`, `#485`):** All per-boot/per-session state must reset on return-to-title / in-place reboot (`0` in-flight jobs, `0` leaked programs/pipelines, `tex_discarded_bytes` accounting kept in sync in `destroy_image`).
7. **Desktop `#ifdef __ANDROID__` parity & 11-locale UI rules (`#457`, `#475`, `#467`):** Never put shared `g_settings` defaults only inside `#ifdef __ANDROID__`. Any new/edited UI string must be updated in **all 11** `runtime/ui/i18n_*.inc` files, and user-facing help text must **never** include issue numbers (e.g., `(#294)`). Verify compilation on both Android and non-Android targets.
8. **Style, comments & provenance (`#315`, `#454`, `#468`):** Place `#include` directives **below** each file's top header comment (`#315`). Update any adjacent comments invalidated by the change (e.g., `"Low RAM only for now"` in `#454`). Document exact upstream source/commit SHA for any third-party license/file (`#315`).

### B. Required Evidence & Attachments in Every Upstream PR
Always match evidence to the **exact final PR commit** (never cite numbers solely from a combined prototype or older release build; `#461`, `#481`):

| PR Type | Must Attach / Include in PR Description |
|---|---|
| **All PRs** | Device model, Android OS, BOTW version (`1.9.0`), GPU + exact driver string (`driver-id`, `info`); exact startup/runtime log lines (`[vk]`, `[vk-cap]`, `[widescreen]`, `[cbuf]`) proving the new code path **actually executed** (`#210`, `#251`, `#466`); explicit **tested vs. untested GPU matrix** (state clearly if Turnip/Qualcomm was not tested); flag-off/non-target zero-cost argument + rollback env var (`LARECOMP_*`); rebase/merge order if stacked on another open PR. |
| **Visual / Rendering Bugfix** (`#210`, `#436`, `#461`, `#457`, `#504`) | **Before/after screenshots** in the affected scene + sanity check in open world / shrine / Plateau grass; if symptoms overlap another open PR, re-verify **on top of that PR** (`#436` atop `#461`) on the exact final commit. |
| **Performance / Vulkan Optimization** (`#183`, `#481`, `#483`, `#466`, `#506`–`#508`) | **Isolated A/B of this PR alone** (flag `0` vs `1` or base vs PR): report **`1% low` / `frame p99` / `gpu p50` / `render_thread_ms`** (not just average FPS); note negative/neutral results honestly (`#458`); include `LARECOMP_LOG_POWER=1` (off-charger) when changing default GPU/wait behavior if measurable (`#466`, `#454`). |
| **Cache / Shader / Compiler** (`#482`, `#475`) | **3-stage test:** (1) cold cache build (verify no worker starvation/regression), (2) warm cache hit rate (`hits / misses`), (3) **upgrade over an older cache** proving clean invalidation/rebuild. |
| **Default-On / Lifetime / Memory / Slot** (`#485`, `#454`, `#481`) | **Stress triad on physical Mali:** (1) high-churn resource loop (e.g., 50+ inventory open/tab/close cycles), (2) multiple **in-place reboots** (`0` still building, `0` leaks), (3) **30+ min continuous session** showing flat `VmRSS` and `0` errors. |
| **UI / Settings** (`#475`, `#457`, `#507`) | Confirmation that all **11 `i18n_*.inc`** locales are updated, zero `#NNN` refs in UI strings, and desktop/Android `g_settings` defaults match. |
| **Benchmark / Tooling** (`#315`, `#480`) | Proof of zero overhead with benchmark off (`#315`) + pass results on all existing starter scenes in `tools/benchmark-scenes.txt` (`#480`). |
