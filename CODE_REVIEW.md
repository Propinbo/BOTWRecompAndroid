# Code Review Record: PR 2B-1 (`feat/vk-mali-renderpass-default`)

- **Branch / Commit:** `feat/vk-mali-renderpass-default` @ `9d5e732c15da16a32a3f99797682ab26c5ccd4e4`
- **Base:** `mamcjr/BOTWRecompAndroid` `main` (`1c02adf994194055f7d517fc99dc5686269e11ed`, `v0.4.10-beta`)
- **Reviewers:** Two independent parallel read-only `independent-verifier` subagents:
  - `c4f84ad0-36ff-4e29-8ee3-c56cc4b7e001` — Vulkan Spec & `rpcompat.c` Lifetime / Concurrency / Reboot Safety
  - `631cd4e7-921e-40ee-9f4b-92f325206c71` — Mali `r49+` Gating, Freestanding `wasm32` / Desktop Parity & Upstream PR Standards

---

## Initial Review Pass (`945f8aa`) & Resolutions

1. **`[MAJOR]` (`device.c` `< 1.3` `dr_ext_ok` / `q_dr` query skipped; flagged by both reviewers):**
   - **Fix:** Preserved `bool dr_ext_ok = cg[OPT_DR];` before filtering `cg[OPT_DR]` below 1.3; used `dr_ext_ok` in `QCHAIN(dr_ext_ok, q_dr)` and `h13.dynamicRendering = dr_ext_ok && q_dr.dynamicRendering` so physical-device capability and log reason (`"Mali r49+ default"` / `"LARECOMP_NO_DYNAMIC_RENDERING=1"`) remain exact while still omitting `OPT_DR` extensions / `VK_KHR_maintenance5` / `e_dr`.
2. **`[MAJOR]` (`rpcompat.c` implicit `RpKey` 2-byte tail padding & struct assignment; flagged by `c4f84ad0`):**
   - **Fix:** Added explicit `uint8_t pad[2];` + `_Static_assert(offsetof(RpKey, pad) + sizeof(((RpKey *)0)->pad) == sizeof(RpKey))` and switched insertion to `memcpy(&rps[free_slot].k, k, sizeof *k)`.
3. **`[MAJOR]` (`rpcompat.c` `RP_MAX` 512 hard cap leak & framebuffer-miss churn; flagged by `c4f84ad0`):**
   - **Fix:** Replaced fixed 512-cap table with dynamic power-of-two `rp_grow()` (`RP_INIT_CAP = 1024`, rehashing 2x at >50% load) with overflow and OOM safety (`(uint32_t)nrps + 1u < rp_cap`).
4. **`[MINOR]` (`device_compat.h` whitespace-only `LARECOMP_NO_DYNAMIC_RENDERING` handling; flagged by `631cd4e7`):**
   - **Fix:** Added `vk_compat_skip_ws()` before checking `*no_dr_env`, so blank whitespace falls back to driver default.
5. **`[MINOR]` (`rpcompat.c` unlocked `nlate` read in `vk_rp_collect()`; flagged by `c4f84ad0`):**
   - **Fix:** Made `nlate` `_Atomic int` (`memory_order_release` on store under `lock`, `memory_order_acquire` on lockless fast-path read).
6. **`[MINOR]` & `[NIT]` (Test coverage & headers; flagged by both):**
   - **Fix:** Expanded `tools/test_mali_rp_default.c` with `v1.r48p1`, `VK_DRIVER_ID_MESA_RADV`, `"   \t "`, `"-0"`, `"+000"`, `"-1"`, `" 1a"`, `"99999999999999999999"`, full `*reason` checks, and Android Bionic `test_rpcompat_cache()` (`700` unique keys triggering `1024 -> 2048` rehash, `700` cache hits, and `make_rp()` OOM recovery). Added `<stdint.h>` + `#if !defined(VK_VERSION_1_0)` guard in `device_compat.h` and updated `rpcompat.c` top comment.
7. **Note on `exec.c` pipeline `VK_NULL_HANDLE` OOM pre-existing path & commit scope:** Keeping `exec.c` untouched maintains strict single-concern PR scope, and dynamic `rp_grow()` eliminates cap-triggered `VK_NULL_HANDLE` returns. Commit `9d5e732` documents the paired `rpcompat.c` O(1) lookup optimization necessitated by defaulting Mali `r49+` onto `rpcompat.c`.

---

## Final Re-Verification Pass (`9d5e732`)

- **`c4f84ad0` Verdict:** **`APPROVE`** — `0 CRITICAL`, `0 MAJOR`, `0 MINOR`, 1 harmless `[NIT]` (`device.c:720` logging `(default)` on whitespace-only env on non-Mali).
- **`631cd4e7` Verdict:** **`APPROVE`** — `0 CRITICAL`, `0 MAJOR`, `0 MINOR`, 1 identical harmless `[NIT]`.

---

## Automated Verification on Exact Final Commit (`9d5e732`)

- `python -B -S .agents/scripts/workspace_check.py pr .mali/worktrees/mali-renderpass-default --base 1c02adf994194055f7d517fc99dc5686269e11ed`: `PASS: PR branch clean and mergeable`.
- `tools/test_vulkan_compat.py` (`wasm32 -ffreestanding -O2 -nostdlib` + Node): `16 Vulkan compatibility checks passed (production C)`.
- `tools/test_vulkan_startup.py` (`wasm32 -ffreestanding -O2 -nostdlib` + Node): `33 Vulkan startup checks passed (production C; no device)`.
- `tools/test_pipeline_cache_save.py`: exit `0`.
- Multi-target NDK Clang `-fsyntax-only -Wall -Wextra` (`aarch64-linux-android30`, `armv7a-linux-androideabi30`, `-U__ANDROID__`, `-U__ANDROID__ -U__linux__`) on `rpcompat.c`, `device.c`, `test_mali_rp_default.c`, `test_dyn_vertex_input.c`: `0` warnings in touched files, `0` errors.
- `tools/test_android_runtime.ps1 -DeviceSerial R52YA06LXAT`: `9/9` Bionic unit tests passed on physical ARM64 Android (`SM-X730`).
