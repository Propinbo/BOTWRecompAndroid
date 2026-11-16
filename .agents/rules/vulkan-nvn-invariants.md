---
trigger: glob
globs: "**/runtime/nvn/**, **/runtime/hle/**, **/tools/test_*, **/tools/**/test_*"
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

3. **Mali vs. Adreno+Turnip Hardware Divergences:**
   - **Mali-G925 / Immortalis:** `VK_FORMAT_R16G16B16A16_UNORM` (`vkfmt=91`, NVN `0x2a`) lacks `SAMPLED_IMAGE_FILTER_LINEAR` (`lin_ok=0`). Preserve `soft_linear` emulation and `#307` `vk_sampler_nearest` interactions (`LARECOMP_NO_FILTER_FALLBACK=1`, `ShaderKey.soft_linear`).
   - **Mali Memory/Formats:** `BC 0 ASTC 1` (CPU-decoded BC textures), `placed maps no` (no `VK_EXT_map_memory_placed` -> CPU guest heap + GPU `/dev/mali0` `GL mtrack` coexist inside `VmRSS`), and Mali `libGLES_mali.so` retains `scudo` pipeline allocations after `vkDestroyPipeline`.
   - **Adreno 830 + Turnip:** `placed maps yes`, `BC 1`, `!g_inplace_copy` paths; always verify `driver-id 18` / Mesa Turnip in `[vk]` logs.

4. **Lifecycle, Reset, and Flag-Off Hygiene:**
   - Any per-boot runtime state must be cleaned via `RT_ON_RESET` (`runtime/hle.h`) and safe against async compiler/translator workers.
   - Diagnostic or benchmark hooks (`bench_on()`, `LARECOMP_*`) must use lazy checks with zero clock reads or atomic writes when disabled.
