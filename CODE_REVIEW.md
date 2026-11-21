# Adversarial Pre-PR Code Review — Workstream 2A (`feat/vk-bc-parallel-decode`)

- **Branch / Commit:** `feat/vk-bc-parallel-decode` @ `122e11e9fdb5af850fe0b59fce3915bec17acdd7`
- **Base Commit:** `1c02adf994194055f7d517fc99dc5686269e11ed` (`upstream/v0.4.10-beta`)
- **Files Changed (8):** `runtime/nvn/vulkan/bc_parallel.h`, `runtime/nvn/vulkan/textures.c`, `runtime/nvn/vulkan/watchdog.h`, `runtime/nvn/vulkan/watchdog.c`, `runtime/platform/android.c`, `tools/test_bc_parallel.c`, `tools/test_bc_shadow.c`, `tools/test_android_runtime.ps1`
- **Reviewers:** Two independent read-only `independent-verifier` subagents running in parallel:
  1. `7ea74a6c` — **Concurrency, Memory Ordering, Watchdog & Lifetimes**
  2. `84c372f5` — **Hardware Divergence, Scheduling/Affinity, Desktop Portability & Upstream PR Standards**

---

## Findings & Resolutions

All `CRITICAL`, `MAJOR`, and valid `MINOR` defects identified across both reviews were fixed in commit `122e11e9fdb5af850fe0b59fce3915bec17acdd7`.

| Severity | Finding Summary | Location(s) | Resolution |
| --- | --- | --- | --- |
| **CRITICAL** | `vk_bc_pool_destroy()` left `p->gen`, `p->fn`, `p->ctx`, `p->threads`, `p->remaining` dirty while workers initialized `seen = 0`, risking UAF / stale execution / premature stack return if a pool is reused after destroy. | `runtime/nvn/vulkan/bc_parallel.h` | Zero full `VkBcPool` via `*p = (VkBcPool){0};` at end of `vk_bc_pool_destroy()`; store `uint64_t start_gen` in `VkBcWorkerArg` so workers start at `seen = wa->start_gen`; added 10-iteration destroy/reuse stress check in `tools/test_bc_parallel.c`. |
| **MAJOR** | Lazy worker creation inside `render_main` inherited `plat_render_thread_pin()`'s narrowed 1- or 2-core mask on low-RAM / `RAM saving` / `LARECOMP_RENDER_BIG=1` configs, serializing all 4 decode threads onto 1–2 cores. | `runtime/platform/android.c`, `runtime/nvn/vulkan/bc_parallel.h` | Converted `g_pinned` to `_Atomic bool` (`acquire`/`release`), saved pre-pin `g_all_cores` inside `plat_render_thread_pin()` before `sched_setaffinity()`, and invoked weak `plat_thread_unpin()` at the start of `vk_bc_worker_setup()` prior to selecting fast CPUs. |
| **MAJOR** | `tools/test_bc_shadow.c` aliased `staging` for both `vk_transient()` and `vk_readback()`, creating a concurrent read/write data race on the `2048x16` shadow resolve path under 4T decode. | `tools/test_bc_shadow.c` | Allocated dedicated `readback_buf[1 << 20]` for `vk_readback()`; merged adjacent `#ifdef __ANDROID__` blocks. |
| **MAJOR** | `vk_bc_select_fast_cpus()` narrowed affinity whenever `n_fast >= 3`, oversubscribing fast cores when user explicitly set `LARECOMP_BC_THREADS > n_fast` (e.g. `6` or `8` on a `1+3+4` SoC). | `runtime/nvn/vulkan/bc_parallel.h`, `tools/test_bc_parallel.c` | Passed `threads` into `vk_bc_worker_setup(int)` and `vk_bc_select_fast_cpus(..., int threads)` requiring `n_fast >= 3 && n_fast >= threads`; expanded affinity unit coverage. |
| **MAJOR / MINOR** | Watchdog marker reused `WD_R_BUILD_WAIT` ("waiting for a shader or pipeline build"), mislabeling BC decode stalls during freeze triage. | `runtime/nvn/vulkan/watchdog.h`, `watchdog.c`, `textures.c` | Added dedicated `WD_R_BC_DECODE` (`"waiting for parallel BC texture decode"`) and applied it at both `textures.c` split call sites. |
| **MINOR** | `LARECOMP_NO_BC_PARALLEL=0` (or empty) disabled parallel decode (`!= NULL`); invalid `LARECOMP_BC_THREADS` values logged `(LARECOMP_BC_THREADS)` instead of `(driver default)`; `"1 threads"` pluralization. | `runtime/nvn/vulkan/textures.c`, `bc_parallel.h`, `tools/test_bc_parallel.c` | Switched to `atoi(no_e) != 0` matching `device.c:env_on()`; added `bool *from_env` out-param to `vk_bc_parallel_threads()`; logged `thread`/`threads` cleanly when `cpu_bc \|\| n > 1 \|\| no_env`. |
| **MINOR** | Raw `sched_getaffinity` buffer `unsigned long[2]` could `-EINVAL` on `>128`-CPU kernels; `bc_decode_rows` narrowed `wb * 4 * bpp` to `int` before `uint64_t` offset math; unchecked `pthread_{mutex,cond}_init` and unbounded `pthread_create` retries; `<sched.h>` / `sched_getcpu()` unguarded on non-Linux hosts. | `bc_parallel.h`, `textures.c`, `tools/test_bc_parallel.c` | Widened affinity syscall buffers to `unsigned long[16]` (1024 CPUs on 64-bit); guarded `pitch64 <= INT_MAX` and used `pitch64` in offset math; added `init_failed` & `spawn_failed` fallback flags; guarded `<sched.h>` / `sched_getcpu()` under `#if defined(__ANDROID__) \|\| defined(__linux__)` with 64-CPU histogram. |

---

## Verification After Fixes (Exact Commit `122e11e9`)

- **Cleanliness (`workspace_check.py pr`):**
  `worktree=bc-parallel-decode base=1c02adf9 commits=1 files=8 mergeable=YES -> PASS: PR branch clean and mergeable`
- **Multi-target NDK Clang `-fsyntax-only -Wall -Wextra` (`tools/test_bc_parallel.c`):**
  0 warnings in `bc_parallel.h` and `test_bc_parallel.c` across `--target=aarch64-linux-android30`, `--target=armv7a-linux-androideabi30`, `-U__ANDROID__`, and `-U__ANDROID__ -U__linux__`.
- **On-device Bionic Regression Suite (`tools/test_android_runtime.ps1` on `SM-X730` `R52YA06LXAT`):**
  **9/9 tests PASSED** (`test_kernel_affinity`, `test_bc_shadow`, `test_buffer_copy`, `test_rt_load_upload`, `test_cbwin_snap`, `test_nvn_lifetime`, `test_object_cache`, `test_benchmark_trace`, `test_bc_parallel`), 0 warnings in touched files:
  - `BC7 512x512 (128x128 blocks)`: `1T 1.22 ms vs 4T 0.97 ms (1.26x)`
  - `BC7 1024x1024 (256x256 blocks)`: `1T 5.29 ms vs 4T 3.73 ms (1.42x)`
  - `BC1->ASTC 1024x1024 (256x256 blocks)`: `1T 18.68 ms vs 4T 6.04 ms (3.09x)`
- **Signed Release APK (`:app:assembleRelease`):**
  - Clean-HEAD (`122e11e9`) build succeeded in `44s` with `0` warnings in touched files (`:app:clean :app:assembleRelease`).
  - APK SHA-256: `475714955e2636174e4fd2fcef4bbd0e334abab7f7027a317f20fcc6afb3c84c`
  - Signer Cert SHA-256: `60fa2a5f462fd95d30b37dde54d769de8986d7c8377e13c3ea5773088f20527a`

**Verdict:** `0` unresolved `CRITICAL`, `MAJOR`, or `MINOR` findings.
