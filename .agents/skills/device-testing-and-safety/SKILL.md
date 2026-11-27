---
name: device-testing-and-safety
description: >-
  Guides physical Android device testing on rooted AYN Odin 3 (SM8750 Snapdragon 8 Elite, Adreno 830 + Turnip, primary target), Galaxy Tab S11 (SM-X730, Mali-G925, protected secondary), and OnePlus 13 (CPH2653), including save backups, scratch isolation, rooted read-only telemetry, driver verification, APK signing, and stock restoration. Use whenever running ADB commands, Bionic unit binaries, or in-game experiments on devices.
---

# Device Testing and Save-Safety Runbook

Follow these mandatory steps for every physical-device operation.

## 1. Pre-Flight and Mandatory Dated Backup

1. Verify ADB connectivity (`adb devices -l`) and read-only root/Magisk/KernelSU access (`su -c id`) when inspecting `/data/data/com.botwrecomp.android` or kernel sysfs nodes. Keep `su -c` strictly scoped to read-only telemetry (`kgsl-3d0`, `cpufreq`, `power_supply`, `dmesg`, `simpleperf`) and scratch paths inside the app's own `files/` directory — never alter `/system`, `/vendor`, `/boot`, SELinux, or user data.
2. Check whether today's backup `.mali/user-backup/<YYYY-MM-DD>/` exists (`odin3/` on Odin 3 via `.mali/odin3-pivot/backup_odin3.py`, or `.mali/user-backup/backup.py` on `SM-X730`/`CPH2653`) and matches current device saves, INI, preferences, and caches (`*.sha256` manifests).
3. If no backup exists for today, or if the owner played since the last backup, capture a new dated SHA-256 backup **before** launching the game. Never overwrite earlier dated backups.
4. Record the currently installed `com.botwrecomp.android` APK SHA-256 and confirm the signed stock rollback APK exists locally under `.mali/apk/` (or `.mali/user-backup/<YYYY-MM-DD>/odin3/base.apk` for stock `v0.5.0-beta`).

## 2. Adreno Turnip-Only & Sequential Odin 3 Telemetry Rules

- Test `Odin 3` and `CPH2653` (Adreno 830) **exclusively with Turnip** (`VK_DRIVER_ID_MESA_TURNIP`, `driver-id 18`, e.g., prepackaged `Turnip_Gen8_V39_Patched` / `V38_Patched` — no custom driver builds) unless the owner explicitly requests comparing or testing stock Qualcomm. Target **Native `1080p` (`100%` `1:1`) @ `40.00 FPS` (`< 6–7.5 W`)** and verify `driver-id 18` / Mesa Turnip in `[vk]` logs before accepting any result.
- Run all Odin 3 benchmarks and `TU_DEBUG`/`IR3_SHADER_DEBUG` launches **strictly sequentially** via `.mali/odin3-pivot/odin3_telemetry.py` (never overlap GPU/CPU runs). Wait for thermal cooldown (`gpu_temp_c <= 42 C`, `thermal_pwrlevel == 0`) between measured runs.
- Because Odin 3 uses Charging Separation (`charge_control_end_threshold = 80`), record **both** `usb_mw` (`usb/voltage_now * usb/current_now`) and `batt_mw` alongside `kgsl_busy`, Oryon cluster (`0-5` vs `6-7`) stats, `SurfaceFlinger --latency` BLAST layer buckets, and `dmesg` KGSL fault deltas (`gpufaults_delta == 0`).
- Disable user mods cleanly in scratch settings (`mods_off = 01007EF00011E000`) during baseline runs without touching `/sdcard/Android/data/com.botwrecomp.android/files/v190/mods`.

## 3. Scratch Isolation and Test Execution

1. **Never** run `adb uninstall com.botwrecomp.android` or `adb shell pm clear com.botwrecomp.android`.
2. Re-sign candidate debug APKs with the local debug keystore (`60fa2a5f...`, `~/.android/debug.keystore`) and install in-place via `adb install -r` (or `-r -d` when rolling back if needed).
3. Launch all game tests strictly against isolated scratch directories via `LARECOMP_SAVE_DIR`, `LARECOMP_SETTINGS`, `LARECOMP_DATA_ROOT`, and `LARECOMP_LOG_FILE`. On `nosymfollow` mounts (such as OnePlus `CPH2653`), keep `LARECOMP_DATA_ROOT` on the read-only payload and snapshot/restore `cache` + `build` around isolated scratch save/settings paths.
4. Run native Bionic C unit tests (`tools/test_android_runtime.ps1` and standalone `android/out/test_*`) separately from in-game runs, verifying `rc=0` directly from device stdout.
5. Keep heavy profiler/diagnostic runs (`--simpleperf`, `LARECOMP_*_TRACE`, `TU_DEBUG=perf`, `LARECOMP_SHADER_STATS=1`) separate from clean FPS/frametime/power baseline runs.

## 4. Post-Flight Stock Restoration vs. Final Sign-Off Play-Ready Deployment

For intermediate A/B runs or when returning a secondary device (`SM-X730` / `CPH2653`) to stock:
1. Force-stop `com.botwrecomp.android` and confirm `pidof com.botwrecomp.android` is empty.
2. Delete only task-created scratch directories (`rm -rf` only when `readlink -f` equals the scratch parent) and `/data/local/tmp/` staging artifacts (never dereferencing symlinks or touching real saves/caches).
3. Restore the exact stock signed APK (`adb install -r`) without launching it when reverting to stock baseline, and verify installed `base.apk` SHA-256 matches stock.
4. Re-verify all user saves, `.ini`, and `shared_prefs` SHA-256 hashes against today's backup (`diffs=0`; note Android/app harmlessly update `android.app.ActivityThread.IDS.xml` and `crash_diagnostics.xml` boot counter on launch — always verify `watch_map.xml`, saves, and INI match byte-for-byte).
5. Verify shader/pipeline caches (`spirv` file count and `vk_pipelines.bin` SHA-256) remain intact.

At **Final Sign-Off on Odin 3** (after Stage D validation):
1. Do NOT roll back to stock `v0.5.0-beta`; install the winning signed release APK directly (`adb install -r`).
2. Clean up all scratch directories and `/data/local/tmp/` artifacts, preserving `27/27` real user saves (`0` SHA-256 diffs vs backup), existing `2686+` SPIR-V cache / `vk_pipelines.bin`, and `Turnip Gen8 V39 Patched`.
3. Back up live `/sdcard/Android/data/com.botwrecomp.android/files/v190/botwrecompandroid.ini`, root INI, and `shared_prefs/*.xml` once to host (`.mali/user-backup/<YYYY-MM-DD>-pre-final-deploy/odin3/`) AND `.bak` beside the live INI, then auto-write optimal settings in-place (preserving user preferences such as `language`, `overlay`, `mods_off`, touch layout) alongside `SM8750` C/Java defaults.
4. Launch once via normal Android launcher intent (zero `--es env`), confirm all optimal paths + config readback + screenshot + `gpufaults_delta == 0`, then force-stop cleanly and re-verify `27/27` user saves SHA-256 (`0` diffs).
