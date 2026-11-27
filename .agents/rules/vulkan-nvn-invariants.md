---
trigger: glob
globs: "**/runtime/nvn/**, **/runtime/hle/**, **/tools/test_*, **/tools/**/test_*"
description: "NVN, Vulkan, Mali vs. Adreno+Turnip invariants and fast unit-first verification loop for runtime and test files."
---

# NVN / Vulkan / Mali & Adreno Technical Invariants

Apply these verified codebase invariants whenever reading, modifying, or reviewing `runtime/nvn/**`, `runtime/hle/**`, or `tools/test_*`:

1. **Linear Render Targets & Guest Spans (`flags & 0x40`):**
   - Tegra linear RTs align row stride to **128 bytes** (`vk_linear_pitch()`), not tight `width * bpp`.
   - Active guest byte span is `(nrows - 1) * pitch + row_bytes` (`vk_tex_guest_span()`); strided padding bytes can contain `NaN`/`Inf` and must **not** be hashed in `tex_hash_full()` / `tex_hash_sample()`.
   - `builder_storage_size()` / `nvnTextureGetStorageSize()` must cover the full `0x40` linear pitch span, and `NvnTexture` caches `pool_size` by value (`texture_from_builder` / `texture_same_desc`) so `nvnMemoryPoolFinalize` never leaves a dangling `t->pool` UAF.
   - `wb_eligible()` must validate `b->format` bytes-per-block (`wb_vk_format_bpb()`) so fallback formats like `RGBA8` cannot overflow `wb_buf`.

2. **NVN Writeback & CPU-Mutation Synchronization:**
   - In `begin_pass()`, even at `lv == 0` (`LOAD_OP_LOAD`), an unowned writeback-eligible RT (`!gpu_owned && !uploaded && wb_eligible`) must trigger initial CPU->GPU upload before `renderer_mark_gpu_owned(t)` so subsequent writeback does not overwrite guest memory with zeros (`#296`/`#438`).
   - When `for_rt_load && b->uploaded && !gpu_owned`, bypass throttling (`checked_frame`, stagger, `texture_unchanged`) via `full_wgen` tracking; if `data_hash` changed, emit a synchronous copy to `s->main` after `vk_end_pass()` bracketed by scoped `VkImageMemoryBarrier2` barriers (pre-copy `ALL_COMMANDS/MEMORY_WRITE -> ALL_TRANSFER/TRANSFER_WRITE|READ`, post-copy `ALL_TRANSFER/TRANSFER_WRITE -> ALL_GRAPHICS|COMPUTE_SHADER|ALL_TRANSFER`).
   - Clear `snap_tex[]`, `snaps[]`, and `inplace_tex[]` on `vk_alias_texture_destroyed()` and preserve `snap_ok` on Turnip `!g_inplace_copy`.

3. **Dual-Target Hardware Invariants (Adreno 830 + Turnip Primary & Mali-G925 Protected):**
   - **Adreno 830 + Mesa Turnip (`driver-id 18`, `Gen8 V39 Patched`):**
     - `VK_EXT_map_memory_placed` (`placed maps yes`) already backs `gpu_heap 3120 MB` on **memory type 1 (`0xf = DEVICE_LOCAL | HOST_VISIBLE | HOST_COHERENT | HOST_CACHED`)**, whereas `transient ring` uses **memory type 0 (`0x7` uncached write-combined)**. Keep CPU reads away from `transient ring` (`type 0`).
     - Turnip `tu_autotune` selectively uses GMEM on heavy passes and falls back to `sysmem` on fragmented passes (`B1` default `14.50 ms` vs `TU_DEBUG=sysmem` `16.21 ms` on `plateau`). **Never force `TU_DEBUG=gmem`** while the renderer emits `140–214` `LOAD_OP_LOAD + STORE_OP_STORE` passes/frame (`TU_DEBUG=gmem` regressed `plateau` to `33.64 ms` and `zora-plaza` to `44.03 ms` at `99% @ 1100 MHz`). Unlock GMEM by reducing `in-place` breaks (`VK_EXT_attachment_feedback_loop_layout`) and adding `STORE_OP_DONT_CARE`/`NONE` + deferred clear coalescing.
     - `TU_DEBUG=perf` logs `LRZ` invalidations under logcat tag `TU` (`Depth write + ALWAYS/NOT_EQUAL`, `FS writes depth or side-effects`, `Depth write + blending`). Preserve LRZ eligibility on opaque geometry/shadow passes.
     - `IR3_SHADER_DEBUG=nouboopt` stays gated strictly to Adreno `840..899` (`device.c`); keep a regression check ensuring `nouboopt` remains OFF on Adreno 830 (`a830` renders shadows cleanly and saves `+1.53 ms` / `-10.6%` GPU time and `~1.05 W` vs `3.7–10.1x` `cat6` bloat). Do not build custom Mesa Turnip drivers.
     - Target **Native `1080p` (`100%` `1:1` bypass with zero upscaler/downsampler overhead)** plus sub-native `16px`-aligned scales (`65%/68%/70%`, optional `85%` `1632x912` `SGSR1`); drop `>100%` SSAA/bicubic passes. Always scale ADPF target duration dynamically (`~21.5–22.5 ms` at `40 FPS`, `~29.5–30.5 ms` at `30 FPS`, `~15.5–16.67 ms` at `60 FPS`) across both game and render threads.
     - `VK_EXT_graphics_pipeline_library` is exposed (`api 1.4.367`); Oryon `6+2` has shared L2 per cluster (`0-5` perf, `6-7` prime) and **zero L3 cache**, so keep Guest Core 0 + Render/Present on `cpu6..7`, Guest Cores 1..2 on `cpu2..5`, workers on `cpu0..1`, and cache-line-align (`alignas(64)`) shared ringbuffer indices while keeping Mali `4+3+1` affinity unchanged.
     - Locked Stage A+B Odin 3 `Native` baseline (`40.00 FPS`, `1920x1080`, `6.19–8.82 W`): `Oryon mode=3`, dynamic `ADPF`, `Native` CPU execution (`native_code=1`), `WC (0x7)` upload ring, `S3.1+S3.2` (`SPIRV_CLEANUP` + `DISCARD_OPT` + `noop_blend_opt` + `RP_MERGE_OPT` + `STORE_OP_NONE`), and `LARECOMP_TURNIP_FB_LOCAL_BARRIER=0` (feedback barrier stays OFF by default). At Final Sign-Off on Odin 3, install the winning signed APK directly (`adb install -r`) and auto-configure live `v190/botwrecompandroid.ini` + `SM8750` C/Java defaults instead of reverting to stock `v0.5.0-beta`.
   - **Mali-G925 / Immortalis (Protected Non-Regression):**
     - `VK_FORMAT_R16G16B16A16_UNORM` (`vkfmt=91`, NVN `0x2a`) lacks `SAMPLED_IMAGE_FILTER_LINEAR` (`lin_ok=0`). Preserve `soft_linear` emulation and `#307` `vk_sampler_nearest` interactions (`LARECOMP_NO_FILTER_FALLBACK=1`, `ShaderKey.soft_linear`).
     - `BC 0 ASTC 1` (CPU-decoded BC + `BC3`/`BC5_UNORM -> ASTC 4x4`), `placed maps no` (keep `LARECOMP_PIPE_KEEP=1500` / `LARECOMP_PROG_KEEP=1000` LRU eviction), classic `VkRenderPass` default (`r49+`), `vertexPipelineStoresAndAtomics=0`, and `HOT_DEMOTE_FRAMES=0`.

4. **Lifecycle, Reset, and Flag-Off Hygiene:**
   - Any per-boot runtime state must be cleaned via `RT_ON_RESET` (`runtime/hle.h`) and safe against async compiler/translator workers.
   - Diagnostic or benchmark hooks (`bench_on()`, `LARECOMP_*`) must use lazy checks with zero clock reads or atomic writes when disabled.

5. **Fast Inner-Loop Verification:**
   - Run fast host Python suites (`python -B tools/test_*.py`, `<2s`) and standalone NDK/WSL2/Bionic C unit binaries (`tools/test_*.c`, `tools/test_android_runtime.ps1`, `~3s`) before spending `20-40s` on full `gradlew.bat assembleDebug` + physical-device APK runs.
