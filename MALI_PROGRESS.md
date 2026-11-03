# Mali contribution execution checkpoint

Checkpoint ID: MALI-M10-COMPLETE-2026-11-02 (milestone 10 complete, H-A supported; preceding checkpoint MALI-M8-COMPLETE-2026-11-02 (milestone 8 complete with failed P2; preceding checkpoint MALI-M7-COMPLETE-2026-11-01 (milestone 7 complete; preceding documentation checkpoints: 1588b1a0fd290234420120c76287ce4b80dd424a (milestone 6), baaedbf2d86796cfcba2e1a0852b4f003c5deb87 (milestone 5b), 51cebfc41500128d35afba5569287b45dd77fbff (milestone 5b added), 48ba5aa6a6b801da5937293c0aa761b36356bf87 (milestone 5), 911e7644a9d4ce69aabf14da4f84386ee68ed017 (0.2.10 migration), 6a8678f9751fa7870d715743c61eef347a9538c2 (0.2.8 migration), b37b036a6f7fbb9c5a4d1f878628eeb1bf98a5a2 (M4 complete), d843d612733c19e9a900c0a2ab9cafb5919fa36b, b9dda86b9bb13d14abaaba9aa440c0a58142d262))).

Updated: 2026-11-02, Europe/Moscow (UTC+03:00).

## Teleport dissolve A/B on 0.2.10, 2026-11-02 (user: "Да, запускай"; not a milestone)

- **User report:** at fast-travel departure Link turns dark/black and vanishes abruptly instead of gradually glowing and breaking into lines. Seen on the user's own save, 0.2.10, Recompiled (`native_code = 0`), every teleport. The user supplied a Switch reference video (`.mali/teleport-dissolve/reference/ref.mp4`, local only).
- **Runs** (SM-X730 over USB, installed 0.2.10 `versionCode 30`, Recompiled, harness settings, Great-Plateau seed save copied into a fresh scratch dir, same calibrated teleport as TP210, `screenrecord` 1280x800). Four runs: TPD-R-DEF (default), TPD-R-NOVS (`LARECOMP_NO_VS_SNAP=1`), TPD-R-NOGS (`LARECOMP_NO_GATHER_SNAP=1`), TPD-R-BC1 (`LARECOMP_BC1_ASTC=0`). All four teleports were performed (confirm→HUD 23.6-25.2 s).
- **Result: the defect reproduces identically in all four runs**, so `9c3a0f3`/`1ff912a` (UV/gather snapping) and `596615e` (BC1→ASTC) are excluded. Departure on Mali: the normal Link vanishes within one frame, then for ~0.3 s an opaque, unlit, saturated blue/pink Link builds up piece by piece (head, sword, then body), then vanishes, and the line column starts. Switch reference: over ~1.5-2 s the lit Link gradually shifts to a translucent glowing light-blue Link, and the lines emerge from it while the body is still visible. Arrival on Mali looks close to the reference (translucent blue Link fading in).
- **Interpretation (not confirmed):** the glow/dissolve material's draw is wrong (opaque instead of blended, or an inverted or binary mask), and the base model's fade-out is not gradual. Not yet tied to a shader or blend state. Static notes: NVN fixed-function alpha test (`nvnColorStateSetAlphaTest`, `nvnCommandBufferSetAlphaRef`) is recorded but never applied in the Vulkan backend; whether BOTW uses it here is unknown.
- **Follow-up (user: "Снять шейдер"):** before dumping shaders, the app logs of all four runs showed draws skipped while shaders build exactly in the departure window (default: 742 + 1,615 skipped draws, 139 + 131 compiles at t=65-69 s; same pattern in every run). Run **TPD-R-SYNC** (`LARECOMP_SYNC_SHADERS=1`, otherwise identical): 0 skipped draws, **Link no longer vanishes or assembles piecewise**; he shifts gradually from lit to blue over ~0.6 s as in the reference. Frame cost is similar (max 156/167 ms vs 171/153 ms default). **Cause of the abrupt disappearance: asynchronous pipeline builds** - the glow material's pipelines (one per Link part) are new at departure, so their draws are skipped until each finishes. Not Mali-specific in mechanism (async compile is the backend's default everywhere); Mali's slower compiles make it longer. Not yet checked: whether a second teleport in the same session is clean (expected if this is the cause).
- **Second teleport in one session (user: "Проверить 2-й телепорт"):** the Great-Plateau seed has only one travel point (TPD-R-TWICE: first teleport done, second not possible; map label never matched). Repeated on a fresh scratch copy of `mali-step4-hateno-seed` (TPD-R-HAT2, Recompiled, default flags, map driven manually): teleport 1 Hateno lab → Myahm Agana Shrine, teleport 2 Myahm Agana → Myahm Agana. **Teleport 1: Link vanishes for 2 frames, then a purple/blue Link appears at once. Teleport 2: no vanish, a gradual lit→blue transition, as in TPD-R-SYNC.** This confirms the async-compile cause: only the first departure in a session shows the abrupt disappearance (fewer missing frames than on the plateau, probably because the driver pipeline cache was warm). Both teleports, like TPD-R-SYNC, still show the saturated blue/purple colour and ~7 frames without Link before the lines appear; the second teleport has its pipelines already built, so this part is not async compile.
- **Glow-colour shader capture (user: "Снять шейдер цвета"; Hateno seed, Recompiled):** TPD-R-PICK1 (debug frames by F9 = `input keyevent 139`, `LARECOMP_DEBUG_NO_PASSES=1`, `LARECOMP_LOG_FSHASH=1`, points in `build/pick.txt`) and TPD-R-DUMP (plus `LARECOMP_SYNC_SHADERS=1`, `LARECOMP_VK_NO_SPIRV_CACHE=1`, `LARECOMP_DUMP_FS=...`). Findings:
  - During departure Link is drawn into the G-buffer with dedicated materials that are not used in normal frames (fs `d5dfd5a97f910f0e`, `17890d52aae48359`; their GLSL was dumped only once the teleport started). In `d5dfd5a97f910f0e`, RT1 (albedo) is scaled by `1 - cb7[34].w` and RT5 (the HDR target, emission) is lerped to a constant glow colour `cb7[34].rgb` by `cb7[34].w`. A dissolve edge adds emission `(0.2, 0.45, 4 + 10·noise)·edge`, with a `discard` from a 3D noise texture while `cb7[37].w < 1`.
  - Measured RT5 on Link pixels (RT5 pick, second teleport): `(0.033, 0.075, 0.67)` then `(0.050, 0.113, 1.0)`, identical across body points, i.e. exactly the glow constant times the ramp. **The translated material outputs the deep blue that its constants ask for**, so the pale cyan of the reference must come later (lighting composite `ec9c39a52f0aac1e`, bloom or tonemap) or from different constant values; not determined. The full-screen pass `211d31f56893a8bc` first suspected is the sky (z = 1, depth EQUAL) and is not the cause.
  - Not analysed: why the body vanishes ~7 frames before the lines (likely the `cb7[37].w` discard path).
- **Post-processing / root cause (user: "Разобрать постобработку"):** the post chain is near-linear for these values (frame 3086, point 0: HDR emission `(0.0125, 0.028, 0.246)` → composite `ec9c…` `(0.056, 0.061, 0.234)` → tonemap `a929…` RGB10A2 `(0.043, 0.048, 0.225)` → final RGBA8 `(58, 62, 131)`), so bloom/tonemap are not the cause. Run **TPD-R-CB** (`LARECOMP_LOG_FS_CB=d5dfd5a97f910f0e:1:7:34:4`, sync shaders) logged the material constants every frame. They correct the earlier reading: `cb7[34] = 0` (no glow lerp), and `cb7[37].w` (dissolve progress) ramps smoothly 0.99 → 0.30 over ~80 frames. So the whole blue is the dissolve-edge term `(0.2 + n·cb1[4].y, 0.45 + n·cb1[4].z, 4 + 10·n)·saturate((1 − w)·2.5)` with noise `n = 0` everywhere: the measured `(0.050, 0.113, 1.0)` is exactly `(0.2, 0.45, 4)·0.25`. With `n = 0` the noise discard (`n < saturate((1 − w − 0.15)·1.176)`) also drops every pixel at once when `w` crosses 0.85, instead of dissolving per texel.
  - **Noise source:** `t4` is a `sampler3D`, bound to a 32×32 `f0x49` = **BC4_UNORM 3D texture** (`@3dca21000`; the same volume is used by other draws, e.g. `a790f9d96ccd4f0b`, tex5 of a sky-adjacent pass). **Cause (code reading, `runtime/nvn/vulkan/textures.c` `upload_texture`):** the CPU BC-decode path for GPUs without BC (and the ASTC path) deswizzles with depth 1 (`tegra_deswizzle(src, blocks, l.wb, l.hb, 1, …)`) and copies `imageExtent.depth = 1`. A 3D BC texture therefore gets only slice 0, read with a 2D block-linear layout, and the rest of the volume stays unwritten. This is Mali-specific: native BC (Adreno) takes the plain path, which passes `l.d`. Expected effect of a fix: per-texel noise, so the edge emission reaches HDR `4 + 10·n` in blue (pale cyan/white after tonemap and bloom, as in the reference) and the body dissolves gradually. Not yet verified by a build; other effects that sample this volume (fog/clouds) are probably affected too.
- **Fix attempt and real cause (user: "Да, исправляй"; local branch from `main` 4bad6dc, nothing committed):**
  - Attempt 1, all slices in the CPU BC/ASTC upload of 3D textures (`upload_texture`): built, installed, Hateno two-teleport run TPD-R-FIX. **No change.** A diagnostic shader patch (TPD-R-VIS2, `LARECOMP_SHADER_PATCH` writing `texture(t4, screen-space coords)` into RT5) showed that **t4 returns 0 at any coordinate**. Diagnostic logs (temporary `[diag3d]` builds): the 32×32×32 BC4 volume `@3dca21000` (6 levels, NVN target 2) is never CPU-uploaded (`uploaded 0`). It is `gpu_owned` from the start.
  - Attempt 2, decoding BC blocks in `copy_buffer_to_texture` before the copy into a CPU-decoded BC image: built, installed, TPD-R-FIX2. **No change** (that path is not used for this texture). Diff kept unverified in `.mali/teleport-dissolve/build/buf2tex-bc-decode-unverified.diff`.
  - **Actual cause (TPD-R-WATCH, `LARECOMP_WATCH_TEXADDR=3dca21000`):** at frame 519 the game *renders* the noise into the BC4 volume, slice by slice (render target, view `setmask 0x27`, layer 0..31), i.e. it GPU-compresses procedurally generated noise through an uncompressed block-sized view (one 8-byte BC4 block per texel). On Mali the image is the CPU-decoded R8 (`bc_vf`), so `vk_view` renders those block texels into the R8 image as colour, and the volume stays effectively zero. A real fix needs render-to-BC support for decoded images: a shadow target in the view format at block resolution, then a GPU (compute) or CPU BC4 decode into the R8 image after the draws. That is a design-level change, not done. It probably also matters for native BC (Adreno) unless block-texel-view-compatible images are already handled; not checked.
  - **State after the attempt:** source reverted, temporary branch `mali-3d-bc-upload` deleted, back on `pace-vsync-phase` (clean). Device reinstalled with stock 0.2.10 debug APK `e4a62881…` (verified on device). The APK installed before this work (updated 2026-11-02 22:39) was not hashed beforehand, so whether it was the same build is unknown. Built APKs kept in `.mali/teleport-dissolve/build/`.
- **Implementation (user: "Реализовать самим"; branch `mali-bc-render-target` from `main` 4bad6dc, uncommitted).** Release check 2026-11-03: `v0.2.12-beta` `c9fb160` is out (touch overhaul, widescreen map, destroyed-view logging; small `exec.c`/`textures.c` changes, not this path). Not migrated; the work stays on 0.2.10. The noise chain turned out to have three defects, all fixed in the diff (`.mali/teleport-dissolve/build/noisefix.diff`; `exec.c` +38/−5, `textures.c` +146, `vk.h` +9):
  1. **Level-0 upload (generic, not Mali-only):** the source R8 volume `@3dca0a000` (32×32×32, CPU-filled) is first drawn into at level 1 (mip generation). `begin_pass` marked it `gpu_owned` before any CPU upload, so level 0 stayed zero. Fix: a colour target first drawn into at `base_level > 0`, never uploaded and not GPU-owned, gets `vk_tex(t, false)` before it is marked.
  2. **Fragment output type (generic):** `rt_kind` came from the texture's format, not the view's. The BC4 compressor draws through an RGBA16_UINT view (`0x2C`), so its shader declared `vec4` into an integer target, which gives zeros on Mali. Fix: `rt_format(i)` (the view's format when it names one) for `fkey.rt_kind` and the blend test.
  3. **Render to CPU-decoded BC (Mali / no BC):** a draw into a `bc_vf` texture through a block-sized view goes to a shadow image in the view's format at block resolution (`vk_bc_shadow_view`). At the next `begin_pass` that does not draw into it, the blocks are read back (one `renderer_flush`, once after load), decoded with `bc_decode`/`bc1_to_astc`, and copied into the image (`vk_bc_shadows_resolve`).
- **Verification (TPD-R-FIX3, Hateno seed, Recompiled, default flags, APK `cd735345…`):** log `BC 32x32x32: 6 levels of blocks decoded (18541 of 18744 bytes nonzero)` (all zero before). Teleport 2 (pipelines built) matches the Switch reference: gradual shift to a pale translucent glowing cyan, the body stays visible and breaks up into the lines (sheet `.mali/teleport-dissolve/ref-before-after.jpg`). Teleport 1 still has ~2 frames without Link from async pipeline builds (a separate cause, unchanged). 30.0 FPS steady afterwards, no device loss or errors in ~5 min. Side effect seen: visible volumetric mist in the forest at Myahm Agana, which the same noise volume feeds. It is probably more correct, but not compared with Switch. Not done: broader regression (Great Plateau route, menus, other areas), Adreno, 0.2.12 rebase.
- **Regression check, 2026-11-03 (user: "Проверить регрессии").** Release check: newest is `v0.2.13-beta`, published 2026-11-03 03:40Z (20 fps, touch gestures, stray leaves); not migrated. Harness `.mali/teleport-dissolve/regress.py` runs one scripted route, Recompiled mode, default flags, on whichever APK is installed:
  - Great Plateau seed: stand, camera, walk, inventory menu, fast travel to the Shrine of Resurrection, walk.
  - Hateno seed: walk, map, fast travel to Myahm Agana, camera, walk.
  - Fix `cd735345…` runs: REG-FIX-GP and REG-FIX-HAT2. Stock 0.2.10 `e4a62881…` runs: REG-STOCK-GP and REG-STOCK-HAT.
  - REG-FIX-HAT is invalid: the HUD detector threshold was too high for the seed's low hearts; it was lowered to >5 red pixels.
  - **Results:** all four runs had 0 error lines, the app was alive at the end, and both fast travels completed (GP 23.7 s vs 23.7 s; Hateno 15.1 s vs 15.3 s). Median 30 FPS everywhere; sub-25 FPS samples are 9/6 (GP fix/stock) and 4/5 (Hateno), all during boot and teleport loading in both builds. The `gpu mem` maximum differs both ways (256 MB allocation blocks), so it is noise.
  - **Shadow path:** the only BC texture that took it in either area is the 32³ noise volume; nothing else in these routes renders to BC.
  - **Visuals** (sheets `regress/gp-sheet.jpg`, `regress/hat-sheet.jpg`): menus, map, shrine interior, HUD and terrain are the same, with no corruption. The fix adds visible distance haze/fog on the Great Plateau and mist in the Myahm Agana forest, from the same noise volume now being non-zero. This is expected, but not compared with Switch. Hateno `03-walked` differs only because the walk diverged (camera ended facing the sign).
  - **Device:** the fix APK was reinstalled at the end.
  - **Not covered:** Adreno, other regions, combat or weather effects, Native mode.
- **Code review and fixes, 2026-11-03 (user: `/code-review`, then "Исправить всё").** Two read-only review agents ran: one on standards (CONTRIBUTING.md plus a smell baseline), one on spec (this section). Standards: no hard violations, 7 judgement calls. Spec: 6 probable edge-case bugs, none of them triggered by BOTW's 32³ volume. All were addressed in the working tree (still uncommitted; diff `.mali/teleport-dissolve/build/noisefix-review.diff`; `device.c` +3, `exec.c` +55/−7, `textures.c` +229/−21, `vk.h` +10).
  - **Spec fixes:**
    - Only drawn (level, slice) pairs are decoded, via a per-slice `drawn` map, so the undefined parts of the shadow no longer overwrite the image.
    - Shadow level extents are `ceil((w>>lv)/4)` (`bc_blocks`). Level 0 is widened so that every level fits and the shadow has all of the texture's levels. This also fixes 2D textures missing their mip levels.
    - A target drawn through a view of another output type into an image of another format (ASTC/BC decoded, no shadow) is no longer bound. Before, its outputs were of the wrong type, which is undefined behaviour.
    - The shadow path follows `LARECOMP_3D_RT` via the shared `rt3d_mode()`, so the Qualcomm guard can no longer be bypassed with `LARECOMP_NO_BC`.
    - The decode also runs before dispatches, texture→buffer and texture→texture copies, and the present, not only before passes.
  - **Standards fixes:**
    - Shared `bc_upload_blocks` for both upload and resolve.
    - `bc_block_bytes` replaced with `FormatInfo::bpb`, held in a `BcShadow` struct (with `img`, `bsize`, `first[]`, `drawn`, `dirty`).
    - One format/extent path in `begin_pass`.
    - `nz` renamed.
    - The level-0 decision moved to `vk_tex_upload_for_mips` in textures.c.
    - The reset concern needs no code: `vk_textures_reset` destroys every backend, which empties the dirty list.
  - **Verification:**
    - APK `4921214b…` built; one warning, pre-existing, `exec.c:6852`.
    - TPD-R-REV, two-teleport harness: the dissolve looks as before (`TPD-R-REV/rev-dissolve.jpg`). Log `63 slices of blocks decoded (18541 of 18744 bytes nonzero)`, identical bytes, one resolve. 0 errors, 30 FPS.
    - REG-REV-GP and REG-REV-HAT: 0 errors, both fast travels done, median 30 FPS. The GP minimum of 3.8 FPS falls in the world-load compile burst (890 compiles; stock had 887 at 14.9 FPS, a different split across the 2 s windows). No `not drawn` lines.
    - Sheets `regress/rev-gp-sheet.jpg` and `regress/rev-hat-sheet.jpg` match the earlier fix: the same haze, no corruption.
    - The second teleport in TPD-R-REV was not performed (map aim failed); the first one was enough.
    - Not checked: the new edge-case paths themselves (no game content triggers them), Adreno, Native mode, a desktop build. The review APK is installed.
- **Audit, regression matrix and v0.2.13 compatibility, 2026-11-03 (user: Vulkan audit, Native/Recompiled, first and second teleport, menu, 3D LUT, fog, a long session, readback cost, comparison with stock 0.2.10, v0.2.13 check, no migration or merge; not a milestone).** Patch still uncommitted on local `mali-bc-render-target` (from `main` `4bad6dc`). Final diff `.mali/teleport-dissolve/build/noisefix-audit.diff` (sha256 `0e6809a2…`; `device.c` +3, `exec.c` +55/−7, `textures.c` +241/−21, `vk.h` +10). APK `noisefix-audit-debug.apk` `bcbd1b9e…`.
  - **Audit (code reading; confirmed by reasoning, not by a validation layer):**
    - Synchronization: every BC-shadow step is fenced by `vk_full_barrier`. One gap was found and fixed: there was no TRANSFER_WRITE→HOST_READ barrier between the image→buffer copy and the CPU read. Added before `renderer_flush`. The readback memory is host-cached and `vk_readback_ready` invalidates it.
    - Image layouts: the shadow and the decoded image stay in GENERAL. The shadow is initialised with `vk_image_init_layout` on the upload command buffer.
    - Staging: readback goes through the single grow-only `vk_readback` buffer, used right after a submit-and-wait. Decoded texels go through `vk_transient`, which lives until its submission completes.
    - Lifetime: `bc_shadow_destroy` runs from `destroy_image`. It removes the texture from the dirty list, calls `views_forget` before `defer` (image and memory are freed only after the pending sequence), and frees `drawn`. `backend_texture_destroyed` drains the render thread first. A format change of the drawing view recreates the shadow. A device reset destroys every backend.
    - Mips: per-level block extents come from `bc_blocks`. Level 0 is widened so that non-power-of-two levels fit. At most 16 levels.
    - Temporary objects: the region array is freed after each resolve, and `drawn` is cleared.
    - Also added (diagnostics only): a log line with decode number, frame, slices, GPU wait and decode time for the first 8 decodes, then every 64th.
  - **Matrix (`regress.py` v2 + `matrix.sh`, 2026-11-03 05:35–06:38Z, Mali-G925):**
    - 10 runs, alternating stock 0.2.10 `e4a62881…` (S) and the patch (P).
    - Recompiled (R) and Native (N), each verified from the log.
    - Routes:
      - GP: walk, inventory, teleport to the Shrine of Resurrection.
      - HAT: Hateno seed; walk, map, teleport 1 to Myahm Agana, forest/fog, inventory, teleport 2.
      - LONG: HAT plus 12 cycles of walk/menu/teleport, 15 min.
    - Starts gated at ≤36 °C (the runs started at 24–27 °C).
    - Table: `regress/compare.md`. Sheets: `m-hat-sheet.jpg`, `m-gp-sheet.jpg`, `m-long-sheet.jpg`.
  - **Confirmed:**
    - All 10 runs: 0 error lines (including DEAD VIEW), alive at the end, median 30 FPS.
    - Every teleport finished: patch 12.3–23.8 s, stock 12.4–25.0 s.
    - Share of play windows below 29 FPS:

      | Run | Patch | Stock |
      |---|---|---|
      | LONG | 143/564 | 142/565 |
      | R-HAT | 20/81 | 21/82 |
      | N-HAT | 23/81 | 25/81 |
      | R-GP | 15/58 | 15/60 |
      | N-GP | 15/55 | 15/57 |

    - 3D LUT: 4 `3D target 8x8x8` lines in every run, and the inventory menu is identical.
    - Fog/haze in the Myahm Agana forest and on the Plateau: no difference from stock on the sheets.
    - Dissolve: in the patched runs Link's body visibly breaks up into the lines on the first, second and every long-session teleport, in both modes. In stock the body is already gone in the same frames.
    - **Readback/decode count: exactly 1 per process**, at load (frames 438–500): 63 slices, 18541 of 18744 bytes nonzero in all 5 patched runs. There was none on any of the 14 teleports of the long session. The `past 0` upload happened once and `not drawn` never appeared.
    - Measured cost:
      - GPU wait (submit-and-wait incl. copy) 4.82–7.08 ms;
      - CPU BC4 decode 0.15–0.19 ms.
    - It falls inside the existing ~1.2–1.4 s load stall, which stock has too:

      | Run | Patch max frame ms | Stock max frame ms |
      |---|---|---|
      | N-GP | 1250 | 1214 |
      | N-HAT | 1297 | 1197 |
      | R-GP | 1400 | 1269 |
      | R-HAT | 1275 | 1312 |
      | LONG | 1298 | 1232 |

    - Original saves unchanged: sha256 of all 430 files under `v190/save`, `mali-step4-hateno-seed` and `mali-step4-great-plateau-seed-v1`. `regress/saves-before.txt` = `saves-after.txt`. Runs used copies in `files/mali-step4/M-*`.
  - **Hypotheses / not shown:**
    - The patch's load stall is on average about 60 ms longer (4 of 5 pairs; −37 to +131 ms). That is ~10× the measured wait, and the run-to-run spread has the same magnitude, so this is not established. If real, plausible sources are the extra `vk_tex` upload of the R8 source and pipelines for format 95.
    - The GPU wait is an upper bound on the added time, since that frame waits anyway.
    - A reload that recreates the noise volume (blood moon, death, title → load) would repeat the single decode. Not observed on the patch.
    - In stock LONG cycle 3, a blood-moon cutscene replaced the teleport. That is the 4.2 s value; it is not a teleport time.
  - **Limitations:**
    - One run per configuration.
    - Only Mali-G925.
    - No Adreno or desktop build.
    - No Vulkan validation layer.
    - The edge-case paths (non-power-of-two BC targets, format changes, 2D BC targets, a full dirty list) are never exercised by the game.
    - 15 min, not a multi-hour session. Upstream #176 is a Mali crash after ~30 min with a destroyed view.
    - The glow colour difference noted below is unchanged.
  - **Upstream release check 2026-11-03T06:40Z:**
    - Newest is `v0.2.13-beta`, tag commit `5403f006…`.
    - The active baseline for this patch stays 0.2.10 `4bad6dc` (bounded campaign; no migration).
    - Compatibility:
      - The diff applies cleanly to v0.2.13 sources (dry run on a scratch copy; not compiled, not tested).
      - Overlaps in `exec.c` `begin_pass`: v0.2.13 adds a DEAD VIEW check before `vk_begin_rendering`. Shadow views come from `vk_image_view` and are released through `views_forget` + `defer`, so they should take part in its dead-view tracking. This needs a build and a run.
      - `wb_eligible` changes, but BC textures are excluded from it.
      - `vk_collect_dead` / `vk_image_view` gain dead-view tracking.
    - Nothing was ported, merged or rebuilt on v0.2.13.
  - **Device left:** the audit patch APK `bcbd1b9e…` is installed; the app is stopped. Scratch dirs `files/mali-step4/M-*` remain. Stock 0.2.10 is in `.mali/upstream-migration/artifacts/` for rollback.
  - **Evidence (local only):** `.mali/teleport-dissolve/regress/` (M-* runs, `compare.py`/`compare.md`, `matrix.log`, sheets, `saves-before.txt`/`saves-after.txt`) and `.mali/teleport-dissolve/build/`.
- **Checkpoint, local commit and port to v0.2.13, 2026-11-03 (user: option 1, then a local Git commit of the patch, then option 2: port to v0.2.13 and test; not a milestone).**
  - Documentation checkpoint `27e9dad` was pushed to `mali-workspace` (this file and MALI_PLAN.md; readback matched).
  - **Local commit:** the patch is committed on local branch `mali-bc-render-target` as `7862afc7ccf8065a95616482c09589dd2772608e` (parent `4bad6dc`). Not pushed. No PR.
  - **Port:**
    - Release recheck at 06:56Z: still `v0.2.13-beta`. The campaign is pinned to `5403f006905343792cc1692a3ea9c4fca89c1506`.
    - New local branch `mali-bc-render-target-0213` from that SHA, with `7862afc` cherry-picked without conflicts as `050fbc2`. Not pushed.
    - Local `main` and fork `main` are untouched (still 0.2.10). No merge.
  - **Compatibility review:**
    - v0.2.13's #176 DEAD VIEW check sees the shadow views. They are created by `vk_image_view` (marked alive) and released through `views_forget` → `defer` → `vk_collect_dead` (marked dead), so a destroyed shadow view reaching a pass would be logged.
    - `e38e692` (#168, linear RT write-back) does not apply to BC textures.
    - Recompiler and game-lib interfaces are unchanged between 4bad6dc and 5403f00.
  - **Builds** (JDK 17, `assembleDebug`):
    - Patched: `BOTWRecompAndroid-5403f00-noisefix-debug.apk` `9a141dc5…`.
    - Stock: `BOTWRecompAndroid-0.2.13-beta-5403f00-debug.apk` `1c28041e…`.
    - Both are in `.mali/upstream-migration/artifacts/` and both report versionCode 33.
    - Only the patched libraries contain the shadow log strings; both contain DEAD VIEW.
    - One warning, pre-existing upstream code (`exec.c:6997`, blame 2026-10-20).
  - **Harness change:** 0.2.12 made the icon touch layout the in-world default (`touch_scheme = 1`). `regress.py` now writes `touch_scheme = 0`, the classic layout its coordinates tap; earlier versions ignore the key. It applies to both 0.2.13 builds equally.
  - **Matrix N-*** (06:58–08:05Z, same routes and modes as M-*): 10 runs, starts at 27.5–28.6 °C. Table `regress/compare-0213.md`; sheets `n-hat-sheet.jpg`, `n-gp-sheet.jpg`, `n-long-sheet.jpg`.
  - **Confirmed:**
    - All runs: 0 error lines, including **0 DEAD VIEW**. All alive at the end, median 30 FPS.
    - Teleports finished: patch 12.2–23.8 s, stock 12.2–24.2 s.
    - Share of play windows below 29 FPS:

      | Run | Patch | Stock |
      |---|---|---|
      | LONG | 150/567 | 156/563 |
      | R-HAT | 22/82 | 25/86 |
      | N-HAT | 27/83 | 23/83 |
      | R-GP | 15/57 | 14/57 |
      | N-GP | 16/55 | 15/55 |

    - 3D LUT: 4 lines in each run, and the menus are identical.
    - Fog: no difference on the sheets.
    - Stock 0.2.13 still has the bug (Link's body gone in the burst frames). The patched build shows the body breaking up on the first, second and long-session teleports, in Native and Recompiled.
    - **Exactly 1 readback/decode per process** (frames 452–559): 63 slices, 18541 of 18744 bytes nonzero. GPU wait 4.68–6.03 ms, decode 0.08–0.18 ms.
    - Load stall, max frame ms:

      | Run | Patch | Stock |
      |---|---|---|
      | N-GP | 1230 | 1227 |
      | N-HAT | 1267 | 1307 |
      | R-GP | 1427 | 1432 |
      | R-HAT | 1272 | 1346 |
      | LONG | 1260 | 1474 |

      Here the patch is lower in 4 of 5 pairs. On 0.2.10 it was higher in 4 of 5. Across the 10 pairs there is no consistent direction, so the earlier "+~60 ms" hypothesis is not supported. The stall is run-to-run noise around the ~5–7 ms measured cost.
    - Original saves are unchanged after this series as well: all 430 files match `saves-before.txt` (`saves-after-0213.txt`).
  - **Not a teleport time:** in N-P-R-LONG cycle 3, a blood-moon cutscene replaced the teleport (2.5 s). As in M-S-R-LONG, it ran within the same process with no repeat decode.
  - **Limitations:**
    - One run per configuration, Mali-G925 only.
    - No Adreno or desktop build, and no validation layer.
    - The 15-min sessions are still shorter than the ~30-min crash window of #176.
    - The edge-case paths are still never exercised by the game.
  - **Device left:** the patched 0.2.13 APK `9a141dc5…` is installed (versionCode 33; before this, 0.2.10 builds were installed). The app is stopped. Scratch dirs `files/mali-step4/N-*` remain. Rollback: the stock 0.2.10 or 0.2.13 APK with `install -r -d`.
  - **Evidence (local only):** `.mali/teleport-dissolve/regress/N-*`, `matrix-0213.sh`/`matrix-0213.log`, `compare-0213.md`, the sheets, and build logs `build/assemble-0213-*.log`.
- **Upstream PR prepared locally, 2026-11-03 (user: "Готовить upstream PR от v0.2.13"; not a milestone; nothing pushed or opened).**
  - **Release check 08:08Z:** still `v0.2.13-beta`. `upstream/main` = `5403f00`, so the PR base is the tag itself.
  - **Upstream state:** open PRs #183 (ours), #187–189 (others) and #96 touch none of the patch's code paths. No upstream issue for the dissolve bug was found (searched teleport / fast travel / dissolve).
  - **PR branch:** local `mali-bc-render-target-0213`, one commit `050fbc2` on `5403f00`. It changes only 4 files in `runtime/nvn/vulkan` (+281/−28).
  - **Branch checks:**
    - Line endings: CRLF, like the repository.
    - `git diff --check`: clean.
    - Content: no private paths, device serials, `.mali` references or Nintendo files.
    - The exported patch applies cleanly to `5403f00`.
  - **Host tests (NDK 30), identical on `050fbc2` and `5403f00`:** timing history 8, shader cache 17, Vulkan startup 33, Vulkan compat 16 checks pass.
  - **Package** in `.mali/teleport-dissolve/pr/`:
    - `PR_DRAFT.md` (title, summary, evidence, merge danger, limitations);
    - `before-after.jpg` (stock 0.2.13 vs the patch, both fast travels; game screenshots, allowed by CONTRIBUTING.md);
    - `teleport-dissolve-bc-shadow.patch`;
    - `diffstat.txt`.
  - **Stated in the draft:** `rt_format` and the "not drawn" guard apply on every platform, not only Android. Adreno is untested.
  - **Open:** pushing the branch to the fork and opening the PR need separate authorization. The commit is authored by the Propinbo noreply identity used for earlier commits.
- **Upstream PR opened, 2026-11-03 (user: "Да, открыть PR").** Before pushing, `upstream/main` and the newest release were rechecked: still `5403f00` / `v0.2.13-beta`.
  - Branch `mali-bc-render-target-0213` (`050fbc2`) pushed to the fork `Propinbo/BOTWRecompAndroid`.
  - PR https://github.com/mamcjr/BOTWRecompAndroid/pull/210 opened against `main` with `gh`. Body: `PR_DRAFT.md` plus the Claude Code attribution line.
  - `before-after.jpg` was uploaded through the user's signed-in Chrome, via the new-comment file input. That comment draft was cleared and not posted (PR comments: 0). The resulting user-attachments image was added to the description with `gh pr edit`.
  - Status at opening: open, mergeable/clean, no CI checks configured. The session's PR monitor (auto-fix, address comments) is on; auto-merge is off.
  - Local `main`, fork `main` and upstream were not changed.
- **Device left (implementation):** the fix build `cd735345…` is installed (versionCode 30). Stock 0.2.10 `e4a62881…` is in `.mali/upstream-migration/artifacts/` for rollback.
- **Device left (shader capture):** app stopped; the `files/v190/build/` diagnostic folder created by these sessions was removed after the GLSL was copied; `files/v190/save` untouched. Evidence (local only): `.mali/teleport-dissolve/TPD-R-PICK1/`, `TPD-R-DUMP/` (logs, `glsl/`, recordings, `dec.py`).
- **Remaining difference (not investigated):** with sync shaders, the glowing Link is a saturated blue/purple, while the reference is a pale translucent cyan/white, and the body disappears before the lines grow instead of under them. Possibly the glow shader's colour/blend or bloom; the shader capture was not needed for the first issue and was not done.
- **Device left:** app stopped, recordings removed from `/sdcard/Download`, scratch dirs `files/mali-step4/TPD-*` left in place; the user's save and settings were not touched.
- **Evidence (local only):** `.mali/teleport-dissolve/` (harness `tp-dissolve.py`, per-run video/log/commands, `compare-depart.png`, `ref-vs-mali-depart.jpg`).

## #174 both-flags run on 0.2.10, 2026-11-02 (user: "Да, оба флага"; not a milestone)

- **Result: normal at all four check points** with `LARECOMP_BC1_ASTC=0` + `LARECOMP_LIN_STAGGER=1` (tall grass, Link on the ground at ~5/10/15/20 min). With either flag alone the defect appears (BC1OFF-N-3, LINSTAG-N-1). So each of `596615e` and `8053dc8` is sufficient on its own, and the 0.2.9 ADPF change `b6725c5` is excluded. Most likely both expose a shared underlying problem; not investigated. One run per configuration; onset variance applies.
- **Run BOTH-N-1:** APK `e4a62881…` (0.2.10), warm bundle `after-GP210-N-S2-warm-1` (654 files, verified), same route/settings/60 Hz/brightness; mode verified Native; no ASTC log line (BC1 flag effective). 37,244 frames, 29.82 FPS, p99 50.05 ms, 2,158 >50 ms (the phase-burst judder, unrelated to #174), 2 >100 ms, max 217 ms. Returns graceful (2.1 s), re-boot 24.7 s.
- **Device left:** 0.2.10, refresh keys deleted, brightness auto, app stopped; working cache = BOTH-N-1 state (bundled `after-BOTH-N-1`), previous archived as `mali-archive/before-BOTH-N-1`.
- **Evidence (local only):** `.mali/issue-174-both/` (harness, conditions, console, captures, `sheet-all.png`, `sheet-comment.jpg`, draft `comment-both.md`). On the user's "Да, публикуй" posted from the user's account via Claude in Chrome at 2026-11-02T20:12:54Z, https://github.com/mamcjr/BOTWRecompAndroid/issues/174#issuecomment-6587371018 (5-row sheet as JPEG; final text `comment-both-final.md`); it also carries the 0.2.8 result.

## #174 0.2.8 run on the same route, 2026-11-02 (user: "публикуй, потом 0.2.8"; not a milestone)

- **Comment posted first:** the LIN_STAGGER result on #174 from the user's account via Claude in Chrome, 2026-11-02T19:10:45Z, https://github.com/mamcjr/BOTWRecompAndroid/issues/174#issuecomment-6586034785 (sheet as JPEG; text `.mali/issue-174-lin/comment-linstagger-final.md`).
- **Result: 0.2.8 is normal at all four check points** (tall grass, Link on the ground at ~5/10/15/20 min), like 0.2.5 (STAB-N-3). One 20-min run; onset varies (≤5 to 10-15 min on 0.2.10), so this is strong but not conclusive. Combined with the two flag runs: the defect appears in 0.2.9/0.2.10, yet neither `596615e` alone nor `8053dc8` alone is required. Remaining explanations: either change alone is enough to trigger it (a common mechanism, e.g. timing or memory pressure on terrain updates), or the 0.2.9 ADPF change `b6725c5`. Discriminating next step: 0.2.10 with both `LARECOMP_BC1_ASTC=0` and `LARECOMP_LIN_STAGGER=1`. Not run; not authorized.
- **Run V028-N-1:** APK `c6ab822e…` (0.2.8-beta `b02302c`, debug, same signer) installed in place with `install -r -d` (no uninstall or clear; `firstInstallTime` preserved), hash verified. Warm bundle `after-GP28-N-S2-warm-1` (656 files restored, hashes verified); route, menus, settings, 60 Hz, brightness 128 as the 0.2.10 runs; mode verified Native; no ASTC line (expected on 0.2.8). 37,240 frames, 29.83 FPS, p99 49.98 ms, 204 >50 ms, 1 >100 ms, max 200 ms. Returns graceful (2.1 s each), re-boot to world 23.6 s.
- **Device left:** 0.2.10 APK `e4a62881…` reinstalled in place (versionCode 30, `firstInstallTime` preserved), refresh keys deleted, brightness auto, app stopped. Working cache = V028-N-1 state (bundled `after-V028-N-1`); the LINSTAG-N-1 cache is archived as `mali-archive/before-V028-N-1`.
- **Evidence (local only):** `.mali/issue-174-v028/` (harness copies, install record, conditions, console, captures, `sheet-025-028-0210.png` rows 0.2.5 STAB-N-3 / 0.2.8 V028-N-1 / 0.2.10 LONG210-N-1 / 0.2.10 LINSTAG-N-1). Nothing about 0.2.8 posted upstream.

## #174 cause-finding run: `LARECOMP_LIN_STAGGER=1`, 2026-11-02 (user: "Да, запускай"; not a milestone)

- **Result: the defect reproduces with `LARECOMP_LIN_STAGGER=1`** (pre-`8053dc8` every-8th-frame check of linear textures). Normal at ~5 and ~10 min (tall grass, Link on the ground), defect at ~15 and ~20 min (no grass, Link knee-deep), normal after launcher return + re-boot. So `8053dc8` is excluded too. With the flag, 0.2.10 behaves like 0.2.9 for this path, and `596615e` (BC1→ASTC) is the 0.2.9 change already excluded by BC1OFF-N-3. The cause therefore most likely predates 0.2.9: between 0.2.5 (STAB-N-3 normal at all four points) and 0.2.8 (LONG28-N-1 not comparable: teleport into the shrine at the first boundary). Not confirmed by a 0.2.9 or 0.2.8 run on the same route. Onset time varies between runs: 5-10 min (LONG210-N-1), ≤5 min (BC1OFF-N-3), 10-15 min (here); LONG8-N-1 (M8 candidate, 0.2.10) also shows it from ~10 min.
- **Run LINSTAG-N-1:** installed APK verified `e4a62881…` (0.2.10-beta `4bad6dc`); same route, menus, warm bundle `after-GP210-N-S2-warm-1` (654 files restored, hashes verified), settings, 60 Hz and brightness 128 as LONG210-N-1/BC1OFF-N-3; mode verified Native. The flag has no log line; it went through the same intent `env` path as `LARECOMP_LOG_FILE` (which took effect), and the ASTC line is present as expected. 37,215 frames, 29.82 FPS, p99 49.96 ms, 163 intervals >50 ms, 3 >100 ms, max 150 ms. Returns graceful (2.1 s each), re-boot to world 27.5 s. Same deviations as BC1OFF-N-3 (USB power and ADB, no GPU/thermal sampling).
- **Device left:** refresh keys deleted, brightness auto (reads 87), app stopped. Working cache = LINSTAG-N-1 state (bundled `after-LINSTAG-N-1`, 712 files); previous working cache archived as `mali-archive/before-LINSTAG-N-1`.
- **Evidence (local only):** `.mali/issue-174-lin/` (harness copies differing only by flag/paths, `make-sheet.py`, conditions, console log, captures, `sheet-ref-bc1off-lin.png` rows LONG210-N-1 / BC1OFF-N-3 / LINSTAG-N-1). Nothing posted upstream.

## #174 cause-finding run: `LARECOMP_BC1_ASTC=0`, 2026-11-02 (user: "Только BC1_ASTC=0"; not a milestone)

- **Result: the defect reproduces with BC1→ASTC off**, already at the ~5 min check point (default ASTC run LONG210-N-1: first seen between 5 and 10 min). Present at ~5/10/15/20 min. Gone after launcher return + re-boot (also normal in LONG210-N-1). So `596615e` (BC1→ASTC) is excluded as the cause; the remaining 0.2.10 suspect is `8053dc8` (per-frame checks on CPU-written linear textures). 0.2.9 not run.
- **Run BC1OFF-N-3:** installed APK verified `e4a62881…` (0.2.10-beta `4bad6dc`); route, menus, warm bundle `after-GP210-N-S2-warm-1` (654 files, hashes verified) and settings identical to LONG210-N-1; flag confirmed by the absence of `[vk] BC1 textures as ASTC 4x4` in the app log. 37,227 frames, 29.82 FPS, p99 50.0 ms, 1,339 intervals >50 ms (LONG210-N-1: 232), 4 >100 ms, max 917 ms. Returns graceful (2.1 s), re-boot to world 23.7 s.
- **Deviations:** USB power and ADB over USB (battery 40%, no charge-to-78% gate; battery/external-power gates removed from the copied script); no 1 Hz GPU/thermal sampling (no root after the device reboot; samples recorded as failed).
- **Harness fix:** after today's device reboot the shell user lost access to `files/v190` and `chown` on external storage fails. BC1OFF-N-1 stopped at settings `chown`, after a cache restore that silently restored 0 files; BC1OFF-N-2 stopped at a settings read-back check (`adb shell cat` adds CR). Neither reached the game. The copies in `.mali/issue-174-bc1/` do cache test/mv/tar and settings writes through `run-as`, and verify by sha256.
- **Device left:** refresh keys deleted, brightness auto (reads 87), USB ADB, app stopped. The working shader cache is now the BC1OFF-N-3 state (bundled as `after-BC1OFF-N-3`, 698 files); the previous working cache is archived on the device in `mali-archive/before-BC1OFF-N-2` (1,924 files). Leftover scratch dirs/ini from N-1/N-2 are harmless.
- **Evidence (local only):** `.mali/issue-174-bc1/` (scripts, conditions, console logs, BC1OFF-N-3 captures and log, comparison sheets, draft comment `comment-bc1off.md`, shareable log `issue174-native-bc1astc0-app.log`, reviewed: app paths and GPU info only). On the user's "Да, публикуй" the comment was posted from the user's account via Claude in Chrome at 2026-11-02T18:18:35Z (https://github.com/mamcjr/BOTWRecompAndroid/issues/174#issuecomment-6584799813), with the Link crop sheet and the app log attached (as `.txt`: GitHub rejected `.log`). Final text: `comment-bc1off-final.md`.

## Release check and upstream feedback, 2026-11-02T17:28Z (no milestone executed)

- New release `v0.2.11-beta` `3556864` (published 15:55Z): input dead-zone floor, in-place mod toggle, version bump. No change to rendering, textures or pacing. Not relevant to #174 or #183; #183 still merges cleanly onto it. Active baseline stays `4bad6dc`; migration only on separate authorization at the next checkpoint boundary.
- #174 maintainer reply: not reproduced on Adreno 830 Native (10.5 min, standing in a grass field); another user sees it on Poco F6 (Hateno) and #159 on Adreno 735, so it is probably not Mali-specific. Requested: Native run with `LARECOMP_BC1_ASTC=0` on the same route, and/or 0.2.9 on the same route, plus a log of a run showing the defect. This matches the bisect/flag step already listed as the cheap option in the 0.2.10 reassessment. Not authorized yet.
- #183: our H-A follow-up comment is posted; no maintainer response yet.

## Milestone 10 outcome: COMPLETE, H-A supported, 2026-11-02 (Claude Code session)

Authorized by "Приступай к шагу 10". Pin `pace-vsync-phase` `9c6ab0e` (= M7 candidate patch, diff SHA-256 `b44927ae...`, candidate APK `f4482815...`). Release checks 08:41Z and 10:03Z: newest still `v0.2.10-beta` `4bad6dc`, main unchanged; PR #183 open, no comments or reviews. Conditions: SM-X730, 120 Hz forced, Native S1, warm cache, 60 s settle + 180 s window, USB power (recorded deviation). Evidence local only: `.mali/milestone-10/` (`M10_REPORT.md`, trace, analysis, runs, DIAG patch and APK).

- **FrameTimeline window M10-FTB-D120** (Perfetto FrameTimeline + atrace gfx). The game's Vulkan SurfaceView has no per-layer FrameTimeline frames, so each game frame was followed via the layer's `BufferTX` counter (arrival, latch), `VSYNC-sf` wakeups and the SF display frame that showed it. Of 5173 frames, 4709 had 33.3 ms intervals, 230 had 41.7 ms and 231 had 25.0 ms. In all three classes:
  - queue lead was 3.8-3.9 ms;
  - the transaction reached SF 0.4-0.5 ms after the queue;
  - every frame was latched at the first SF wake after arrival.

  Only the time from latch to screen differs. It is 15.2 ms for 33.3 and 41.7 ms frames and 7.6 ms for 25 ms frames. Display frames: 8379 were `Late Present` / `Display HAL`, exactly one vsync (8.49 ms) after SF's expectation, and 888 were on time. The 25 ms frames are those on-time presents, mostly single ones; a 41.7 ms interval follows in 206 of 231 cases. **H-A supported:** display-HAL present latency jitters by one vsync, outside the app. H-B as stated is contradicted. V1 as implemented would be a no-op, and V3 is excluded because leads are identical across classes. Window background: block mean 0.089, max 0.173. One ~7.5 s collector ring gap was excluded. Attempt 1 is invalid because the tablet was locked (keyguard); the user unlocked it.
- **V2 check** (user chose "Проверить V2" over ending or running the full V0-V3 set). DIAG APK `fc428141...` = `9c6ab0e` + env-gated `m10-diag.diff` (`f44d3292...`, `device.c` only). Interleaved windows; `setFrameRate` returned 0 in both V2 windows:

  | Window | FPS | p99 | Block mean / max |
  |---|---|---|---|
  | V0-1 | 30.000 | 41.69 | 0.134 / 0.200 |
  | V2-1 | 30.000 | 41.70 | 0.199 / 0.353 |
  | V0-2 | 30.000 | 41.69 | 0.146 / 0.273 |
  | V2-2 | 30.000 | 41.71 | 0.247 / 0.320 |

  V2 fails the pre-set criterion and is worse than V0 in both windows. No 60 Hz check was needed.
- **Conclusion.** The residual 120 Hz background is outside the app, and no tested lever removes it. PR #183 needs no change. A factual comment on #183 is possible but needs separate authorization. Not covered: other devices or HWC vendors, adaptive refresh settings, FIFO present mode, battery power.
- **Restoration.** Baseline APK `e4a62881...` reinstalled and hash-verified after each driver; refresh keys deleted; brightness auto mode 1. Source tree restored to a clean `9c6ab0e`. No commit, push or upstream write.
- **#183 comment (user: "фактический комментарий в #183 о причине фона при 120 Гц", then "делай все, что считаешь нужным"):** posted from the user's account at 2026-11-02T10:09:24Z, https://github.com/mamcjr/BOTWRecompAndroid/pull/183#issuecomment-6574475018. Summary figures only; no logs or evidence files published. Local copy `.mali/milestone-10/pr183-comment.md`.
- **Next (needs authorization):** the open M8 S3 checks (60 FPS in-world, movie).

## Plan revision: milestone 10 added, 2026-11-02 (executed above)

The user approved adding a milestone for the residual 120 Hz 41.7 / 25 ms background ("Да, сформируй новый план для этой ошибки"). Reason: milestone 8 showed that this background is phase-independent and separate from the phase bursts fixed by PR #183. It was attributed to the compositor without proof. Milestone 10 in MALI_PLAN.md defines one bounded cycle: a FrameTimeline jank classification, then env-gated variants (`desiredPresentTime`, `setFrameRate(30)`, lead changes) at 120 Hz. Its success criteria are fixed before measurement. Estimated time is about 2.5-3 h, of which about 1.5 h on the device. Completed milestones 1-9 are unchanged. Execution needs `Приступай к шагу 10`. At the time of this revision the tablet (SM-X730) was reachable over ADB, and nothing was run on it.

## Milestone 9 outcome: COMPLETE, 2026-11-02 (Claude Code session)

Authorized by "Приступай к шагу 9". Preceding checkpoint: MALI-M8-COMPLETE-2026-11-02. Nothing is committed, pushed or submitted.

- **Upstream recheck (06:33Z):** newest release `v0.2.10-beta`, main `4bad6dc`; the only open PR is draft #96 (AAudio); no PR, commit or issue touches present pacing or `VK_GOOGLE_display_timing`. No adaptation needed.
- **Patch review:** the working-tree diff (4 files, +133/-8, SHA-256 `b44927ae...3cce`) is the milestone-7 candidate, identical to what built candidate APK `f4482815...`. `git apply --check` on a pristine `4bad6dc` export passes; `git diff --check` is clean; no planning files, logs, generated code, game data, local paths or diagnostic code. Not rebuilt or retested (no source change since M7/M8).
- **Package (local, `.mali/milestone-9/`):** `botw-vsync-phase-pacing.patch`, `PR_DRAFT.md` (title, body, test table, limitations), `REPRODUCTION.md`, `M9_REPORT.md`, `diffstat.txt`. Shareable evidence is the summary tables in the draft; raw evidence stays under `.mali/milestone-5b|7|8/`.
- **Claims vs evidence:** the draft reports P1, P3, P4, P5, S1 and S2 as passed, and states plainly that P2 failed at 120 Hz (max block 0.253 vs 0.16; mean 0.175 vs baseline 0.228) and that 60 FPS in-world and movie checks were not done; one device, USB-powered sweeps and long session, one cell per baseline comparison, no automated regression test.
- **Submitted (user: "Отправлять PR как есть", 2026-11-02):** commit `9c6ab0e472c9b1255a16f4c83475b0b58d40fbfd` on branch `pace-vsync-phase` (fork), diff SHA-256 unchanged; upstream PR https://github.com/mamcjr/BOTWRecompAndroid/pull/183 with the draft body (limitations included). Only the 4 source files; no evidence files published.
- **Decisions for the user:** (1) submit as is, run the open S3 checks first, or hold; (2) separate authorization is needed for each of: committing the patch on a branch, pushing it, publishing evidence, opening the upstream PR. Milestone 9 completion authorizes none of them.

| Release check | Newest release | Tag SHA | Active baseline | Note |
|---|---|---|---|---|
| 2026-11-02T06:33Z | `v0.2.10-beta` (2026-11-01T15:14:53Z) | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.10-beta` `4bad6dc` | No newer release; `main` still `4bad6dc`. |

## Milestone 8 outcome: COMPLETE with one failed criterion (P2), 2026-11-02 (Claude Code session)

Authorized by "Приступай к шагу 8"; "Делай что надо" authorized the extra P2 cause runs. Candidate (4 files, +133/-8, diff SHA-256 `b44927ae...`, uncommitted on `4bad6dc` = v0.2.10-beta) compared with unchanged 0.2.10 (APK `e4a62881...`). Candidate APK `f4482815...`; sweep APK `e6457459...` (candidate + temporary diagnostic grid sweep, local only). Evidence local only under `.mali/milestone-8/` (Part A sweeps in `runs/`, Part B performance/stability in `perf/`, summary in `PART_A_RESULTS.md`).

**Results against the milestone-6 criteria**
| Criterion | Result |
|---|---|
| P1 (60 Hz, share per 150-frame block <=0.02, no run >=6 alternating) | PASS: max block 0.013, longest off-grid run 2 |
| P2 (120 Hz, no block above 0.16) | **FAIL**: max block 0.253 (mean 0.175). Criterion not relaxed. Cause runs: baseline pacing + sweep 0.228/0.480; candidate + sweep 0.175/0.253; candidate, no sweep 0.176/0.333 (mean/max). The candidate removes the phase peak, but a ~0.17 phase-independent background remains at 120 Hz, attributed to compositor-side behaviour outside the patch (not proven on other devices) |
| P3 (lead within r/2+-r/4 for >=99%) | PASS: 1.000 at 60 Hz and 120 Hz |
| P4 (`LARECOMP_PACE_VSYNC=0` reproduces baseline zone) | PASS: block 0.433, lead share 0.488 |
| P5 (Recompiled 60 Hz sweep meets P1) | PASS: max block 0.013, longest run 1 |
| S1 (FPS within +-0.05 of 0.2.10 cells, >100 ms count not above baseline+1, p99 <=33.6 ms; 4 cells, 120 s) | PASS. Candidate vs 0.2.10: N cold 29.971/29.971 FPS, >50 ms 14/21, >100 ms 1/1; N warm 29.933/29.933, 14/50, 2/3, p99 33.47 vs 50.01; R cold 29.971/29.975, 14/19, 1/1; R warm 29.967/29.929, 13/15, 2/2. p99 <=33.5 ms in every candidate cell |
| S2 (no burst run >=20 alternating intervals, long session with 3 menus + re-boot) | PASS: long session LONG8-N-1 (Native, warm cache, 4x300 s, 37379 frames): 29.84 FPS, p99 49.93 ms, 159 intervals >50 ms, 3 >100 ms, max 166.6 ms; longest alternating off-grid run 4. 0.2.10 long session for comparison: 29.85 FPS, p99 50.0 ms, 232 >50 ms, 3 >100 ms, max 150 ms |
| S3 (graceful teardown/re-boot, no crash or visual change, smoke checks) | PARTIAL: graceful teardown confirmed in all windows; 3 menu transitions passed, launcher return 2.2 s, re-boot to world 26.8 s; no crash recorded in the session; captures reviewed, no visual artefacts. 60 FPS setting smoke (`frame_rate = 60`, candidate): launcher at 16.67 ms, title screen stable at 33.33 ms, process alive; the world was not reached by hand, so 60 FPS in-world is **not verified**. **Movie not checked** (no automatic route) |

**Limitations and deviations (recorded)**
- Sweeps (Part A) and the long session ran on USB power, a deviation from M3-v1 battery conditions; short S1 cells kept battery conditions. The candidate S1 cells ran at a cooler ambient (23-24 C battery) than some 0.2.10 cells.
- The 0.2.10 cell comparison is one cell per mode/cache, so variability is not separated from the effect; the >50 ms reduction is supportive only.
- Single device (Samsung SM-X730, Mali-G925); other vendors and 90 Hz panels untested. The long session is 20 min of crash-free testing, not proof of absence of crashes.
- Failed/collided attempts kept aside (`attempt1-120hz-invalid`, `attempt2-archive-collision`, `attempt3-scratch-collision`, `M8-SMOKE60-attempt-collector-needs-battery`, `before-M8-SMOKE60-attempt1-incomplete`/`attempt3`); the SMOKE60 helper edits to the temporary settings copy were reverted (`frame_rate = 30` verified).
- Device restored: baseline APK `e4a62881...` reinstalled and hash-verified, `peak_refresh_rate`/`min_refresh_rate` deleted, brightness 88 with auto mode, ADB back to USB (`service.adb.tcp.port` 0). Working tree equals the candidate (diff SHA unchanged).
- Verdict: the patch meets P1, P3, P4, P5, S1, S2 and removes the 60 Hz bursts, but fails P2 at 120 Hz and leaves the 60 FPS in-world and movie smoke checks open. A failure does not authorize another implementation cycle. **Milestone 9 is not authorized**; scope/priority decisions on P2 and the open smoke checks belong to the user.

| Release check | Newest release | Tag SHA | Active baseline | Note |
|---|---|---|---|---|
| 2026-11-02T05:13Z | `v0.2.10-beta` (2026-11-01T15:14:53Z) | `4bad6dcb0390ae81d0e5d2e174809422d7198d97` | `v0.2.10-beta` `4bad6dc` | No newer release; `main` still `4bad6dc`. |

## Milestone 7 outcome: COMPLETE, 2026-11-01 (Claude Code session)

- **Authorization.** The user's `Приступай к шагу 7`. Milestone 8 is not authorized.
- **Release checks.** At 18:56Z and 19:47Z the newest release was still `v0.2.10-beta` = upstream main `4bad6dc`; the only open PR is #96 (AAudio), with no overlap. The work was pinned to `4bad6dc`.
- **Candidate patch** (uncommitted in the working tree; local copy `.mali/milestone-7/m7-candidate.patch`, SHA-256 `b44927ae…3cce`). 4 files, +133/−8:
  - **`vulkan/device.c`:**
    - enables `VK_GOOGLE_display_timing` where advertised (Android; `LARECOMP_PACE_VSYNC=0` leaves it off);
    - chains `VkPresentTimesInfoGOOGLE` with `desiredPresentTime` 0;
    - after each present, `vsync_update` keeps the newest `actualPresentTime`, the refresh period (re-queried every 32 presents), a fit check, and the median of the last 9 hand-over→queueBuffer delays;
    - `backend_vsync_offset()` returns the shift that puts queueBuffer half a vsync before the next vsync. It returns false without valid or recent timings, or when the grid is not a whole number of vsyncs.
  - **`objects.c`** (`framepace_on()` grid only):
    - the full shift at a grid restart;
    - otherwise 0.25 ms/frame steering once the shift is above 0.5 ms;
    - a cap of one vsync of steering per correction, then left until the next restart.
  - **`vulkan/async.c`:** the PRESENT request carries the hand-over time.
  - **`nvn.h`:** one declaration.
- **Gate G0: pass.** Timings collected and pacing unchanged (`NOSTEER` diagnostic). Native S1, warm cache, 60 s windows, USB power (recorded deviation).
  - `actualPresentTime` equalled SurfaceFlinger's actual present for 1800/1800 frames at both 60 and 120 Hz.
  - refreshDuration was 16.66 / 8.33 ms.
  - The app's queue time is 0.13 ms before SurfaceFlinger's desiredPresent.
  - The in-window hand-over→queue delay was median 1.0-1.1 ms, below 3 ms.
- **Design changes against the milestone 6 proposal.** These are within the approved mechanism and driven by evidence; the success criteria are unchanged.
  1. **Dead band ±r/4 → ±0.5 ms, step 0.5 → 0.25 ms per frame.** At 120 Hz, G0 showed a 0.227 burst share at a lead of 5.97 ms, inside the proposed band. The M5B-D120 sweep also shows elevated shares at leads of 5.8-8 ms: the 120 Hz zone spans about 1.5 ms before to 2.5 ms after a vsync.
  2. **A whole-vsync grid guard.** No phase lock without a frame limit or with a non-multiple grid.
  3. **A steering stop/continue rule** (smoke v1 defect). After an overshoot, the grid went the long way round through the bad zone: a 17.7 ms shift in 70 frames.
  4. **Median-of-9 delay and the one-vsync cap** (smoke v2 defect). After loads, a clamped average decayed at exactly the steering rate, so steering chased it on the title screen and the world load.
  5. The period is logged only on a change of more than 1%.
- **Smoke v3, final logic** (functional check, not milestone 8 validation; candidate + DIAG-M7 logging, APK `c5d551a5…`):

  | Run | Lead median | Lead in r/2±r/4 | Burst share | Max 150-frame block | FPS / p99 | Max grid shift in 70 frames |
  |---|---|---|---|---|---|---|
  | SMK3-T60 | 8.08 ms | 100% | 0.000 | 0.000 | 30.000 / 33.5 | 3.7 ms |
  | SMK3-T120 | 3.87 ms | 100% | 0.103 | 0.153 | 29.92* / 41.7 | 4.2 ms |

  The steering cap never triggered, and all teardowns were graceful.

  \* The SMK3-T120 timing is flagged invalid only because the collector lost SurfaceFlinger's ring once (5 frames). The app's own display timings show all 1800 frames, with a maximum interval of 41.7 ms.
- **120 Hz residual (risk for milestone 8 P2).** Late-shown frames at 120 Hz were queued and GPU-ready on time; the misses are isolated (runs of 2-4) and on the compositor side. This is the phase-independent 120 Hz background noted in milestone 5b, outside the patch's scope. Max block 0.153 is close to the P2 limit of 0.16. Changing P2 would need the user's approval; it was not changed.
- **Local verification.**
  - All `assembleDebug` builds exited 0 with the verified environment. No compiler warnings in the changed files.
  - Candidate APK `BOTWRecompAndroid-0.2.10-4bad6dc-M7-CANDIDATE-debug.apk`:
    - SHA-256 `f4482815c3d366522688d7902b8e0990afd6dff1fd6f0dedc6792e8be39fdae6`, 99,683,337 bytes;
    - versionCode 30, v2 signature with the same debug signer `60fa2a5f…`, zipalign OK;
    - game-library ABI `65da5d95e597ba58` unchanged;
    - not installed.
  - Host tests match the baseline: 8 / 17 / 33 / 16 PASS. Upload caches fails as before (the 3b known limitation).
  - No regression test was added. The behaviour depends on SurfaceFlinger's latch, and a host test would need moving the arithmetic into a header. Device validation is required.
  - No `RT_ON_RESET` hook is needed: a reboot is a long frame, so it restarts the grid.
- **Restoration.** Baseline APK `e4a62881…` was reinstalled after every device phase (hash verified). Refresh keys are null and brightness is on auto mode 1 (reads 75-82 after adaptive adjustment; 88 written back). ADB is USB-only.
- **Evidence (local only).** `.mali/milestone-7/`: `M7_REPORT.md`, patches and build diffs, G0/SMOKE/SMOKE2/SMOKE3/CANDIDATE APKs, build logs, scripts and helper copies, analysis scripts, `runs/`, `host-tests/`. No commit, push or upstream write.
- **Next step (needs explicit authorization; not started).** Milestone 8 validates the candidate with the milestone 6 criteria. The M5B sweep diagnostic patch must first be rebased onto the candidate. Decisions before milestone 8:
  - **P2: kept as defined (user decision, 2026-11-01, "Оставить P2 как есть").** At 120 Hz no 150-frame block may exceed 0.16, even though the compositor-side background is close to that limit. If P2 fails, the result is recorded as a failure with its cause analysed; the criterion is not relaxed after the fact.
  - **Long session: USB power (user decision, 2026-11-01, "От USB").** It is recorded as a deviation from M3-v1, as in milestones 5 and 5b. The long session is judged on pacing (S2: burst runs), and its thermal readings are not compared with the battery-powered baseline sessions. The short S1 cells keep the M3-v1 battery conditions as defined in milestone 6; this decision covers only the long session.

## Milestone 6 outcome: COMPLETE, 2026-11-01 (Claude Code session)

- **Authorization.** The user's `Приступай к шагу 6`. Milestone 7 is not authorized.
- **Start checks.**
  - Local AGENTS/PLAN/PROGRESS were byte-identical to `origin/mali-workspace` `baaedbf`.
  - Source `main` = `origin/main` = upstream main = `4bad6dc`, clean.
  - Releases at 18:47Z and 18:54Z: newest still `v0.2.10-beta` `4bad6dc`. The work was pinned to it.
- **Selection.**
  - **Selected: candidate (b),** vsync-phase-locked present pacing for the 50 ms bursts. The milestone 5b cause is confirmed causally. The judder is visible and can last for minutes. The mechanism is display-side and vendor-agnostic.
  - **Deferred: candidate (a),** the pipeline-cache save on start-screen return. It is small, but its impact is one rare 100-200 ms hitch. It remains a possible later, separate contribution.
- **Upstream overlap (18:47Z).** No PR in any state touches present pacing; the only PRs are #12, #96, #97 (framepace linking only) and #103. No issue reports the 50/16.7 judder; the maintainer treats #114 as thermal throttling. Public code search of the `mamcjr` repositories finds no `VK_GOOGLE_display_timing` or `Choreographer`. No recent present-path commit changes the grid phase. Recheck at milestone 7 start and before the PR.
- **Proposal summary** (full text, local: `.mali/milestone-6/M6_PROPOSAL.md`).
  - **Defect:** the `framepace_on()` grid in `nvnQueuePresentTexture` (`runtime/nvn/objects.c`) has an arbitrary phase against the display. The phase is set at each grid restart. When queueBuffer lands within about 1.5 ms before the latch, frames alternate 50/16.7 ms (60 Hz) or 41.7/25 ms (120 Hz).
  - **Intended behavior:** each frame is queued about r/2 before the next vsync (8.3 ms at 60 Hz, 4.2 ms at 120 Hz). Cadence, FPS, the game's clock and mean latency are otherwise unchanged.
  - **Mechanism:**
    - `VK_GOOGLE_display_timing` gives the refresh period and past `actualPresentTime` (CLOCK_MONOTONIC, the same clock as `plat_now_ns`). On Android the platform swapchain layer provides it, and r49p1 advertises it (M4 preflight `device-gpu.stdout.txt`).
    - Presents chain `VkPresentTimesInfoGOOGLE` with `desiredPresentTime = 0`, which keeps current scheduling.
    - The queue lead is measured just before `vkQueuePresentKHR`.
    - The pacer sets the phase at grid restarts. Otherwise it nudges by at most 0.5 ms a frame, only when the lead moves more than r/4 from r/2 (hysteresis).
    - Fallback to today's pacing when the extension or timings are missing or implausible, after swapchain recreation until new timings arrive, or with `LARECOMP_PACE_VSYNC=0`.
  - **Affected code:** about 70-110 lines in `runtime/nvn/vulkan/device.c`, the `framepace_on()` branch of `runtime/nvn/objects.c`, one header declaration, and possibly an `RT_ON_RESET` hook. The `g_vsync_count` path is untouched.
  - **Exclusions:** `desiredPresentTime` scheduling, AChoreographer, `setFrameRate` or panel-rate selection, present mode, swapchain image count, ADPF, the unexplained 120 Hz background, candidate (a), #174, #89, and any diagnostic code in the patch.
  - **Risks:** bogus timings (plausibility checks), clock domain (gate G0), oscillation (hysteresis and clamp), adaptive refresh or surface loss (reset and re-query), 60 FPS mode, movies and menus (smoke checks), other vendors (untestable here; stated as a limitation).
  - **Gate G0, first step of milestone 7 (about 20 min on the device):**
    - timings enabled without a pacer change;
    - `actualPresentTime` within ±0.2 ms of SurfaceFlinger actual present in ≥95% of frames;
    - refresh duration 16.67 / 8.33 ms;
    - no change in FPS, p99 or bursts.

    If G0 fails, stop and report. The alternatives (AChoreographer, or candidate (a)) need approval.
  - **Success criteria, defined before coding (milestone 8):**
    - **Diagnostic,** the M5B sweep on the candidate with M5B-D60/D120 conditions:
      - **P1:** 60 Hz 50/16.7 share ≤ 0.02 in every 150-frame block (baseline zone 0.11-0.47), with no run of ≥ 6 alternating intervals;
      - **P2:** at 120 Hz, no block above 0.16 (baseline peak 0.24-0.51);
      - **P3:** queue lead within r/2 ± r/4 for ≥ 99% of frames;
      - **P4:** `LARECOMP_PACE_VSYNC=0` reproduces the baseline zone;
      - **P5:** a Recompiled 60 Hz sweep meets P1.
    - **Performance,** M3-v1 conditions:
      - **S1:** S2 cold and warm pair in both modes. FPS within ±0.05 of the 0.2.10 cells, >100 ms count not above baseline + 1, p99 ≤ 33.6 ms;
      - **S2:** no burst run of ≥ 20 alternating intervals in any cell or in one Native long session with 3 menus and a re-boot;
      - **S3:** graceful teardown and re-boot, no crash or visual change; smoke checks of 60 FPS mode, a menu, the title screen and a movie.
  - **Estimates:** milestone 7 about 1.5-2.5 h (G0 about 20 min on the device). Milestone 8 about 3-3.5 h.
- **Evidence (local only).** `.mali/milestone-6/`: `M6_PROPOSAL.md`, `release-check.txt`, `upstream-prs.json`, `upstream-issues.json`, `code-search.txt`. No device action, source edit, build or upstream write.
- **Next step (needs explicit authorization; not started).** Milestone 7: `Приступай к шагу 7`. Implement only this proposal, gate G0 first, as an uncommitted patch.

## Milestone 5b outcome: COMPLETE, 2026-11-01 (Claude Code session)

- **Authorization.** The user's `Приступай к шагу 5b`. Milestone 6 is not authorized.
- **Release checks.** At 18:15Z and 18:39Z the newest release was still `v0.2.10-beta` = upstream main `4bad6dc`. The work was pinned to that SHA.
- **Code path.**
  - The pacer in `nvnQueuePresentTexture` (`runtime/nvn/objects.c`) sleeps the game thread with `clock_nanosleep` to a CLOCK_MONOTONIC grid of 1e9/30 = 33,333,333 ns.
  - After a frame more than one period late, the grid restarts at that frame.
  - The present then goes to the render thread (`async.c`) and to `vkQueuePresentKHR` in MAILBOX mode (`device.c`).
  - The app uses no Choreographer, `setFrameRate`, `VK_GOOGLE_display_timing` or present timestamps, so the grid's phase against the display's vsync is arbitrary.
  - ADPF and `timing_trace.c` only aggregate durations.
- **Hypothesis H1:** the grid's phase is set at each grid restart and then barely moves. When queueBuffer lands just before SurfaceFlinger's latch point, sub-millisecond jitter makes frames alternately miss and make the latch, which gives 50/16.7 ms.
- **Offline evidence** (existing M4/M5 SurfaceFlinger data: queue, ready and present time per frame).
  - In 2,724 blocks of 150 frames, a median queue lead of 0–1 ms before the vsync gave a burst share of 0.42 (90 of 99 blocks bursty). Any other lead gave 0.00–0.06.
  - Inside bursts the app's queue cadence stays 33.3 ms, and frames are GPU-ready 0.4–3 ms after queue.
  - The grid is longer than 2 vsyncs by only 17–39 ns a frame.
  - In LONG28-N-1 the lead held constant for minutes and changed only at long frames (map, menus). The 515-interval burst began when the third menu moved the lead to 1.7 ms.
- **Experiment** (diagnostic, not a performance run).
  - Temporary, env-gated instrumentation `LARECOMP_DIAG_PACE_SWEEP=500,150` moved the grid 0.5 ms later every 150 presents, sweeping the whole phase within the 180 s window. Patch and diagnostic APK `7db1d732…` are kept locally.
  - Conditions: Native, S1, warm cache, USB-powered (recorded deviation, as in milestone 5). Battery 28.8–29.0 C, thermal status 0.

  | Run | Panel | FPS / p99 | Alternation | Bursts where |
  |---|---|---|---|---|
  | M5B-D60 | 60 Hz | 29.997 / 50.0 ms | 50.0 / 16.7 ms | only at a queue lead of 0.4–1.4 ms (share 0.11–0.47), 0.00–0.01 elsewhere; the zone recurred at the same lead one full cycle later |
  | M5B-D120 | 120 Hz | 29.997 / 41.7 ms | 41.7 / 25.0 ms, no 50/16.7 | peak at a lead of 0.5–1.2 ms (0.24–0.51), twice per 16.7 ms of sweep; 0.01–0.16 background elsewhere |

  Both windows had valid timing and a graceful teardown.
- **Result.**
  - **Supported (causal at 60 Hz):** the bursts are a phase effect between the free-running 33.333 ms present grid and the display's latch. They occur when queueBuffer lands about −0.1 to +1.5 ms before the vsync, persist until the next grid restart, and follow the phase, not the content.
  - At random phase that is roughly a 10% chance per grid restart. At the device's default 120 Hz the same mechanism gives half the amplitude.
  - **Rejected:** GPU or render-thread overrun, ADPF or thermal causes, and grid drift alone.
  - **Not explained:** the phase-independent 1–16% background of 41.7/25 intervals at 120 Hz.
  - **Not measured:** Recompiled under the sweep, the exact latch offset, and `VK_GOOGLE_display_timing` support in the r49p1 driver.
- **Restoration.** Baseline APK `e4a62881…` reinstalled in place (hash verified on the device). Display settings restored (refresh keys null, brightness 88, auto mode 1). Source reverted to a clean `4bad6dc`; a rebuild gives an APK whose entries are all byte-identical to the baseline.
- **Evidence (local only).** `.mali/milestone-5b/`: `M5B_REPORT.md`, `m5b-results.json`, analysis scripts, the patch, the diagnostic APK, `runs/M5B-D60`, `runs/M5B-D120`, `driver/`, `setups/`, `cache-operations/`. Nothing was written upstream.
- **Next step (needs explicit authorization; not started).** Milestone 6 chooses one contribution from:
  - **(a)** the milestone 5 pipeline-cache save on start-screen return (small, low impact);
  - **(b)** phase-aware present pacing for the 50 ms bursts: align the grid to the display vsync with a fixed lead (AChoreographer), or use explicit present timestamps (`VK_GOOGLE_display_timing`, if the driver exposes it). Higher player impact, larger change; it needs an upstream overlap check and a measurable criterion such as no 50/16.7 runs at any phase in a sweep.

## Plan change: milestone 5b added, 2026-11-01 (approved; executed above)

- After milestone 5 the user asked what the 50 ms pacing bursts are and selected that symptom ("Выбираю этот симптом"). The session proposed a separate milestone 5b instead of going straight to milestone 6, and the user approved it ("Одобряю 5b").
- **Reason.** Milestone 5 showed that the compile-wait symptom is a pipeline-cache state effect with small player impact. The pacing bursts are a sustained, visible judder at a steady 30 FPS mean, and their cause is unknown, so milestone 6 could not select a supported fix for them without another bounded investigation.
- **Data recheck before the proposal (M4/migration evidence).** The bursts alternate 50.0 / 16.7 ms present intervals, one vsync late then one early. GP-R-S2-cold-1 had 906 of 3,600 intervals at 50 ms, GP-N-S1-warm-3 227, and GP210-N-S2-warm-1 53. All of this was at a 60 Hz panel.
- **Change.** Milestone 5b is defined in MALI_PLAN.md: scope, an estimate of about 2-2.5 h with 1-1.5 h on the device, and USB power allowed as a recorded deviation. Milestones 1-5 and their decisions are unchanged. Milestone 6 will now use evidence from both 5 and 5b.
- **Authorization.** Approval of the plan change does not authorize execution. Milestone 5b needs `Приступай к шагу 5b`.

## Milestone 5 outcome: COMPLETE, 2026-11-01 (Claude Code session)

- **Authorization.** The user's `Приступай к шагу 5`. Milestone 6 is not authorized. During the milestone the user asked for all communication in Russian, with the expected duration stated for every test run.
- **Release checks.** At 17:44Z and 18:07Z the newest release was still `v0.2.10-beta` = upstream main `4bad6dc`. The work was pinned to that SHA. Issue #174 gained a second reporter on a Poco F6 (an Adreno GPU), so that defect is not Mali-specific. The follow-up comment drafted at the 0.2.10 checkpoint now appears on #174, posted from the user's account at 17:42Z.
- **Symptom selected:** the small-batch pipeline-creation wait, candidate (b). The terrain defect was not chosen: upstream has it as #174 and it now reproduces on non-Mali hardware, so upstream can reproduce and fix it themselves.
- **Code paths.**
  - "waited" is the render thread blocked in `wait_built` (`runtime/nvn/vulkan/exec.c`).
  - The pipeline cache is saved on exit, on `WILL_ENTER_BACKGROUND` in `ui_run`, and by a 30 s check that fires only after more than 64 KB of growth (`device.c`).
  - The in-place return to the start screen (`main.c teardown` → `backend_reset` → `vk_exec_reset`) does not save. It keeps `g_vk.pcache` in memory and clears `pipe_tab`. The launcher loop has no background save.
- **Evidence before the experiment.**
  - The 0.2.5 warm cells were seeded from the prewarm bundle `warm-N-S2-v1`, which already contained the batch. The 0.2.8 and 0.2.10 warm cells were seeded from the cold cell's after-bundle, whose window ends before the batch.
  - The long sessions' seed `vk_pipelines.bin` equals the warm cell's save at t≈209.5 s, taken just before the batch at t≈213 s. The batch never reached disk.
- **Hypothesis H1:** the 0.2.5→0.2.8 increase is a pipeline-cache state effect, not slower pipeline creation.
- **Experiment.**
  - Diagnostic, not a performance run: Native, S2 warm route, USB-powered.
  - The helpers were copied with only serial and path changes; the charging and battery-level gates were relaxed and recorded as a deviation.
  - Panel 60 Hz and brightness 128 during the runs, then restored to the user's values (peak/min refresh null, brightness 88, auto mode 1).
  - Battery 27.4-28.1 C, thermal status 0 throughout.

  | Run | Cache state | Batch wait | Frame max at the batch | FPS / >50 ms / >100 ms |
  |---|---|---|---|---|
  | A: fresh process from the after-warm 0.2.10 bundle | SPIR-V present, batch not in the pipeline cache | 14 compiles, **120 ms** | 141 ms | 29.94 / 30 / 3 |
  | B: second boot in A's process | batch in the pipeline cache in memory | 14 compiles, **15 ms** | 43 ms | 29.97 / 15 / 1 |
  | C: fresh process from the bundle saved after A+B | batch on disk | 14 compiles, **15 ms** | 42 ms | 29.97 / 20 / 2 |

  A+B's 4113 KB save fired during B's boot, after 76 KB of growth that included A's batch. At A's teardown that growth existed only in memory. C was predicted to wait about 105 ms; that prediction was wrong because B's boot triggered the save. C instead shows that once the batch is on disk it costs 15 ms in a fresh process.
- **Result.**
  - **Supported:** the small-batch "regression" is a pipeline-cache state effect. With the batch cached, 0.2.10 waits 15 ms, within 0.2.5's 13-34 ms. There is no evidence of slower pipeline creation in 0.2.8 or later.
  - **Rejected:** "the 0.2.8 shader changes (`GMAP_MAX` 32, structured skips) made Mali compile this batch about 5× slower."
  - **Verified small defect:** pipelines compiled after the last periodic save are lost when the session ends with a start-screen return followed by a process kill, or with a kill or crash without a background transition. Growth under 64 KB, or growth within about 30 s of the end, is not saved. A normal exit through Home or recents saves through the background event. Player impact is an occasional single 100-200 ms hitch.
  - **Not measured:** the fully cold cost of the batch on 0.2.5 versus 0.2.10. On 0.2.10 it is 162-167 ms fully cold and 120 ms with SPIR-V only.
  - **Protocol lesson:** warm cells must be seeded from a bundle whose `vk_pipelines.bin` contains the route's late pipelines. The 0.2.8 and 0.2.10 warm-cell compile-wait comparison with 0.2.5 is invalid for this batch.
- **Evidence (local only).** `.mali/milestone-5/`: `M5_REPORT.md`, `m5-results.json`, scripts, `driver/`, `runs/M5-{A,B,C}`, `setups/`, `cache-operations/`. Source and the working tree are unchanged; nothing was written upstream.
- **Next step (needs explicit authorization; not started).** Milestone 6 has two candidates:
  - **(a)** save the pipeline cache when a session ends by start-screen return. A one-call change in the in-place reboot path, independently reviewable and Mali-agnostic, but with small player impact.
  - **(b)** choose a higher-impact symptom instead: the 50 ms pacing bursts at the 30 FPS cap, or #174 if upstream does not fix it in the next release. These need another milestone-5-type investigation first.

## Upstream migration phase 0.2.8-beta → 0.2.10-beta: COMPLETE, 2026-11-01 (Claude Code session)

- **Authorization.** The user answered "Migrate to 0.2.10" to the proposal at checkpoint `6a8678f`. The phase has the same limits as the 0.2.8 phase and does not authorize milestone 5. During the phase the user:
  - instructed skipping the recharge before the Recompiled long session;
  - approved filing an upstream issue for a new defect.
- **Release check and pin.** At 15:19Z the newest release was `v0.2.10-beta`, tag = upstream main = `4bad6dcb0390ae81d0e5d2e174809422d7198d97`. The campaign was pinned to it. A recheck at 17:36Z found no newer release and no newer main commit.
- **Branches.** Local `main` and fork `origin/main` were fast-forwarded `b02302c` → `4bad6dc` (5 commits, runtime only), with no rebase, force push or merge. The working tree is clean. `mali-workspace` stays documentation-only (`6a8678f` before this checkpoint).
- **Build.** `assembleDebug` exited 0 in 25 s with the unchanged environment.
  - APK `BOTWRecompAndroid-0.2.10-beta-4bad6dc-debug.apk`: SHA-256 `e4a62881a954423ea8626628ae4a13d4dc9f9aaf9787c8d64396732c2f6a2c50`, 98,277,296 bytes.
  - Package metadata: versionCode 30 / 0.2.10-beta, arm64-v8a, minSdk 30.
  - Signing: v2 signature, same debug signer `60fa2a5f…527a`, zipalign OK.
  - ABI `65da5d95e597ba58`, unchanged, so the game library `d9a8bd53…` was kept.
- **Install.** Over USB, in place (`install -r`) with no data loss. The installed `base.apk` equals `e4a62881…`, `firstInstallTime` was preserved, and the game library, saves and seeds are intact. Before the install, the USB-only ADB state left by the 0.2.8 restoration was confirmed (`service.adb.tcp.port` 0).
- **Bounded regression** (same conditions and gates as 0.2.8; long sessions without the teleport, so they compare with STAB-*-3):
  - **S2 cold pair** (start Δ 0.1 C):

    | Cell | FPS | p99 | >50 ms | >100 ms | Compiles |
    |---|---|---|---|---|---|
    | Recompiled | 29.98 | 33.6 ms | 19 | 1 | 1,841 |
    | Native | 29.97 | 33.5 ms | 21 | 1 | 1,867 |

    In both cells a large cold batch of about 550 pipelines waited about 3.4 s.
  - **S2 warm pair** (start Δ 0.3 C):

    | Cell | FPS | p99 | >50 ms | >100 ms |
    |---|---|---|---|---|
    | Recompiled | 29.93 | 33.4 ms | 15 | 2 |
    | Native | 29.93 | 50.0 ms | 50 | 3 |

    The fixed small warm batch (11-14 pipelines) waited 162-167 ms. It was 174-182 ms on 0.2.8 and 13-34 ms on 0.2.5.
  - All four short cells tore down gracefully. GPU memory peaked at 1,440 MB in each.
  - **Native teleport check (TP210-N-1).** The fast travel reached the HUD in 22.6 s. GPU memory after the load was 1,440 MB (0.2.8: 2,914 MB). The destination rendered correctly.
  - **LONG210-N-1** (start 25.4 C, battery 78%): complete, 29.85 FPS, p99 50.0 ms, 232 intervals >50 ms, 3 >100 ms, max 150 ms, 3 menus passed. Launcher return took 2.29 s, the re-boot reached the world in 26.4 s, and the second return was graceful.
  - **LONG210-R-1** (start 26.1 C, Δ 0.7 C from Native; battery 70%, **no recharge, user instruction**; recorded as a deviation). **Truncated:** Wi-Fi/internet dropped at about 17:27Z (user-confirmed), so wireless ADB was lost about 989 s into play, early in segment 4. The frame and sensor data up to that point are valid (no ring loss; `recomputed-stats.json`): 29,494 frames, 29.82 FPS, p99 50.0 ms, 4 >100 ms, max 150 ms. Segment 3 contains a 50 ms pacing burst of 1,398 intervals. All 3 menus passed. The game kept running while ADB was down. The session was recovered over USB and returned to start gracefully (teardown 239 ms); `recovery/` holds the notes and captures. The repeated boot was not done for this session.
  - **Thermal**, compared with the 0.2.5 sessions on the same route (no teleport):

    | | Max thermal status | Big-core median | SoC median |
    |---|---|---|---|
    | 0.2.10 Native | 0 | 52.1 C | 56.7 C |
    | 0.2.10 Recompiled (to ~989 s) | 0 | 56.6 C | 59.0 C |
    | 0.2.5 Native (STAB-N-3) | 2 (from 801 s) | 58.7 C | 62.9 C |
    | 0.2.5 Recompiled (STAB-R-3) | SEVERE stop at 1,187 s (status 2 from 690 s) | 63.7 C | 68.8 C |

    0.2.10 Recompiled had no thermal escalation by 989 s, where 0.2.5 had already reached status 2 at 690 s. GED clock and utilization are unchanged (546-572 MHz, 87-89%). This is consistent with the ADPF fixed-30-fps change (`b6725c5`) but not proven by an A/B test. The Recompiled session was truncated, so SEVERE at about 1,190 s is not excluded with certainty.
  - **#89.** GPU memory plateaued at 1,440 MB in every 0.2.10 run, against 2,658-2,914 MB on 0.2.5/0.2.8; this is the expected effect of BC1→ASTC (`596615e`). Upload is unchanged at about 928-930 MB per 2 s log line.
- **New defect: Native terrain/grass sinking.** The user noticed it live, and captures confirm it.
  - **What it looks like:** after about 5-10 min of the Native long session, grass disappears from the route field and the ground is drawn above Link's walking surface (knee-deep). It persisted through menus 2 and 3, by day and night.
  - **Where it does not occur:** it is absent at the same route point and session time on 0.2.5 (STAB-N-3, and STAB-R-3), and in 0.2.10 Recompiled at about 5, 10 and 25 min. Two-minute 0.2.10 cells look normal.
  - **Versions not checked:** 0.2.8/0.2.9 were not checked at this point and time.
  - **Cause:** not established. The candidates are `8053dc8` (linear textures checked every frame) or `596615e` (BC1→ASTC); it has not been bisected.
  - **Upstream report:** with the user's approval, it was filed as **mamcjr/BOTWRecompAndroid#174** from the user's GitHub account, via Claude in Chrome, with 3 screenshots (0.2.5 vs 0.2.10 comparison, close-up, night shot). Draft and record: `.mali/upstream-migration/issue-terrain/`. A follow-up comment with the Recompiled result is drafted, not posted.
- **Visual review.** The other 0.2.10 captures were inspected: boot, menus/inventory text and icons, rain, interior after teleport, launcher, end of the short cells. Nothing else was abnormal. The coarse look of the captures comes from the protocol settings (1x/1080p output, SGSR1 upscale, no AA). Full-resolution sky crops show no new banding in 0.2.5, 0.2.8 or 0.2.10.
- **Teardown hang.** Native graceful returns: 4 of 4 on 0.2.10 and 4 of 4 on 0.2.8, so 0 hangs in 8 since the 1 of 18 on 0.2.5. Recompiled: 3 of 3 on 0.2.10.
- **Restoration (17:35Z).** Refresh keys deleted; brightness 80 (pre-phase value) with auto mode 1 (reads 88 after adaptive adjustment); `adb usb`, after which `service.adb.tcp.port` reads 0 and only the USB transport is listed (`conditions-restore-0210.txt`). The app was left on its start screen.
- **Evidence (local only).**
  - `.mali/upstream-migration/` (ff-update-0210, build log, APK, signature, badging, install-0210, conditions-*-0210, TP210-N-1, issue-terrain);
  - `.mali/milestone-4-resume-1/{regression-0210, stability-0210, runs/GP210-*, cells/GP210-*, cache-operations/*GP210*}`;
  - the 0210 scripts. Manifest: `.mali/upstream-migration/migration-0210-manifest.json` (1,118 files, about 1.2 GiB, SHA-256 per file; it overlaps the 0.2.8 manifest for shared `upstream-migration/` files).
- **Next proposed step (needs explicit authorization; not started).** Milestone 5 on the pinned `v0.2.10-beta`, choosing one of:
  - **(a)** a bounded cause-finding experiment for the Native terrain defect (the `LARECOMP_BC1_ASTC=0` A/B, then 0.2.8 vs 0.2.9 vs 0.2.10 on the same route), feeding #174. Recommended only if upstream has not fixed it in the next release.
  - **(b)** the small-batch pipeline-creation wait regression on Mali.

  Releases are checked first, per policy. A process change to tiered per-release checks is proposed in MALI_PLAN.md and awaits approval.

## Upstream migration phase 0.2.5-beta → 0.2.8-beta: COMPLETE, 2026-11-01 (Claude Code session)

- **Authorization.** The user's pasted migration-phase instruction: fast-forward-only main update to the verified latest release, an unchanged rebuild and in-place install, a bounded device regression with read-only root, the rolling-upstream policy, and a three-document checkpoint. It does not authorize milestone 5. Before it started, milestone 4 was verified COMPLETE at documentation checkpoint `b37b036`.
- **Release check and pin.** At 2026-11-01T09:53Z the newest published release was `v0.2.8-beta` (published 2026-10-31T21:53:54Z, prerelease), tag commit `b02302c73fc2a0df7bd70b2e51a56d9d99af9421`, with no newer untagged commits. The campaign was pinned to that SHA. There are 24 commits from `3d66ac0`. Relevant ones:
  - #89/#131: GMap 32 slots and learned ranges, the upload ring returning chunks (`24beda8`, `17fae0d`), and arena space reuse with the oldest learned windows freed (`2a9360f`);
  - Mali host import with the arena off where the driver offers import (`db2451a`, r54p1+; likely not active on r49p1, not verified);
  - Mali render targets without MUTABLE_FORMAT (AFBC), and a guest yield that skips the host sched_yield (`57fe05b`);
  - structured shaders, a cached GMap lookup, and clears as load ops (`2fe2737`, `6681b37`).

  No commit touches the Native teardown wait.
- **Branches.** Local `main` and fork `origin/main` were fast-forwarded `3d66ac0` → `b02302c` with no rebase, force push or merge, and the working tree is clean. `mali-workspace` stayed documentation-only (`b37b036` before this checkpoint). Upstream was not written to.
- **Build.** `assembleDebug` exited 0 in 35 s with the unchanged verified 3a/3b environment.
  - APK `BOTWRecompAndroid-0.2.8-beta-b02302c-debug.apk`: SHA-256 `c6ab822e6cddeee685eff038eba8b9697d792967fa609b29fdba37256e1a3340`, 98,273,564 bytes.
  - Package metadata: versionCode 28 / 0.2.8-beta, arm64-v8a, minSdk 30.
  - Signing: v2 signature, debug signer certificate SHA-256 `60fa2a5f462fd95d30b37dde54d769de8986d7c8377e13c3ea5773088f20527a` (same signer as the installed build), zipalign OK.
  - `libmain.so` carries game-library ABI `65da5d95e597ba58`, unchanged from 0.2.5, so the Recompiled library `d9a8bd53…` was kept and not rebuilt.

  The 0.2.5 baseline APK `aabb4ea6…` is preserved.
- **Install.** In-place `install -r` with no uninstall, downgrade or data clear. The installed `base.apk` hash equals `c6ab822e…`. `firstInstallTime` was preserved, and the saves, seeds and game library are intact. Shader GLSL changed, so the app-managed caches recompiled; nothing was blindly deleted.
- **Bounded device regression.** Run on the SM-X730 / Immortalis-G925 MC12 (r49p1) under M3-v1 conditions: 30 FPS cap, 1080 output, panel fixed at 60 Hz, wireless ADB during measurements, and charge, temperature-stability and ±1 C start-pairing gates.
  - **S2 cold pair** (start Δ 0.1 C): Recompiled 30.01 FPS, p99 33.5 ms, 2 intervals >50 ms, 0 >100 ms; Native 30.00 FPS, p99 33.4 ms, 8 >50 ms, 0 >100 ms. About 1,850 pipeline compiles per cell. Graceful teardown in both.
  - **S2 warm pair** (start Δ 0.2 C): Recompiled 29.97 FPS, p99 33.4 ms, 1 >50 ms, one 200 ms interval; Native 29.96 FPS, p99 33.4 ms, 4 >50 ms, one 216.7 ms interval. Both hitches coincide with a fixed batch of 14 compiles waiting 174-182 ms; all six 0.2.5 warm cells waited 13-34 ms on the same batch.
  - **Long sessions, about 21 min each** (4×300 s S2-route segments; a teleport from the Great Plateau to the Shrine of Resurrection at the first boundary; menus at the second and third; launcher return plus repeated boot; 1 Hz read-only root GPU/thermal sampling). Native LONG28-N-1 started at 26.0 C; Recompiled LONG28-R-1 at 27.0 C (Δ 1.0 C, the pairing limit).

    | | LONG28-N-1 | LONG28-R-1 |
    |---|---|---|
    | Outcome | completed, no stop | completed, **no thermal stop** |
    | FPS / p99 / >50 ms / >100 ms / max | 29.81 / 50.0 / 689 / 5 / 533 ms | 29.81 / 50.0 / 269 / 6 / 567 ms |
    | Teleport (confirm→HUD; window max, at map open) | 23.4 s; 533 ms | 23.3 s; 567 ms |
    | Thermal status first 1 / 2 / 3 | 649 s / – / – | 701 / 1076 s / – |
    | Big-core median / max; SoC median; gpu0 median | 54.1 / 99.5; 57.9; 52.0 C | 58.7 / 94.4; 60.1; 54.8 C |
    | GED clock / utilization median | 546 MHz / 88% | 546 MHz / 87% |
    | Return / re-boot to world / return | 2.27 / 23.2 / 2.28 s | 2.29 / 27.3 / 2.28 s |

    The >100 ms intervals in each session fall in three places: one at about 168-170 s in segment 1, the maximum at the map opening (300.7-301.1 s) that starts the teleport, and two or three at the first menu (640-653 s). None occurs in later gameplay. The Native fourth segment has a 50 ms pacing burst of 515 intervals.
- **#89 observations.**
  - GPU memory reached a 2,658 MB plateau, rose to 2,914 MB after the teleport, and stayed bounded to the end (0.2.5: 2,914 MB from about 150 s). There was no unbounded growth in about 21 min.
  - Upload per 2 s log line was about 925-934 MB in both versions and both modes, unchanged from 0.2.5 (930-932 MB) on this route.
- **Visual review.** End captures of the four short cells, plus the boot, teleport arrival, menu, end-of-session and re-boot captures of both long sessions, were inspected. World, HUD, text, shadows, interior lighting and the inventory UI rendered correctly, with no visible corruption; transients are not excluded. Rain in the warm-cell captures is in-game weather.
- **Fixed, unresolved, new.**
  - **Not shown fixed:**
    - Native teardown hang: 0 of 4 Native returns on 0.2.8 against 1 of 18 on 0.2.5, too few to claim a fix; no commit addresses it.
    - Recompiled thermal: no SEVERE this time, maximum status 2. But segments 2-4 ran at the Shrine of Resurrection after the teleport, a different and probably lighter workload than STAB-R-3's open field, so this is **not** evidence of a fix.
  - **Unresolved:** 50 ms pacing bursts, and the #89 upload volume, unchanged.
  - **New:** a possible pipeline-creation slowdown. The 14-compile batch waits 126-182 ms in all 6 measured 0.2.8 runs (four short cells plus two long sessions), including after earlier runs had compiled it, which weakens the cold-driver-cache explanation. The cause is unverified.
  - No crashes. No new visual regressions.
- **Root and conditions.** `adb shell` runs as uid 0 (KernelSU allowlist, `u:r:ksu:s0`, SELinux Enforcing; no modules). Root use was read-only sysfs/procfs sampling only: `/proc/gpufreqv2`, GED `hal/*`, and thermal zones 2/17/31/32. The temporary display settings (60 Hz, brightness 128 manual) and wireless ADB were restored at 15:15Z: refresh keys deleted, brightness 78 with auto mode 1 (reads 75 after adaptive adjustment), and `adb usb` issued (`upstream-migration/conditions-restore-028.txt`). The USB transport was not re-verified because the cable was not attached.
- **Charging-control feasibility.** A separate read-only investigation requested by the user: `.mali/charging-feasibility/REPORT.md`. Slate mode, pass-through/bypass, and battery-protection hold do not reproduce battery-only operation, and `dumpsys battery unplug` is simulation only. The recommendation is a LAN-controlled AC-side switch for the charger. Nothing was written.
- **Evidence (local only).** `.mali/upstream-migration/` (releases, FF, build, APK, signature, install, conditions, teleport calibration), `.mali/milestone-4-resume-1/{regression-028,stability-028,cache-operations/*GP28*}`, and the 0.2.8 scripts. Manifest: `.mali/upstream-migration/migration-028-manifest.json` (1,172 files, about 1.15 GiB, SHA-256 per file).
- **Release check before this report.** At 2026-11-01T15:17Z (rechecked unchanged at 15:18Z before publication), **`v0.2.9-beta` (`b753dcd`, published 12:24:22Z) and `v0.2.10-beta` (`4bad6dcb0390ae81d0e5d2e174809422d7198d97`, published 15:14:53Z)** had appeared during the pinned campaign. They were not adopted mid-campaign, per policy. The 5 commits, all runtime-only (generator inputs untouched, so the ABI is probably unchanged; to be verified):
  - Mali BC1 textures as ASTC 4x4 instead of RGBA8, reported to roughly halve GPU memory (#143/#155/#162, related to #89) (`596615e`);
  - an ADPF 30 fps fixed-rate budget, lowering cluster clocks and power (`b6725c5`);
  - terrain "curtains" after a fast travel or tower (`8053dc8`);
  - two version bumps.
- **Next proposed step (needs explicit authorization; not started).** A bounded migration phase to `v0.2.10-beta` `4bad6dc`, using this phase's procedure:
  - FF-only main update, unchanged rebuild, in-place install, ABI check;
  - S2 cold/warm pair;
  - one long session per mode without the teleport (comparable with STAB-*-3), plus one teleport check.

  It would measure GPU memory under BC1→ASTC, the Recompiled big-core/thermal trend under the ADPF change, the 14-compile batch, and teardown. After that, milestone 5 would take one symptom on the newest baseline. The alternative is to authorize milestone 5 now on 0.2.8, which is not recommended.

## Milestone 4 outcome: COMPLETE, 2026-11-01 (Claude Code session)

- The user instructed "Finish step 4". The local final report is `.mali/milestone-4-resume-1/M4_FINAL_REPORT.md`; its private manifest is `session-2026-11-01-final-manifest.json` (303 files) alongside the earlier 2426-file session manifest. Source is still `main` 3d66ac0 and clean; no source, test or app change; no upstream write.
- **Stability sessions** (M3-v1: about 20 min warm, S2 route, menu every 5 min, launcher return plus repeated boot). The user approved 1 Hz read-only root GPU/thermal sampling for these sessions only, labelled separately from the matrix.
  - **Native STAB-N-3 completed**: 29.86 FPS, p99 50.0 ms, 231 intervals >50 ms, 0 >100 ms. All 3 menu transitions passed; launcher return took 2.2 s, the repeated boot reached the world in 24.7 s, and the second return took 2.2 s. Thermal status peaked at 2.
  - **Recompiled reached Android thermal status SEVERE, so the protocol guard stopped it.** This happened at 802 s in STAB-R-1 (started 2.6 C warmer; pairing failed) and, after the user chose a matched re-run, at **1187 s in STAB-R-3 (start within 0.5 C of Native)**, so 2 of 2 sessions.
  - **Matched comparison.** GPU GED clock (≈546-598 MHz), utilization (86-89%) and the gpu0 sensor are similar in both modes. The difference is the big CPU cluster: about 100 C sustained from about 7.5 min in Recompiled, against a single 94 C spike in Native. SoC median was 68.8 vs 62.9 C. Battery heating was not higher in Recompiled. Recompiled pacing was smoother: p99 33.6 ms and 116 intervals >50 ms. The cause is not established.
  - Automation-caused aborted attempts are retained with notes and are not app failures. STAB-N-1 and STAB-N-2 were false-negative menu checks; screenshots show the menu working. STAB-R-2 was a PLAY tap the launcher did not register. Fixes: a reference-panel menu check made non-fatal, and PLAY retried until the `[loader]` log line appears. The protocol is unchanged.
- **Symptom summary.**
  - Native teardown hang: 1 of 18 Native graceful-return attempts, 0 of 15 Recompiled.
  - Recompiled sustained-load thermal escalation to SEVERE: 2 of 2 sessions, 0 of 1 Native.
  - 50 ms pacing bursts at the 30 FPS cap: 7 of 24 short windows (4 Native, 2 Recompiled) and also in stability segments; no throughput loss.
  - Gameplay crashes: 0. Visible corruption: 0 in end and menu captures; transients not excluded.
  - Cold versus warm shader-cache stalls are not distinguishable at this cap: no cold window has a >100 ms interval, and the driver cache is uncontrolled.
- **Restoration.** Display settings were restored from `frame-preflight/system-settings-before.json`: refresh keys deleted (panel back to 120 Hz), brightness 87, auto mode 1; record in `restoration/`. Brightness read **0** (not the protocol's 128) just before restoration, and when it changed is unknown; recorded as an unexplained deviation. With the user's cable connected, ADB was then switched back to USB-only (`adb usb`): `service.adb.tcp.port` reads 0 and a TCP connect to 5555 is refused (`restoration/adb-usb-restore.txt`). All restoration is complete.
- **Next action requiring authorization (proposal only, no plan change yet).** Before milestone 5, a bounded synchronization of source/main to upstream 0.2.6-beta `2a1bf32`, a rebuild with the verified 3a/3b environment, and a short device check that includes a teleport/load. Reason: `24beda8`/`17fae0d` address the #89 copy-path upload blowup on G925 and could make a milestone-5 investigation duplicate upstream work. The alternative is to authorize milestone 5 directly on 3d66ac0 with one of the measured symptoms. Either option needs explicit approval; nothing is authorized automatically.

## Controlled matrix completed, 2026-11-01 (Claude Code handoff session)

(The "Remaining in milestone 4" bullet at the end of this section is superseded by the outcome above.)

- Handoff: a Claude Code session took over from Codex. It verified that local AGENTS/PLAN/PROGRESS were byte-identical to `origin/mali-workspace` b9dda86 and that source `main` was at 3d66ac0 and clean. The user then said "Continue"; this session treated that as continuation of the already authorized milestone 4 only.
- Automation v3 hashes matched campaign-tool-manifest-v3. The failure/cleanup path was exercised in isolation with a deliberately failing helper (`validate-cleanup-v3.py`, `cleanup-v3-validation/`): bounded evidence was collected in 2.3 s, force-stop exited 0, and no PID remained. Limitation: the app was not running, so this was not tested against a live hang.
- Coordinators `resume-matrix-v3.py` and then `resume-matrix-v4.py` reuse the unchanged v3 cell/pair scripts and the v2 boundary/visibility/temperature/battery-reserve checks. Charge was conditioned to 78 percent and the user unplugged at 79 percent. S1-warm-2 was re-collected as **GP-N-S1-warm-2-resume** + GP-R-S1-warm-2, following the GP-N-S2-warm-1-resume precedent. The original GP-N-S1-warm-2 window remains teardown-hang evidence. That teardown hang did not recur: all 14 new cells logged graceful teardown.
- **S2-warm-2 failed the <=1 C pairing at 1.7 C** (N 23.0 / R 24.7). The battery log shows an external drop of about 5.6 C in 10 minutes, idle and screen on, around 23:30 local; the user has not explained it. Per the user's choice the pair was kept unchanged and **re-run as S2-warm-2-retry (0.2 C, pass)**; no waiver was granted. v4 adds a pre-pair gate: three battery readings 60 s apart within 0.3 C, at most 30 min. No criterion was relaxed.
- **All 24 planned windows are now collected**: rep1 8, rep2 8 (including S1-warm-2-resume and S2-warm-2-retry), rep3 8. Each window has 3599-3601 actual-present frames over 120 s, valid percentiles, no >100 ms interval, and graceful teardown. Start-temperature pairing passed in every pair except S1-cold-1 (1.4 C, user waiver) and the superseded S2-warm-2 (1.7 C). S2-warm-1 has no pair summary file; its computed difference is 0.1 C. All end captures were visually reviewed: exit clearing, full health, no visible corruption (`review-v3/`). End captures do not exclude transient events.
- Observation, not yet analysed: all windows hold the 30 FPS cap in mean, but 7/24 show bursts of 35-749 intervals >50 ms with p99 about 50 ms. That is cadence irregularity with compensating short intervals, not throughput loss. Native: S1-cold-1, S1-warm-2-resume, S1-warm-3, S2-warm-3. Recompiled: S2-cold-1, S2-cold-2. No cause is claimed; frequency, mode association and significance belong to analysis or milestone 5. Per-window stats are in `review-v3/matrix-24-stats.json`.
- **Root/privilege finding (read-only, `root-readonly/`):** the device runs KernelSU with the `shell` user allowlisted. Every `adb shell` command, including all earlier M4 "unprivileged" collection, ran as uid 0 in SELinux context `u:r:ksu:s0`. SELinux is Enforcing, no KernelSU modules are installed, `service.d` is empty, and no ksud/sulogd process was running at check time. `resetprop` is present, so boot-state props (green/locked) may be spoofed and are reported as observed only. Useful read-only GPU sources: `/sys/class/misc/mali0/device/gpuinfo` (Mali-G925-Immortalis 12 cores r0p1, core mask 0xEE0077, power policy coarse_demand), `/proc/gpufreqv2/gpufreq_status` (OPP/frequency, PPM ceiling/floor), and GED `/sys/kernel/ged/hal/{current_freqency,gpu_utilization,gpu_fps}`. SoC/CPU thermal zones read normally, while gpu0/gpu1/vtskin read -274 at idle. `/sys/kernel/gpu` reports a stale "Mali-G57 MC2" and 702 MHz max and must not be used. A Samsung hyper-service sets the GED custom boost. Hardware performance counters were not verified. Nothing was written to system state.
- Upstream moved to **0.2.6-beta `2a1bf32`**, 15 commits ahead; it was fetched only, and source and baseline are unchanged. Relevant to this device's copy path (Mali r49p1 without external_memory_host): `24beda8` and `17fae0d` fix learned-range GMap overflow after loads, which caused a 15 GB / 2 s upload blowup and 6 fps on a G925 (#89), and the transient-ring growth (#131). `db2451a` (Mali host import/arena off) likely does not apply to r49p1. `2651d3e`/`5cc248d` change command-recording cost generally. None touch the teardown wait. The matrix stays on 3d66ac0. The post-milestone-4 proposal (requires approval) is a bounded sync to 0.2.6 plus a rebuild and a short check including a teleport/load before milestone 5, to avoid duplicating the #89 fix.
- Private manifest: `.mali/milestone-4-resume-1/session-2026-11-01-manifest.json` (2426 files). Device state: app stopped, unplugged, 51 percent, 25.7 C. Display settings and TCP ADB are not yet restored.
- **Remaining in milestone 4:** charge conditioning, then the two 20-minute warm stability sessions (one per mode). Optional root GPU sampling there needs the user's approval because it changes the protocol. Then final analysis, restoration of display settings and the ADB transport, and the final checkpoint. Milestone 4 is not complete; milestone 5 is not authorized.

## Checkpoint branch-name migration, 2026-10-31 (maintenance only)

- The user explicitly authorized this one-time repository-maintenance task: officially rename `Propinbo/BOTWRecompAndroid:codex/mali-workspace` to `mali-workspace`, migrate active documentation/recovery references, and publish only AGENTS.md, MALI_PLAN.md, and MALI_PROGRESS.md. This task does not resume milestone 4, authorize milestone 5, or change any completed milestone, historical decision, evidence, application source, or baseline.
- Re-read the live remote immediately before renaming; the old branch existed at **`06d4fca12dec98cd64d1ef47eeb6a0d5ff5606b9`**, the new name was available, and default branch remained `main`. Used GitHub's official `POST /repos/Propinbo/BOTWRecompAndroid/branches/codex%2Fmali-workspace/rename` with `new_name=mali-workspace`; HTTP 201 returned the new branch at the **same SHA**. No remote branch deletion/recreation was used. The immediate branch-list read briefly lagged; subsequent REST branch listing and Git advertised heads agreed that only the new checkpoint name is active. No rename retry was performed.
- Current publication/recovery target: https://github.com/Propinbo/BOTWRecompAndroid/tree/mali-workspace and `origin/mali-workspace`. Updated active references in these three documents. Former names in historical publications, agreements and the explicitly labelled original prompt remain historical; they are not active recovery destinations. The final containing documentation SHA must be obtained from `git rev-parse origin/mali-workspace` after fetch or the maintenance task's final report; a document cannot contain its own commit hash.
- The source checkout remains on `main` at `3d66ac0482ae9d99546a579170675dd6eb3c047c`, with its index, tracked source, local exclusions and untracked guidance preserved. In the existing separate `BOTWRecompAndroid-checkpoint` clone, renamed the local branch, changed its single-branch fetch refspec and upstream to `mali-workspace`, and fast-forwarded the stale checkpoint through existing documentation commits only. Removed the obsolete `origin/codex/mali-workspace` remote-tracking ref from these two active repositories using the observed old values. No checkpoint history was merged into main or a contribution branch.
- Reviewed local recovery scripts, repository refs/worktrees, project workflows, current milestone-4 orchestration, Codex automation files and Windows scheduled tasks. No project workflow/current milestone-4 script uses the former name; no matching scheduled task or local Codex automation file was found. Six `finalize-checkpoint` scripts under `.mali/milestone-3/`, `milestone-3a/`, `milestone-3b/`, `milestone-3b-resume-1/`, `upstream-sync-025/` and `milestone-4/` are historical, stage-specific scripts with fixed old SHAs, not reusable recovery automation. Preserve them unchanged and do not run them for current recovery. Two historical verification clones under `.mali/workflow-persistence/retrieval/` and `retrieval-final/` retain former branch/config/ref names as raw evidence. Their archived configs/refs, old JSON/Markdown snapshots, Git reflogs and private backup archives are intentionally not rewritten. These are retained historical references, not unresolved active dependencies.
- Verified independent recovery in a new empty Git repository: fetched `refs/heads/mali-workspace` from the fork, restored only the three documentation paths with an empty index, and matched each blob to the renamed remote commit. After publishing the first migration-documentation commit **`5343ece4b3ec8fc1799f1824b8f66767b326564a`**, repeated recovery in a second new empty Git repository: all three remote contents matched, active recovery references used the new name, and the application tree was unchanged from the pre-rename checkpoint. No application source checkout or original workspace data was needed. This completion record is a subsequent documentation-only checkpoint; its exact containing SHA and final readback are recorded in the maintenance task's final report and local receipts under `.mali/branch-rename-2026-10-31/`. This tests Git retrieval in a fresh environment, not an application/OS restart or persistence of private raw evidence.
- Independently rehashed all **4,384** prior manifested files other than the three authorized documentation updates: every size and SHA-256 matched, with no missing file or mismatch. Preserve the old manifest and raw evidence unchanged; update only the current maintenance manifest/document hashes and remote checkpoint pointer locally.
- Archived the pre-maintenance documents, manifest and Git configuration locally. Publication is limited to the three reviewed documents and a normal fast-forward of the renamed branch; preserve concurrent remote/local changes and reconcile before any retry. No benchmark, device action, application build/source edit, milestone execution, upstream write, PR, force-push or default-branch change belongs to this task. Stop after maintenance; milestone 4 remains IN PROGRESS and milestone 5 remains Not started.

## Authorization and status

Current authorization (2026-11-01): nothing is currently authorized. Milestones 4, 5, 5b, 6 and 7 and the upstream migration phases 0.2.5→0.2.8 and 0.2.8→0.2.10 are COMPLETE. The candidate patch is uncommitted in the source working tree. Milestone 8 (device validation) needs `Приступай к шагу 8`. Earlier status: milestone 4 was explicitly authorized. After the initial data-blocker checkpoint, the user instructed doing everything needed to prepare the game and supplied its PC folder. This explicitly approves the preparation addendum now recorded in MALI_PLAN.md. The data blocker is resolved: the user-supplied game/update/DLC were extracted, verified and transferred; Native boots 1.9.0 with DLC 3.0 and loads a scratch copy of the supplied save. The user then explicitly selected preparation of Great Plateau and Recompiled for the full original comparison. That approved addendum is recorded in MALI_PLAN.md; no Native-only alternative or shortened check is selected. Milestone 4 is COMPLETE (see the outcome section): 24 matrix windows, the stability sessions and the final analysis are done, and device restoration is complete; do not treat setup gameplay or a preflight sanity check as a completed baseline. Milestones 1-3/3a/3b and synchronization remain complete. Milestones 5-9, source/test fixes and source publication remain unauthorized.

| Milestone | Status |
| --- | --- |
| 1. Prepare the fork, workspace, and checkpoints | COMPLETE; preserved and reverified |
| 2. Triage the reports against current upstream | COMPLETE; stopped after triage |
| 3. Build an unchanged baseline and prepare the measurement protocol | COMPLETE via original blocker alternative; investigation preserved, APK gate now satisfied by 3b |
| 3a. Prepare and verify the pinned build environment | COMPLETE; exact toolchain/submodules verified, Gradle configuration passed |
| Bounded upstream synchronization before 3b | COMPLETE; source/main fast-forwarded to 0.2.5-beta, 3a preserved |
| 3b. Build native dependencies and the unchanged baseline APK | COMPLETE; full baseline APK/hash/signature verified, known host-test harness limitation retained |
| 4. Collect reproducible baseline evidence from your physical device | COMPLETE; 24-window matrix, stability sessions (Recompiled thermal SEVERE 2/2, Native teardown hang 1/18), final report local; device settings and ADB transport restored |
| Upstream migration phase 0.2.5→0.2.8 (between 4 and 5) | COMPLETE; main FF to b02302c, APK c6ab822e installed in place, bounded regression done, policy added; newer 0.2.9/0.2.10 released during the campaign |
| Upstream migration phase 0.2.8→0.2.10 | COMPLETE; main FF to 4bad6dc, APK e4a62881 installed in place, regression done (Recompiled long session truncated by a Wi-Fi outage), GPU memory halved, no thermal escalation, new Native terrain defect filed as mamcjr/BOTWRecompAndroid#174 |
| 5. Investigate one symptom with a bounded diagnostic experiment | COMPLETE; the small-batch compile-wait "regression" is a pipeline-cache state effect (15 ms with the batch cached vs 120 ms without, on 0.2.10); a small unsaved-session-tail defect is verified |
| 5b. Investigate the 50 ms pacing bursts at the 30 FPS cap | Added 2026-11-01 with user approval; not started; awaiting `Приступай к шагу 5b` |
| 6. Select one narrowly scoped contribution | Not started; awaiting explicit authorization |
| 7. Implement the selected fix and perform local verification | Not started; awaiting explicit authorization |
| 8. Validate the candidate on your device | Not started; awaiting explicit authorization |
| 9. Prepare a reviewable upstream PR package locally | Not started; awaiting explicit authorization |

The user-approved revision adds 3a and 3b after the completed original milestone-3 definition and makes milestone 4 depend on a usable APK from 3b. Original milestones 1-9, completed investigations and historical decisions are preserved. Reason: milestone 3 found incompatible JBR 25 plus missing pinned NDK/CMake/submodule/native dependencies; substantial preparation requires bounded steps. Before this revision all 72 entries of the milestone-3 manifest matched SHA-256/size. Pre-revision AGENTS/plan/progress/manifest are archived under `.mali/milestone-3a/`; milestone-3 evidence itself remains unchanged under `.mali/milestone-3/`. Earlier historical checkpoints remain in `.mali/milestone-1/` and `.mali/workflow-persistence/`. These raw records are local, not published.

## Confirmed Native teardown hang, 2026-10-31 (local live update)

User reported a frozen game. The coordinator had already stopped on GP-N-S1-warm-2 helper failure; no measurement/helper processes were still running, but the app itself remained alive. Its120s S1/warm Native gameplay window had completed3600 actual-present frames. During the subsequent requested launcher return, the log repeatedly reported waiting for two guest SDK threads and never logged game teardown completion. The automation's nonzero-exit path had left the hanging process running for roughly half an hour before this user-triggered audit: that is a failure of the bounded-cleanup workflow, not compliant freeze handling.

Preserved the live state before intervention. A normal PLUS hold produced no visible response across30seconds; before/after screenshots are byte-identical, and the exact SurfaceFlinger layer returned no new actual-present timestamps (empty histories in this observation, not a new valid frame-time dataset). Logs continued waiting for the same two threads. This confirms a visible teardown hang, not a root-cause diagnosis. Package was then explicitly operator-force-stopped; PID absence verified. Private evidence: current-freeze-audit-path.txt and its freeze-observation directory under `.mali/milestone-4-resume-1/`.

Eleven full gameplay-window records now exist; they must not be described as eleven fully successful runs, because this last teardown failed. No new gameplay crash, rendering-corruption cause, or source fix is established. Long stability sessions and the rest of the requested matrix remain incomplete. Step4 remains in progress; no automatic step5.

Preserved/hash-verified automationv2 before preparing v3. V3 polls teardown completion for up to30s rather than an immediate2s check, and on a nonzero helper exit preserves bounded failure evidence then attempts force-stop of only the test package. This is excluded test orchestration, not an app/runtime patch. Syntax checked; not yet verified through a new in-game failure, and helper timeout handling is not claimed fully validated. Battery was43percent at audit, below protocol start minimum. User reconnected charging; no automatic restart of tests. Future measurements require charge conditioning, disconnection and cooling first.

## Guarded resumption verified (local live update)

GP-N-S2-warm-1-resume completed120seconds/3600 actual-present frames with no ring loss, explicit Awake/unlocked checks before and after, and graceful teardown. End capture reviewed: same rainy clearing, full health, no visible corruption. This completes the eighth collected logical cell; first7 retain v1 evidence. The interrupted S2/warm first pair spans v1/v2 rest display conditions and remains explicitly qualified in its resumed summary.

Remaining repetitions2/3 are resumed through matrix-resume-v2, with fail-closed screen checks, full frame-history checks, automatic end-game visibility screening and per-pair temperature checks. Automated visibility screening is not human visual review. The coordinator stops before a new pair below53percent battery to leave reserve; it does not charge/waive conditions automatically. All long stability sessions, final analysis, system restoration and final checkpoint remain pending; do not call milestone4 complete.

## Continuation after screen-lock audit (local live update)

User instructed continuation. Original six automation scripts were hash-verified against v1 and archived before changes. V2 adds fail-closed Awake/keyguard=false boundary verification and an end-state record, with no additional queries during the measured window. Live device Awake/unlocked was accepted; synthetic asleep/locked/missing-field cases rejected. V2 also removes forced screen sleep during app-stopped300s rest, keeping fixed display settings and all original thermal/power/battery checks. This changes idle display power relative to v1 and must remain explicit in comparisons; it is an operational adjustment within milestone4, not a runtime/source fix.

Interrupted GP-N-S2-warm-1 preparation is preserved. Only its replacement GP-N-S2-warm-1-resume is now running; old coordinator remains stopped. Confirm new boundary/capture evidence before further queueing. First7 collected windows retain their v1 provenance; no retrospective screen-state guarantee. Step4 remains in progress.

## Screen-lock concern audit, 2026-10-31 (local live update)

After the user questioned screen locking, the operator stopped the background coordinator and its active helper processes. Seven120-second windows have complete actual-present histories and end captures of gameplay, all visually reviewed; this is not final acceptance of every protocol criterion. Sleep was issued only for the app-stopped cooling interval; the launch helper sent Wakeup before boot. Automationv1 did not independently assert awake/unlocked state at the measurement boundary. Current power/window snapshots show Awake and no keyguard, but are not retrospective proof for every instant. No evidence presently supports saying most windows failed because of a lock screen.

GP-N-S2-warm-1 was interrupted during cooling, before a gameplay window; label this operator interruption, not a crash. Queue is STOPPED, not still running as the earlier live record described. Before resuming, require explicit awake/keyguard verification and recorded automation provenance; retain original v1 evidence. Audit/contact sheet under `.mali/milestone-4-resume-1/screen-lock-audit/`. Step4 remains incomplete; no source fix or step5 authorized.

## Resumed controlled collection, 2026-10-31 (in progress)

Wireless access restored after the long interruption; external power was actually absent, battery70percent. First S1/app-cold R and N windows each collected3600 actual-present buffers across120seconds, with no ring loss or operator-induced termination inside the window; graceful teardown confirmed for each. End captures show the expected clearing, full health and no visible corruption. This does not exclude transient corruption or establish long stability.

Actual R/N start battery temperatures20.1/21.5C: original <=1C pairing check failed at1.4C. User explained that the tablet had been brought indoors from outside and explicitly instructed accepting this first pair. MALI_PLAN.md now records this specific approved exception; raw numeric values and original failure are preserved. No waiver applies to later pairs. The initial Native cadence has longer present intervals despite the same capped mean; do not infer cause or comparative superiority from one excepted pair.

The S1/warm first pair is active; a local coordinator waits for it and then executes the remaining planned matrix pairs sequentially, stopping on any helper error. Raw state is under `.mali/milestone-4-resume-1/matrix-continuation/`; completed captures still require review and temperature checks. Two20min stability sessions and final analysis/restoration/checkpoint remain pending. Step4 is not complete; no step5 or source changes authorized. The published preparation checkpoint7e68401 and verified1939-entry private backup remain the preceding immutable recovery point; this new collection is not yet in that archive or remote summary.

## Continued milestone 4 preparation (intermediate checkpoint; baseline result pending)

- The user approved all necessary preparation and supplied their own PC containers. All 359 previous manifested files matched before edits. Source/main remains 3d66ac0482ae9d99546a579170675dd6eb3c047c, previous published documentation db718c198dd84969e28664c939d5056cc6f551fb; tracked application/test sources are unchanged.
- Isolated NSZ 5.0.0 source at 78a12fe652a1f5ea4a7d8d1ca1d3dab80df1b377 with a private virtual environment decompressed the supplied containers. The verified publisher-hashed Windows NSZ executable failed to load its bundled Python DLL; unchanged official source succeeded instead. Hactool 1.4.0 reads the base but rejected the update BKTR layout; NSTool 1.9.2 (source e89ea84bd70c11b67ebec9153fb4a446f18a9d3d) extracted the patched game using the base. Existing local user key material only; no externally acquired keys/assets, no key values in reports, temporary title lookup removed, original key file preserved.
- Unchanged tools/botw_dump.py passed after installing its LZ4 dependency in that isolated environment: main build ID cd57b23fa4bbad65803d9788c01821ee, actual game 1.9.0. Full patched RomFS has 41062 files. DLC 3.0 full data contains 7094 files; the second DLC unlock package was extracted and accounted for. Original four PC containers (11450125732 bytes total) remain unchanged.
- Built unchanged android/tools/romfs_tool.c + runtime/romfs_build.c as a host data utility under the preserved WSL environment, with no app rebuild/source edit. Independently checked table bounds, all paths/sizes and every source file hash against the two generated images. Game image: 15132505364 bytes, SHA-256 c298120aa63203709f5c6da4311f6a0adff6d1be976f003674b3606d45d82948. DLC image: 1390370680 bytes, SHA-256 85bccaae458702bb9130813929cc87a945e31f1a044c735a67a4330049bf2066. Deployment contains 43 files/18069497724 bytes (images, ExeFS and necessary loose movies), and all 43 hashes matched after transfer to new app-owned device folders. No existing game/save data was replaced.
- Supplied save ZIP copied into a 226-file immutable seed candidate; original ZIP preserved. Each setup session receives a separate app-owned scratch copy and settings/log paths. Native title shows 1.9.0/DLC 3.0 and the scratch save loads at Tabantha Frontier. A matching Recompiled library was built from this verified 1.9.0 ExeFS through unchanged gen.py and android/gamelib CMake with the pinned NDK/CMake; generation and compilation completed (632 C files, no heavy-file overrides). Host output is ELF64 AArch64, 126226352 bytes, SHA-256 d9a8bd53da03285cf69e6ff0e58d08e5eddf8023e9455ac0b943c653fa57f0c2, matching runtime ABI 65da5d95e597ba58. Both transferred library/ID hashes match; Recompiled loaded 125144 main + 40516 SDK functions and reached title. Full original seed was used in a dedicated Great Plateau scratch: actual travel to Shrine of Resurrection, walking through the exit and a manual save produced a separate 205-file slot-3 seed (all device hashes verified). Both modes reloaded that seed in game. The +64-pixel camera hold is visually near a full turn in 30 seconds in each mode (no exact angular measurement). A direct-forward route fell from a ledge in setup and was rejected; the selected short right/left walk on the exit clearing was verified in Native at full health. Native S1/S2 full 120-second prewarm passes completed with continuous actual-present data and graceful teardown; their 879-file cache bundles were copied and independently hash-verified. Recompiled S1/S2 prewarm also completed, each with a verified 879-file bundle. All four full-scene warmups and their graceful returns are retained separately. All these runs remain outside baseline counts. Do not invent a paired mode result.
- First task-generated settings file used CRLF; several enum/string settings were read differently by the existing Android parser (auto FPS/FSR1/SMAA). Preserved that setup evidence; corrected the generated file to LF and started fresh scratch session setup-native-2. Device readback confirms 30 FPS, original 16:9, 1x, 1080 output, SGSR1, AA off, Native. These setup launches are not controlled performance runs. Operator force-stops are labelled, not crashes.
- User reports room around 20 C, confirms case removed and no active cooling. Recorded prior display settings and temporarily applied 60 Hz/50 percent manual brightness; actual SurfaceFlinger refresh period changed to 16666667 ns. Restore prior settings after work. Authorized ADB transport was moved temporarily to the local wireless connection; user disconnected USB, and battery service confirms USB power false. Restore the temporary ADB listener/USB transport after work when practical.
- Perfetto trace processor v59.1-1e9c61e98 prepared. Actual trace has no game SurfaceView rows; compositor frames must not be substituted. Exact game BLAST-layer SurfaceFlinger latency output does provide actual-present timestamps (column 2). Adjacent one-second samples overlap. The completed unplugged on/off sanity check reports stock 30.0/30.0 FPS, no difference at the cap, max query 0.171 s and overlapping rings throughout (934 unique actual-present timestamps); this does not establish zero overhead or spare headroom. The first on/off test overlapped USB removal and is retained only as capability evidence. Raw captures, SQL/commands and limitations remain local.
- Local M4 execution details fix the practical route, per-mode/scene warm-cache bundles, actual input timing and finite first-visible-world detection/ADB reaction latency before formal data. No exact first-rendered-frame or camera-angle claim is made; normal game-time/weather advance is retained, with the same seed and elapsed schedule within each paired cache category. Matrix remains 24 windows plus two 20-minute stability sessions.
- Current local evidence: `.mali/milestone-4-resume-1/`; private reproducible game outputs/tools are separately excluded under `.mali/game-preparation/` and `.mali/tools/`. Old milestone-4 blocker history below remains intact. After preparation the charge reached 60 percent; the user reconnected charging for conditioning to 80 percent before the long matrix (AC power observed, not assumed from USB alone). No controlled S1/S2 window or bounded stability run is complete yet, and no symptom frequency/paired performance claim is made. Resume using WORK_STATE.json and real records, not this live status as completion evidence. No milestone 5 or source fix is authorized.

## Milestone 4 initial data-blocker outcome (historical; resolved by preparation above)

- Explicit milestone-4 authorization received on 2026-10-31. Source remains **main at 3d66ac0482ae9d99546a579170675dd6eb3c047c**; tracked source, index and gitlinks are unchanged. Remote documentation checkpoint **5cf0b1e9a68213460fbb1336ed29aa3541e9656d** was verified; all **236** prior manifest entries matched size/SHA-256. Archived pre-stage guidance/manifest under `.mali/milestone-4/`; completed evidence and M3-v1 are preserved, not repeated.
- Actual authorized ADB access succeeded with platform-tools 37.0.1. Device reports **Samsung SM-X730**, SoC **MediaTek MT6991**, Android **16 / API 36**, security patch 2026-09-05, arm64-v8a. Vulkan system JSON and the app's own startup log independently report **Mali-G925-Immortalis MC12**, Vulkan **1.3.278**, Arm driver ID 9, driver info **v1.r49p1-03bet0.f8a98b506b89b21c80a508d70abfcb3f**. These are observed device fields; no unreported commercial SoC variant is inferred. Fingerprint, serial and full private system output stay local.
- No pre-existing `com.botwrecomp.android` package or application data root was present. Installed the preserved full debug baseline without uninstall, replacement or data clearing; ADB install exit 0 / Success. **Installed base.apk SHA-256 equals aabb4ea6f8c86276092a2ed8043c15bfedf60da643f77173c59b617b979f32c8**, matching the 3b artifact exactly, including its previously verified signer **60fa2a5f462fd95d30b37dde54d769de8986d7c8377e13c3ea5773088f20527a**. Version 0.2.5-beta / code 25 confirmed. No APK, game data or raw evidence was published.
- Launcher opens and creates the system-Vulkan device/swapchain. A local screenshot shows Game Files setup, with no game configured. This is application/setup evidence only: the selected 1.0.0 UI default is not a verified installed game version, and activity launch timings are not gameplay loading/FPS measurements. Initial shell-created scratch directories were not app-writable; preserved that failed logging/settings attempt, removed only the empty task-created directories and retried using app-owned internal scratch paths. File logging and separate settings then succeeded. The single operator force-stop between setup launches is explicitly recorded as USER REQUESTED, not a crash. No original save was imported, overwritten or modified.
- The user identified their supplied game/save folder. Inspection found base/update NSZ and DLC NSP/NSZ containers, plus a save ZIP; **no extracted exefs/romfs tree**. User then confirmed no extracted game folder is available. The unchanged app's import path requires exefs with main/main.npdm plus romfs (or romfs.bin), and rejects package-only inputs (README.md and BOTWRecompAndroidActivity.java:824-862, 967-982). The update filename/version is unverified metadata, not a game-version result. No container decompression/decryption, external extraction-tool preparation, key access, game-code generation or asset acquisition occurred.
- Copied the supplied save ZIP locally, verified its archive hash/CRC and **226** file hashes; it contains game_data.sav/caption.sav slot candidates. The original stays unchanged on device. Slot, scene, version/DLC compatibility and immutable seed selection remain pending real game setup; archive presence alone does not establish the S1/S2 seed. Save contents and raw inventory remain private/local.
- Capability preflight confirms **android.surfaceflinger.frametimeline** is advertised. No in-game layer, per-buffer validity, trace loss, measurement overhead or frame-time source has been validated. Available startup battery/thermal/memory/display observations are retained separately; USB power was active, so performance prerequisites were not met. Ambient conditions, refresh/brightness/power controls, fixed game settings, cache provenance and scratch-seed behavior still need verification before gameplay. Defaults captured at setup are not the fixed performance configuration.
- **Gameplay attempts/completions/crashes/freezes: 0/0/0/0**; S0 game boot/title/save load, all 24 S1/S2 performance windows, diagnostic gameplay and both bounded stability runs were not started. FPS/frametime percentiles, symptom frequency and Native/Recompiled comparison are **N/A**. No claim about gameplay correctness, Mali performance, memory growth, crash reproduction or crash-free gameplay duration is supported yet.
- Local evidence/report: `.mali/milestone-4/DEVICE_BASELINE_REPORT.md`, device identity, raw ADB preflight/command records, install/installed-hash results, setup screenshot/logcat/app log/settings/exit records and private save archive/manifests. MALI_PLAN.md and AGENTS.md are unchanged. No instrumentation, source edit/test repair, source publication, upstream mutation or milestone 5 was performed.

**Specific blocker / next action:** provide an already extracted lawful game folder containing **exefs + romfs (or romfs.bin)** for a supported game version. Then resume the authorized milestone 4, verify that game's actual build/version and save compatibility, protect the original save via a verified scratch seed, finish preflight and run M3-v1. Preparing an extraction/dependency workflow is not silently added to this bounded device milestone. Milestone 4 is **not complete**; the data-blocker stopping condition was reached. **STOP here; do not advance to milestone 5.**

Only the sanitized progress summary is changed in the documentation checkpoint. Exact publication SHA/readback and the local evidence backup receipt are recorded under `.mali/milestone-4/` and in the task report after publication. Raw captures, logs, full device identifiers and the user's save remain local; same-disk backups do not establish offline or remote evidence persistence.

## Continued milestone 3b outcome and current stopping point

- Source remains **main at 3d66ac0482ae9d99546a579170675dd6eb3c047c**, 0.2.5-beta. Recovered documentation checkpoint **b06ac6d44c42840681a3d3f31d5cbf876225f0d5**. All **184** prior manifest entries, **825** prepared dependency output files and **12** source archives matched their recorded hashes before continuation. New evidence is under `.mali/milestone-3b-resume-1/`; pre-resume documents/manifest are archived there. Prior evidence, logs, partial dependency inventory and backups remain unchanged. Completed Windows milestone 3a was not repeated or reinstalled.
- WSL 3.0.3.0 / kernel 6.18.54.1-1 with Ubuntu-26.04 (26.04.1 LTS) was installed, but initial commands waited for distro OOBE. The user completed the remaining setup prompt and confirmed `id` worked; Linux commands then succeeded. No WSL shutdown/restart or Windows setting change was performed. Prepared standard Linux build utilities through available distro root execution: Make 4.4.1, CMake 4.2.3, Ninja 1.13.2, Python 3.14.4, pkg-config 2.5.1. These Linux host-tool versions are not pinned by the Linux dependency scripts; the pinned Windows application toolchain is unchanged.
- Prepared exact Linux NDK **30.0.16248370**, checking the official Google repository archive size/checksum and recording SHA-256 **753611f410d002cfcd3f3dc2ef49aad532089d3180b436c060a90bf0fcb64df2**. Verified installed revision and clang 21.0.0. Reused the previously verified source cache. Unchanged `build_ffmpeg.sh` and `build_unicorn.sh` completed under WSL, both **exit 0**; unchanged `build_gen_deps.sh` then completed under Git Bash, **exit 0**, finishing LZ4/bindings/Zig without rebuilding core SDL/shaderc or generating game code. No source/pin/script substitution. All 12 source hashes and seven shaderc dependency Git revisions match. New inventory records **3204** prepared output files / **432610946** bytes with full agreed composition ready.
- Actual app build from android/botwrecompandroid using the preserved process-only Temurin JDK 17.0.20.1+1: `./gradlew.bat assembleDebug --no-daemon --console=plain --stacktrace`, **exit 0, BUILD SUCCESSFUL in 41s, 39 tasks executed**. Build time 20:26:58–20:27:40 UTC on 2026-10-30. Gradle 8.14.3 / AGP 8.13.0, Windows NDK 30.0.16248370 / SDK CMake 3.30.4. Compile metadata confirms movie.c included, no LARECOMP_NO_FFMPEG, runtime -O2/-g and Android30 AArch64 target. Warnings are retained in raw logs; no warning/source/test repair attempted. Global Java settings restored unchanged.
- **Baseline APK:** 0.2.5-beta, versionCode 25, com.botwrecomp.android, debug, arm64-v8a, min SDK 30 / target 36; **98260536 bytes**. Local preserved artifact: `.mali/milestone-3b-resume-1/artifacts/BOTWRecompAndroid-0.2.5-beta-3d66ac0-baseline-debug.apk`. **SHA-256: aabb4ea6f8c86276092a2ed8043c15bfedf60da643f77173c59b617b979f32c8**. The built-output and preserved-copy hashes match; no claim of bit-identical upstream release reproducibility.
- **Signature verified:** apksigner exit 0, APK v2 scheme, one Android Debug RSA-2048 signer; **certificate SHA-256: 60fa2a5f462fd95d30b37dde54d769de8986d7c8377e13c3ea5773088f20527a**. No signing secrets/keys published. zipalign -c -P 16 4 exit 0, ZIP CRC/metadata/required assets verified. All **17** native ELF members are AArch64 with statically compatible 16-KiB LOAD alignment/congruence. Full FFmpeg and Python/pymain/Capstone/Unicorn/LZ4/Zig builder members present; no generated botwgame.so in APK. Runtime game-library ABI ID **65da5d95e597ba58**, generator payload ID **977a9c772b6c85c5e693f988f06ea990f95463c800a99c9051594ce3c87ecbd6**. Static/package checks do not establish in-game correctness.
- Applicable same-SHA/pinned-toolchain host evidence from the first 3b pass remains: four groups / **74 assertions passed**, with **test_android_upload_caches.py still failing to compile its extracted helper due to undeclared ga_wrote**. Its source/input issue is unchanged; passing helpers were not needlessly rerun. This known harness limitation is explicitly retained; no all-green suite claim and no repair without separate agreement. Full APK construction succeeded independently of that harness. No ADB, probe, installation, uninstall, game-code generation, device test or measurement occurred.
- Local report `.mali/milestone-3b-resume-1/BUILD_REPORT.md`, new inventory, exact commands/exit records, NDK metadata/checksums, APK/native-build/signature/ZIP inspection records and full logs preserve the trace. `BASELINE_PROTOCOL_RECORD.md` fills actual source/APK/certificate/ABI/cache metadata while original M3-v1, prior addendum and test history stay unchanged. Phone/game-version/driver/current-package signature still require real step-4 evidence. MALI_PLAN.md and AGENTS.md are unchanged; tracked source/index/gitlinks remain equal to HEAD, prior recovery clone preserved.

The missing-WSL dependency blocker is resolved. **3b is complete with the documented existing host-harness limitation; milestone 4 is ready for separate authorization.** Do not treat this as device/rendering/performance validation, and do not install the APK or change saves automatically. Step 4 must identify the actual device and installed certificate before selecting a save-preserving installation procedure.

Only sanitized checkpoint documents are published on the existing checkpoint branch; this revision changes MALI_PROGRESS.md only. Final documentation SHA/readback is recorded locally in `.mali/milestone-3b-resume-1/publication.json` and the task report. The full dependency-cache archive (3217 data entries plus inventory) and evidence/APK archive are verified separately; previous partial-cache/evidence archives remain intact. Backups are local on the same disk, not remote raw-evidence persistence. **STOP after 3b; wait for the user's explicit milestone-4 command.**

## Initial milestone 3b outcome and stopping point (historical blocker; evidence preserved)

- Source remains main at **3d66ac0482ae9d99546a579170675dd6eb3c047c**, 0.2.5-beta; source/index/remotes/pinned submodules preserved. Recovered preceding documentation checkpoint **ef01732c3f787a9cd77570df79b238f6eef95ec8**; all **136** previous manifest entries matched size/SHA-256 before work. Pre-stage documents/manifest/exclusions and earlier generated host-test outputs were copied locally before reuse. Completed 3a was not repeated, reopened or reinstalled; its historical validation and all prior raw evidence remain preserved.
- Ran unchanged `android/deps/build_deps.sh` with Git Bash and the existing pinned NDK r30 / SDK CMake 3.30.4: **exit 0**, about 128 seconds. Built arm64 SDL 3.4.16 and shaderc 2026.3; prepared Vulkan-Headers/volk SDK 1.4.350.1, VMA 3.3.0, matching SDL Java glue and DejaVu 2.37 font/license. All **12** core/builder/FFmpeg source archives were ultimately cached and independently SHA-256 verified against unchanged pins; all **7** shaderc DEPS Git revisions matched. Dependency inventory records 825 prepared output files with hashes, rather than claiming an APK exists.
- The first unchanged `build_gen_deps.sh` attempt stalled at 12288 bytes of Zig download. After over 120 seconds without progress, the exact task-owned curl process was identified and stopped for a bounded retry; partial file and both attempt records remain. One retry from the **same official URL**, with a 55-second timeout, succeeded and matched the unchanged Zig archive SHA-256. The resumed script prepared CPython Android 3.14.8/pymain and ARM64 Capstone 5.0.9, then exited **1** at its explicit Windows requirement: build `libunicorn.so` through WSL first. LZ4/bindings/Zig payload steps after that guard remain incomplete; their source archives are verified and retained. No source pin, script or checksum was changed.
- **Precise full-APK blocker:** `wsl.exe --list --verbose` exits 1: Windows Subsystem for Linux is not installed. Current Windows session is not elevated. The pinned FFmpeg and Unicorn build scripts require Linux/WSL on Windows, so FFmpeg libraries and Unicorn remain absent; Linux NDK/build-tool preparation cannot run in this session as-is. No OS-feature/distro installation, privilege escalation or reboot was attempted. [Microsoft installation guidance](https://learn.microsoft.com/en-us/windows/wsl/install) describes administrator setup and restart; this is a host prerequisite, not a failure of completed Windows milestone 3a. FFmpeg 9.0.2 source is cached/verified but was not built under an unsupported shell.
- The agreed full composition remains FFmpeg plus the on-device Recompiled builder. No reduced APK was substituted. **APK, APK SHA-256 and signing-certificate identity are N/A.** `assembleDebug` was not invoked with that composition unmet; this is an observed prerequisite blocker, not an Android/native application compilation failure. Local readelf inspections confirm prepared core/Python/Capstone artifacts are ELF64 AArch64; no Android execution is claimed. No game-code generation or game-data acquisition occurred.
- Rechecked existing host tests with **pinned NDK 30.0.16248370 / clang 21.0.0**, not r28: timing history **8 PASS**, shader cache **17 PASS**, Vulkan startup **33 PASS**, Vulkan compatibility **16 PASS**. Upload-cache harness still exits 1 before assertions because extracted ga_load_copy calls undeclared **ga_wrote**; production defines it outside the harness's extracted region. Four groups / 74 assertions passed, not an all-green suite. No source/test repair was made; such changes still require separate agreement. Earlier failures remain historical evidence and were not overwritten. Device regression runner, probe, game tests, benchmark and step 4 were not run.
- Added only local exclusions `/android/deps/src/`, `/android/deps/work/`, `/android/deps/out/` after archiving prior exclusion rules. Resolved native output roots were checked within the workspace before build/unpack operations. Tracked .gitignore and application/test/dependency-script sources are unchanged. Prepared archives/artifacts are excluded local build state, never PR contents. The pre-existing recovery clone remains preserved.
- Local deliverables: `.mali/milestone-3b/BUILD_REPORT.md`, build/transfer/WSL/test command logs and exit records, `dependencies.json`, ELF inspection records, copied prior test outputs, new `PROTOCOL_ADDENDUM.md`, integrity/publication/backup records. Original M3-v1 protocol and completed 3a/synchronization records stay unchanged. The addendum records 0.2.5 SHA/cache/default/known-issue metadata with no device measurements. MALI_PLAN.md scope/order and AGENTS.md are unchanged.

**Next required action:** prepare WSL 2 and a Linux distribution through an administrator-controlled Windows session and explicitly handle any required reboot; then instruct resumption of **3b**, reuse verified caches and the completed Windows 3a environment, prepare the scripts' exact Linux NDK/build utilities, build FFmpeg/Unicorn, finish builder preparation and produce the unchanged full APK/hash/signature. A focused upload-test harness repair, if desired, needs separate agreement. This checkpoint does not install OS features, change the plan, authorize step 4, or claim a usable baseline.

Only sanitized checkpoint documents are published on the existing branch; this revision changes MALI_PROGRESS.md only. Actual publication SHA/readback are recorded in `.mali/milestone-3b/publication.json` and the final report. Separate verified local archives preserve dependency source caches/prepared artifacts and the documentation/raw-evidence package; receipts remain local. They are same-disk backups, not remote/offline persistence. **STOP after this blocked 3b checkpoint; await the user's next instruction.**

## Bounded upstream synchronization outcome and stopping point (historical; completion preserved)

- Recovered the completed 3a checkpoint `f7f64097d626e401a488a8d5d6f30150f685f9a7`; all **108** previous manifest entries matched SHA-256/size before editing. Archived pre-sync documents/manifest under `.mali/upstream-sync-025/`. All **56** manifested files under `.mali/milestone-3/` and `.mali/milestone-3a/` remained unchanged after synchronization; historical validation, environment records, protocol and failed host-test evidence are preserved. No completed milestone was reopened or repeated.
- New application baseline: **`3d66ac0482ae9d99546a579170675dd6eb3c047c`**. It is fetched upstream/main and lightweight tag `v0.2.5-beta`; the release is a prerelease published 2026-10-30 19:29:21 UTC. Relative to old baseline `4acbf40c9312618e16e1153f3696c64e125d2a2f`, reviewed 13 commits / 25 changed files. The API latest-release endpoint returned 404 because the current releases are prereleases; the release collection and tag resolve 0.2.5. No release APK was downloaded or substituted for our future baseline APK.
- Verified ancestry, clean source/index, fork identity/parent/push permission, expected remote main heads, absence of incoming guidance/evidence/recovery-clone collisions, and absence of custom active hooks. Fork main advanced through a non-force GitHub ref update with expected old SHA; local main advanced with `git merge --ff-only --no-edit <reviewed SHA>`. Readback confirmed **local HEAD = origin/main = live fork/main = live upstream/main = 3d66ac0**. No source patch, new source/merge commit, reset/stash, branch switch, force push, upstream write or default-branch change. Both pinned submodules and the pre-existing untracked recovery clone are preserved. Tracked files/index equal new HEAD; guidance and raw evidence remain locally excluded.
- Most relevant new fix: [0f48cf3](https://github.com/mamcjr/BOTWRecompAndroid/commit/0f48cf301f7d0b0c5426fb3a09a3dc7d8e6a4ab8), explicitly linked by upstream to #54/#89, changes global-memory range selection/write-back eligibility, removes huge-range overflow collapse, bounds learned ranges, and grows the resident partition 32→48 MiB within the same 128 MiB arena. It also clears pending writes/dirty/learned/counter state on restart and changes exit-handler ordering/Android idle-wait behavior. These are real source changes overlapping the old large-upload/gameplay/teardown candidates, not proof that the exact #89 crashes or #105 freeze/purple corruption are resolved on G925. Do not select a duplicate contribution without new-baseline device evidence.
- [f5384bb](https://github.com/mamcjr/BOTWRecompAndroid/commit/f5384bbec26433e3357a70a461c5ba97243efa74) handles Android surface loss/native-window-in-use during swapchain creation and retries surface creation on a later frame (#108). [02ab939](https://github.com/mamcjr/BOTWRecompAndroid/commit/02ab939b1406c6a5e27eeccb9afa2344d3932991) rounds matching UNORM material-ID conversions in the Maxwell translator (#112, black distant trees with scaled/dynamic resolution). These are specific fixes, not blanket explanations of all freezes or visual corruption. New optional debug hooks in ea16b20 are available for a later authorized diagnostic stage and were not run.
- Movie ABI/failure handling and romfs.bin/game-vs-DLC setup paths also changed; full FFmpeg composition remains relevant for the future baseline. Game-code generator/GAMELIB_ID inputs and build flags did not change. The old Mali caching fix 4a1aebd remains an ancestor. #97/no-FFmpeg and #103/renamed-Adreno heap behavior were already in the old baseline; do not count them as new Mali fixes in this synchronization.
- New upstream notes raise mapping-count exhaustion in write tracking as a **hypothesis**; no runtime/platform/android.c change appears in this diff, so a proposed mitigation is not implemented here. Prior #105 evidence limits remain. Upstream still lists a 1.0.0 combat corruption/crash limitation and reports Adreno test hardware; no performance/rendering/stability claim is verified on the user's phone. Existing attachment investigations were not repeated.
- **3a compatibility: preserved by unchanged requirements and matching installation records.** App Gradle changes only versionCode/name; Gradle 8.14.3, AGP 8.13.0, Java 17, SDK 36/min 30, NDK 30.0.16248370, CMake 3.30.4, arm64-v8a, native/probe CMake, dependency scripts/archive pins, submodule pins and contributor rules are unchanged. All **11** recorded SDK metadata/tool-binary hashes match; existing JDK binaries/release file match Temurin 17.0.20.1+1. No new environment delta identified, no reinstall. The original 27-second Gradle help success remains evidence at the old SHA; **no Gradle/configuration/compiler/test/build was rerun on the new SHA**. Actual new-source build/runtime verification is still pending 3b/4.
- Remaining **3b** prerequisites are unchanged apart from the new source SHA/version: separately authorize 3b, select preserved process-only JDK 17, prepare the pinned SDL/shaderc/Vulkan/VMA/volk/font dependencies and agreed FFmpeg/on-device builder composition, address the already-planned WSL/Linux NDK prerequisites where needed, then build and hash/sign the new baseline APK and run applicable checks. No native dependency output or locally built APK exists. Earlier host-test harness failures remain unresolved historical observations; repairs are not implicitly authorized. Preserve M3-v1 and use a new-baseline metadata/cache addendum; no device measurement or new experiment now.

Local report: `.mali/upstream-sync-025/UPSTREAM_REVIEW.md`. Evidence includes resume integrity, upstream patch/history/tag/release metadata, fast-forward preflight/readback, `compatibility.json` and its read-only script, and final publication/integrity/backup records. Only sanitized AGENTS.md/MALI_PLAN.md/MALI_PROGRESS.md are published; AGENTS.md is unchanged. The checkpoint branch intentionally keeps its historical application tree independently of main. It is a documentation recovery source, not the current application build checkout; never merge it into a source branch. Final containing documentation SHA is in `.mali/upstream-sync-025/publication.json` and the task report. Raw evidence and separate verified same-disk backup stay local.

**STOP after this bounded synchronization.** Await a separate execution instruction for 3b. Neither 3a repetition nor 3b/4 execution, dependency installation or source patching is authorized by this checkpoint.

## Milestone 3a outcome and stopping point (historical; completion/evidence preserved)

Plan revision approved and applied before environment preparation. Source baseline remains `4acbf40c9312618e16e1153f3696c64e125d2a2f`, branch `main`; the preceding remote checkpoint is `4b80d38e2c238d2d990f539db939007cfa4f004c`. The completed milestone-3 report/protocol/test failures below are preserved historical findings; their earlier unapproved proposal is superseded only by the approved revision above.

- Verified all 72 previous manifest entries before editing. Archived the pre-revision three documents and manifest under `.mali/milestone-3a/`. After the environment work, all 20 manifested files under `.mali/milestone-3/` still matched their original SHA-256 and byte count. No investigation/protocol/test result was discarded, reset or repeated.
- Official SDK discovery (`sdkmanager.bat --list --verbose`, exit 0) explicitly listed exact `ndk;30.0.16248370` and `cmake;3.30.4`. Installed only those packages alongside existing versions (`sdkmanager.bat --install ...`, exit 0). SDK 36 revision 2 and Build Tools 35.0.0 were already present. Source properties and actual executables verify NDK r30 / clang 21.0.0, SDK CMake 3.30.4 and bundled Ninja 1.10.2. Package metadata and tool binary hashes are retained locally. Original SDK download archive hashes were not separately retained; do not confuse this metadata check with the independent JDK archive check below.
- Prepared portable Eclipse Temurin JDK **17.0.20.1+1**, java/javac 17.0.20.1, from official Adoptium API/release metadata. Downloaded Windows x64 HotSpot ZIP size 190817615 bytes and SHA-256 `e53a79c3c3d86865bd7e787903884331068e71321714ffd44f145785affc7cb0` matched the published API value before extraction. The archive and separate installation stay in the user's local toolchain directory; no private absolute path is published. Use the recorded immutable release/checksum for reproduction, not an assumption about the moving latest-JDK API response.
- Used JDK 17 only through temporary process JAVA_HOME assignments restored afterward. Process/user JAVA_HOME still selects existing Android Studio JBR 25; machine JAVA_HOME remains unset. No global Java/PATH setting, Gradle/AGP pin, application source or local.properties was changed.
- Initialized `third_party/libadrenotools` at gitlink **8fae8ce254dfc1344527e05301e43f37dea2df80** and nested `lib/linkernsbypass` at its gitlink **aa3975893d83ef1bc84c321ec60c65fbf1287887** with `git submodule update --init --recursive -- third_party/libadrenotools`. Both checkouts are clean; no `--remote`, source patch or library build. No nested AGENTS.md was found in these newly initialized sources. Root Git configuration now registers the submodule, and its pinned contents are present; the source/index/gitlinks remain unchanged.
- From `android/botwrecompandroid`, with the temporary JDK 17, ran `./gradlew.bat help --no-daemon --console=plain --stacktrace`: **exit 0, BUILD SUCCESSFUL in 27s, only task :help executed**. Separate `--version` readback confirms Gradle 8.14.3 and actual launcher/daemon JVM 17.0.20.1. The previous JBR-25 Kotlin DSL blocker is resolved for this build process. This verifies Gradle project configuration, not native CMake configuration, compilation, linking, packaging or runtime correctness.
- Local report: `.mali/milestone-3a/ENVIRONMENT.md`. Supporting records: resume/preserved-evidence hashes; JDK metadata/install/script/logs; SDK discovery/install logs and exit records; recursive submodule setup/state; Gradle configuration/version logs and exit record; actual tool versions and installed metadata in `environment-final.json`. No credentials, game data or raw logs are published.
- No remaining **3a** blocker. Native dependencies (`android/deps/out`), full baseline composition and APK/hash/signature remain for **3b**; no dependency-build script, assemble task, game-code generation, host-regression rerun, probe, device test or measurement was performed. The earlier host-test failures remain unresolved historical observations. Source branch/HEAD/index and pre-existing recovery clone are preserved; no source commit/push, upstream write, PR, branch switch/merge or default-branch change.
- At final remote verification, live upstream/main had advanced to **3d66ac0482ae9d99546a579170675dd6eb3c047c**, while origin/main and the local baseline remained at 4acbf40. Read-only comparison metadata shows 13 commits ahead, including Android Gradle/activity and Vulkan device/exec/translation changes. No detailed triage or validation of these new changes was performed. This does not alter the successful 3a result at the recorded SHA or authorize updating source. Milestone 3b remains defined on the recorded baseline; before adopting newer upstream, propose the baseline change and obtain approval under the adaptive-plan rule. A targeted upstream review is recommended before deciding to change that baseline. Local evidence: `.mali/milestone-3a/upstream-advance.json`.

Checkpoint publication changes only MALI_PLAN.md and MALI_PROGRESS.md within the standing three-document checkpoint; AGENTS.md is unchanged. The containing SHA and remote readback checks are recorded in `.mali/milestone-3a/publication.json` and the final task report. Local manifest and a separate same-disk evidence backup are verified after publication; backup receipts remain local. The backup preserves documentation/evidence, not large SDK/JDK installations or archives; reinstall those from the recorded pinned setup if needed. GitHub still does not preserve raw evidence.

**STOP after 3a.** The next eligible execution command is for **3b**, which remains unauthorized. The approved split does not authorize dependencies/APK work or milestone 4. No further plan revision is proposed by this successful environment preparation.

## Milestone 3 outcome and stopping point (historical; investigation preserved)

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

### Blockers, proposed plan adjustment, and next authorization (historical milestone 3)

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
- Original publication target (historical; renamed on 2026-10-31): https://github.com/Propinbo/BOTWRecompAndroid/tree/codex/mali-workspace. Authenticated account and fork owner are Propinbo; the fork has push permission, upstream parent is mamcjr/BOTWRecompAndroid, and default branch is main. No checkpoint branch existed at initial inspection.
- Published first documentation revision `db0a5d2b3ce89ff19362cf6ec61c0855f3aa8e4d` on the checkpoint branch through authenticated GitHub Git-data/ref operations (equivalent remote Git storage, without a shell push). Its sole parent is the recorded application baseline; comparison against that baseline contains exactly AGENTS.md, MALI_PLAN.md, and MALI_PROGRESS.md. No application source change was published. Commit messages use `[skip ci]`; no application build was invoked.
- Independently cloned the checkpoint branch from GitHub into a new local directory without copying the original workspace or its evidence. All three Git blob IDs and text contents matched the originals (Git's Windows checkout line endings were normalized for comparison). Recovery evidence: `.mali/workflow-persistence/retrieval-first.json`, retained locally.
- Ran a genuinely fresh, ephemeral Codex CLI session (`codex-cli 0.166.0-alpha.5.1`, no resumed chat history) against that clone with the read-only sandbox. It exited successfully, read the three files, identified root AGENTS.md as active guidance, preserved complete milestones 1-2 and unauthorized milestones 3-9, and correctly stated the manual milestone command, material-plan-change approval rule, and documentation-only publication destination. No milestone, build, device operation, edit, commit, push, or network mutation was performed by the verification session. Private transcript/result and command audit remain under `.mali/workflow-persistence/` and are not published.
- During hash diagnostics an isolated clone line-ending setting temporarily made Git flag existing CRLF checkouts as modified. Restoring the inherited setting cleared those flags without writing application source files. Both the recovery clone and the original source checkout then passed working-tree/index diff checks. The first session's checkout caveat is thus resolved; its instruction-loading and status results remain valid.
- This establishes remote documentation recovery and instruction access in an independent CLI session, independent of this chat's memory and of the original local files. It does not test a desktop-app or OS restart, guarantee future GitHub/network availability, or establish persistence of the excluded raw evidence. A default-branch-only clone will not automatically contain these documents; recover the checkpoint before starting Codex.
- Only these three documentation files will be stored remotely. `.mali/` evidence, local manifests, patches, and exclusions do not accompany a fresh clone. Retain a user-controlled local/offline backup. Missing raw evidence must be disclosed, not fabricated or treated as automatic grounds to repeat a completed milestone.

## Resume in a new Codex session

Recover documentation before starting the session so root AGENTS.md can load. Use a new, nonexistent local directory for a clean recovery:

```powershell
git clone --single-branch --branch mali-workspace https://github.com/Propinbo/BOTWRecompAndroid.git BOTWRecompAndroid-checkpoint
Set-Location BOTWRecompAndroid-checkpoint
git rev-parse HEAD
Get-Content AGENTS.md
Get-Content MALI_PLAN.md
Get-Content MALI_PROGRESS.md
```

Open that repository root in a new Codex session. A current recovery-only prompt is:

> Read AGENTS.md, MALI_PLAN.md, and MALI_PROGRESS.md from Propinbo/BOTWRecompAndroid, checkpoint branch mali-workspace. Verify the checkpoint and existing work. Preserve completed milestones, evidence and decisions. Milestone 4 remains in progress; do not resume it or start another milestone without the user's current instruction. Recovery itself authorizes no project work.

Historical milestone-3 prompt from the original persistence verification (retained verbatim, including the former branch name; superseded as an active recovery prompt and not execution authorization):

> Read AGENTS.md, MALI_PLAN.md, and MALI_PROGRESS.md from Propinbo/BOTWRecompAndroid, checkpoint branch codex/mali-workspace. Verify the checkpoint and existing work. Milestones 1-2 are complete; preserve their findings. Приступай к шагу 3. Execute only milestone 3 and stop afterward. Do not commit or publish source changes. Update only the three checkpoint documents on codex/mali-workspace under the standing documentation authorization.

For an existing source checkout, preserve uncommitted work and compare any newer local notes before recovery. Fetch only the checkpoint branch from the verified fork, then restore only the three documentation paths into the working tree without staging them:

```powershell
git fetch --no-tags https://github.com/Propinbo/BOTWRecompAndroid.git refs/heads/mali-workspace:refs/remotes/origin/mali-workspace
git restore --source=origin/mali-workspace --worktree -- AGENTS.md MALI_PLAN.md MALI_PROGRESS.md
```

The restore assumes these paths have no newer local content or staged changes; if they do, preserve and reconcile first. On a source branch, locally exclude `/AGENTS.md`, `/MALI_PLAN.md`, `/MALI_PROGRESS.md`, and `/.mali/` via `.git/info/exclude` (or the path reported by `git rev-parse --git-path info/exclude`). These exclusions are local and must be recreated in a new source checkout. Keep documentation untracked there; stage source paths explicitly and never cherry-pick documentation commits into a source PR or merge the checkpoint branch into a source branch. Before a source PR, verify its complete diff contains none of the checkpoint files.

Current source baseline is `3d66ac0482ae9d99546a579170675dd6eb3c047c` after the explicitly authorized synchronization. Historical milestone-3/3a results remain tied to `4acbf40c9312618e16e1153f3696c64e125d2a2f`; a documentation commit is not an application baseline. The checkpoint branch retains its older application tree deliberately: a clone of that branch recovers documents/history, not current source. To resume application work, use a main checkout at the recorded current baseline and restore only the three guidance paths as above, preserving newer local content and exclusions; do not merge checkpoint history into source. Verify branch/remotes/SHA/patch/evidence before work. Earlier example prompts and document recovery are not execution authorization; 3a is complete and the next eligible milestone is separately authorized 3b.

## Documentation task outcome (historical)

- Created AGENTS.md and updated only the workflow/persistence portions of MALI_PLAN.md and the progress/checkpoint record here. Verified all nine milestone definitions/order, all nine status rows, and the prior key findings against the archived originals. The adaptive-planning policy and restricted documentation-publication exception are the user-approved reasons for these updates.
- Original source checkout remains on main at `4acbf40c9312618e16e1153f3696c64e125d2a2f`, with unchanged tracked files and index. No project milestone was executed; no application build, device test, source edit/commit/push, upstream write, PR, merge, default-branch change, or force-push was performed. Root AGENTS.md and the two Mali notes are untracked and locally excluded there; `.mali/` remains excluded. Tracked .gitignore is unchanged.
- Only the three reviewed documents are checkpoint changes in the fork. Historical originals, evidence, fresh-session transcripts, and verification artifacts stay local. The updated local `.mali/checkpoint-manifest.json` covers current documents/evidence after publication; the pre-task manifest remains archived. Do not compare new workflow notes against the historical milestone-2 hashes.
- The initial verified remote SHA is recorded above. This verification record was published as a subsequent documentation-only fast-forward on the same branch; for the current containing commit SHA use `git rev-parse origin/mali-workspace` after fetching (the checkpoint branch was renamed on 2026-10-31), or consult the relevant task's final report for its historical SHA. A document cannot contain its own final commit hash. Historical remote readback and local integrity results are retained in `.mali/workflow-persistence/publication.json`.
- Stop here. Milestone 3 and every later milestone still require their own explicit command. Future documentation-only updates to this same branch remain authorized after future explicitly authorized milestones; source publication remains separately controlled.
