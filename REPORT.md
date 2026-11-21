# PR 2B-1 Evidence Report: Classic Render Pass Default on ARM Mali r49+ & `rpcompat.c` Hash Cache

- **Branch / Commit:** `feat/vk-mali-renderpass-default` @ `9d5e732c15da16a32a3f99797682ab26c5ccd4e4`
- **Base:** `mamcjr/BOTWRecompAndroid` `main` (`1c02adf994194055f7d517fc99dc5686269e11ed`, `v0.4.10-beta`)
- **Signed Release APK SHA-256:** `d69356b03c31b4a75ffd78e1a3c725f2f2a8c990a1bdf2de2e5d469a2f820dd5` (`0` warnings in modified files)
- **Device / Driver / Game:** Samsung Galaxy Tab S11 (`SM-X730`, `MT6991`), Android 16 (API 36), `Mali-G925-Immortalis MC12 - v1.r49p1-03bet0.f8a98b506b89b21c80a508d70abfcb3f` (`driver-id 9`, Vulkan `1.3.278`), BOTW `1.9.0` (`native`), warm cache (`11317` SPIR-V/meta files = `4230 .spv` + `7087 .smeta`)
- **Test Configuration:** Stock default runtime env (`screen 75% SGSR1`, `shadows=1`, `out_lines=0`, `antialias=0`; telemetry-only `LARECOMP_PACE_TRACE=1`, `LARECOMP_TIMING_TRACE=1`, `LARECOMP_LOG_GPU_IDLE=1`; Arm A adds `LARECOMP_NO_DYNAMIC_RENDERING=0`, Arm B adds no DR flags).

## Startup & Reboot Verification

- **Arm A (`LARECOMP_NO_DYNAMIC_RENDERING=0`):** `[vk] dynamic rendering enabled (LARECOMP_NO_DYNAMIC_RENDERING=0)`
- **Arm B (`Mali r49+` driver default):** `[vk] no dynamic rendering (Mali r49+ default): classic render passes` (cached classic render passes `1..8` built)
- **In-place Reboots:** Clean across all 4 arms (`[vk] reboot: ... (0 kept)`, `0` runtime errors).

## Isolated A/B Metrics (`cap30`, `15s` steady window, matched `36.5 C` pre-thermal skin)

| Scene (`451` frames/arm) | Metric | Arm A (`DR on`) | Arm B (`Classic RP default`) | Delta |
| --- | --- | --- | --- | --- |
| `plateau` | `render_thread_ms` p50 / p95 / p99 | `19.129 / 22.464 / 23.812` | **`16.725 / 19.227 / 20.334`** | **`-2.404` / `-3.237` / `-3.478 ms`** |
| | `gpu_ms` p50 / p95 | `27.529 / 31.438` | `27.559 / 32.333` | `+0.030 ms` p50 |
| | `fps_mean` / `display_low1` | `30.00 / 23.97` | `30.00 / 23.98` | Neutral at 30 cap |
| `zora-plaza` | `render_thread_ms` p50 / p95 / p99 | `19.486 / 22.593 / 25.174` | **`17.311 / 20.123 / 22.102`** | **`-2.175` / `-2.470` / `-3.072 ms`** |
| | `gpu_ms` p50 / p95 | `30.128 / 36.266` | `30.144 / 35.133` | `+0.016 ms` p50 |
| | `fps_mean` / `display_low1` | `30.00 / 23.99` | `30.00 / 22.17` (`skipped=58`) | Steady CPU `-11.2%` |
| `kakariko-day` | `render_thread_ms` p50 / p95 / p99 | `19.919 / 23.356 / 24.699` | **`18.050 / 20.480 / 21.463`** | **`-1.869` / `-2.876` / `-3.236 ms`** |
| | `gpu_ms` p50 / p95 | `27.789 / 31.777` | `27.731 / 31.636` | `-0.058 ms` p50 |
| | `fps_mean` / `display_low1` | `30.00 / 23.98` | `30.00 / 23.04` (`skipped=253`) | Steady CPU `-9.4%` |

## Uncapped `60p` Warmup Steady Phase (`15s` after `5s` initial settle)

| Scene | Metric | Arm A (`DR on`) | Arm B (`Classic RP default`) | Delta |
| --- | --- | --- | --- | --- |
| `plateau` | `fps_mean` / `low_1%` | `43.60 / 35.35` | **`44.73`** / `32.46` | **`+1.13 fps`** |
| | `render_ms_p50` | `15.288` | **`14.364`** | **`-0.924 ms`** |
| `kakariko-day` | `fps_mean` / `low_1%` / `disp_low1` | `33.94 / 25.23 / 23.23` | **`37.40 / 27.99 / 29.95`** | **`+3.46 fps` (`+10.2%`)** |
| | `render_ms_p50` | `19.775` | **`16.794`** | **`-2.981 ms` (`-15.1%`)** |
| `zora-plaza`* | `fps_mean` / `render_ms_p50` | `35.66 / 18.737` | `33.90 / 20.508` | *Thermally throttled in Arm B* |

\* Honest thermal note: `ArmB_60p_default` ran back-to-back after `ArmA_60p_dr` (`skin_c 44.3 -> 44.9 C`, `ap_c 56.2 -> 57.4 C`, `gpu_ceiling_idx=12`, CPU clocks dropped from `[1800, 2500, 2600]` to `[1500, 2300, 2300]`). Under thermal-matched `cap30`, `zora-plaza` improved by `-2.175 ms` p50 / `-3.072 ms` p99.

## `simpleperf` (`kakariko-day` 60p default, 56,904 total / 14,403 `render_main` samples)

- `scudo::MapAllocator::deallocate` + `munmap` is down to **`2.85%`** (`unmap_page_range` `1.12%`).
- Remaining top render-thread CPU symbols: `libGLES_mali` `stdlibp_neon_f32x16_to_f16x16` (`35.78%` / `6.33%` self), `stage_set` (`23.24%` / `9.90%` self), `__memmove_aarch64_simd` (`9.62%`), `guest_copy` (`7.72%`), `texture_info` (`5.88%`), `__memcmp_aarch64` (`3.95%`), `stage_fill_soft_linear` (`3.81%`).

## Visual & Unit Checks

- `ArmA_cap30_dr` and `ArmB_cap30_default` screenshots (`zora-plaza`, `kakariko-day`) are visually identical with zero artifacts.
- Automated verification: `workspace_check.py pr` PASS; `test_vulkan_compat.py` (16/16), `test_vulkan_startup.py` (33/33), `test_pipeline_cache_save.py`, `tools/test_android_runtime.ps1` (9/9 Bionic tests on `SM-X730` including `test_mali_rp_default`), and 4-target NDK Clang `-fsyntax-only -Wall -Wextra` (`0` warnings in touched lines).
- Adversarial code review: 2 parallel `independent-verifier` subagents both returned `APPROVE` (`0 CRITICAL`, `0 MAJOR`, `0 MINOR`) on `9d5e732c`.
