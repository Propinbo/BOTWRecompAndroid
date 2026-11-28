// Deterministic integer-divisor frame pacer for 120 Hz Mali / Android displays (1:3 = 40 FPS, 1:4 = 30 FPS).
// Pure C with zero Vulkan/Android dependencies so both vulkan/device.c and standalone unit tests include it.
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint64_t next_vsync_ns;      // scheduled display VSync edge for the next frame (CLOCK_MONOTONIC ns)
    uint64_t vsync_period_ns;    // validated display refresh period (ns, default 8333333 at 120 Hz)
    uint64_t last_actual_ns;     // latest hardware actualPresentTime from VK_GOOGLE_display_timing
    uint32_t last_divisor;       // last integer VSync divisor (3 at 40 FPS, 4 at 30 FPS on 120 Hz)
    uint32_t last_sent_id;       // latest presentID queued via mali_pacer_next_present_ex
    uint32_t last_actual_id;     // latest presentID observed in mali_pacer_observe_present
    uint64_t frames_paced;
    uint64_t resync_late_count;
    uint64_t slip_resync_count;
    uint64_t divisor_switches;
} MaliFramePacer;

static inline bool mali_pacer_valid_period(uint64_t vsync_period_ns, bool lock_120hz) {
    if (lock_120hz) return vsync_period_ns >= 7500000ull && vsync_period_ns <= 9200000ull;
    return vsync_period_ns >= 4000000ull && vsync_period_ns <= 50000000ull;
}

static inline void mali_pacer_reset(MaliFramePacer *p) {
    if (!p) return;
    *p = (MaliFramePacer){0};
}

static inline uint32_t mali_pacer_divisor(uint64_t vsync_period_ns, int target_fps) {
    if (vsync_period_ns == 0 || target_fps <= 0) return 0;
    // Exact integer divisors on ~120 Hz panels (7.8 ms .. 9.0 ms VSync period).
    if (vsync_period_ns >= 7800000ull && vsync_period_ns <= 9000000ull) {
        if (target_fps >= 38 && target_fps <= 42) return 3;
        if (target_fps >= 28 && target_fps <= 32) return 4;
        if (target_fps >= 58 && target_fps <= 62) return 2;
        if (target_fps >= 18 && target_fps <= 22) return 6;
    }
    uint64_t target_period = 1000000000ull / (uint64_t)target_fps;
    uint64_t div = (target_period + vsync_period_ns / 2) / vsync_period_ns;
    return (uint32_t)(div < 1 ? 1 : div);
}

static inline void mali_pacer_observe_present(MaliFramePacer *p, uint32_t present_id, uint64_t actual_ns,
                                              uint64_t vsync_period_ns, bool lock_120hz) {
    if (!p) return;
    if (mali_pacer_valid_period(vsync_period_ns, lock_120hz))
        p->vsync_period_ns = vsync_period_ns;
    else if (lock_120hz && !mali_pacer_valid_period(p->vsync_period_ns, true))
        p->vsync_period_ns = 8333333ull;
    if (!actual_ns || actual_ns <= p->last_actual_ns) return;
    p->last_actual_ns = actual_ns;
    if (present_id) p->last_actual_id = present_id;
    uint64_t P = p->vsync_period_ns;
    if (!p->next_vsync_ns || !P) return;

    // Anti-snap-back on VSync slip: advance next_vsync_ns to actual_ns if the presented frame
    // slipped past next_vsync_ns + P/2, so the next frame targets actual_ns + step_ns
    // instead of snapping back to a 2*P gap.
    if (actual_ns > p->next_vsync_ns + P / 2) {
        p->next_vsync_ns = actual_ns;
        p->slip_resync_count++;
        return;
    }

    // Signed phase error of (actual_ns - next_vsync_ns) mod P in [-P/2, +P/2].
    uint64_t rem = (actual_ns >= p->next_vsync_ns)
                 ? (actual_ns - p->next_vsync_ns) % P
                 : (P - (p->next_vsync_ns - actual_ns) % P) % P;
    int64_t err = rem <= P / 2 ? (int64_t)rem : (int64_t)rem - (int64_t)P;
    int64_t mag = err < 0 ? -err : err;
    if (mag <= 250000) return;  // within 0.25 ms phase lock
    int64_t max_step = mag > (int64_t)(P / 3) ? (int64_t)(P / 3) : (int64_t)(P / 8);
    if (max_step < 1) max_step = 1;
    int64_t adj = err > max_step ? max_step : err < -max_step ? -max_step : err;
    int64_t next = (int64_t)p->next_vsync_ns + adj;
    if (next > 0) p->next_vsync_ns = (uint64_t)next;
}

static inline void mali_pacer_observe_actual(MaliFramePacer *p, uint64_t actual_ns, uint64_t vsync_period_ns) {
    mali_pacer_observe_present(p, p ? p->last_sent_id : 0, actual_ns, vsync_period_ns, false);
}

static inline uint64_t mali_pacer_next_present_ex(MaliFramePacer *p, uint64_t now_ns, uint64_t vsync_period_ns,
                                                  int target_fps, bool has_display_timing, uint32_t present_id,
                                                  bool lock_120hz) {
    (void)has_display_timing;
    if (!p || target_fps <= 0) return 0;
    uint64_t P = mali_pacer_valid_period(vsync_period_ns, lock_120hz)
               ? vsync_period_ns
               : (mali_pacer_valid_period(p->vsync_period_ns, lock_120hz) ? p->vsync_period_ns : 8333333ull);
    p->vsync_period_ns = P;
    uint32_t div = mali_pacer_divisor(P, target_fps);
    if (!div) return 0;
    if (p->last_divisor && p->last_divisor != div) p->divisor_switches++;
    p->last_divisor = div;
    if (present_id) p->last_sent_id = present_id;

    uint64_t step_ns = (uint64_t)div * P;
    uint64_t min_vsync = now_ns + P / 2;
    if (!p->next_vsync_ns) {
        uint64_t anchor = 0;
        if (p->last_actual_ns > 0 && p->last_actual_ns <= min_vsync) {
            uint64_t k = (min_vsync + P / 2 - p->last_actual_ns + P - 1) / P;
            anchor = p->last_actual_ns + k * P;
        } else {
            anchor = now_ns + step_ns;
        }
        p->next_vsync_ns = anchor;
    } else {
        uint64_t cand = p->next_vsync_ns + step_ns;
        // Clock/suspend jump guard
        if (cand > now_ns + 4 * step_ns) cand = now_ns + step_ns;
        // Anti-burst resync on late frame: jump cleanly forward to the nearest future integer VSync edge
        // so the following frame resumes at cand + step_ns without queuing a burst of 8.33 ms frames.
        if (cand < min_vsync) {
            uint64_t k = (min_vsync - cand + P - 1) / P;
            cand += k * P;
            p->resync_late_count++;
        }
        p->next_vsync_ns = cand;
    }
    p->frames_paced++;
    // Aim half a VSync before the target VSync edge (mid acceptance window).
    return p->next_vsync_ns > P / 2 ? p->next_vsync_ns - P / 2 : 0;
}

static inline uint64_t mali_pacer_next_present(MaliFramePacer *p, uint64_t now_ns, uint64_t vsync_period_ns,
                                               int target_fps, bool has_display_timing) {
    return mali_pacer_next_present_ex(p, now_ns, vsync_period_ns, target_fps, has_display_timing, 0, false);
}

// Compute the raw ADPF targetDurationNs before android.c applies its headroom margin (default 20%).
static inline uint64_t mali_pacer_adpf_target_ns(int eff_fps, int fps_mode, int env_fps, bool pace_slow,
                                                   bool follow, bool adpf30_override, bool is_mali,
                                                   bool adpf_relax, double adpf30_ms) {
    if ((env_fps < 0 && fps_mode == 3) || (env_fps > 0 && env_fps <= 20))
        return 37500000ull;
    bool between_active = (env_fps < 0 && fps_mode == 4) || (env_fps >= 35 && env_fps <= 50);
    if (between_active && eff_fps >= 35 && eff_fps <= 50) {
        uint64_t period_ns = 1000000000ull / (uint64_t)eff_fps;
        return (period_ns * 11ull) / 10ull;  // 27.50 ms raw -> 22.00 ms effective @ 40 FPS
    }
    bool is_30 = ((env_fps < 0 && fps_mode == 1) && !adpf30_override) ||
                 (env_fps > 0 && env_fps <= 32) ||
                 (between_active && eff_fps > 0 && eff_fps <= 32) ||
                 (env_fps < 0 && fps_mode == 0 && follow && pace_slow);
    if (is_30) {
        if (adpf30_ms > 0.0) return (uint64_t)(adpf30_ms * 1e6);
        return (!adpf_relax || is_mali) ? 25000000ull : 37000000ull;  // upstream 25 ms default unless opt-in relax
    }
    if (adpf_relax && !is_mali && ((env_fps == 60) || (env_fps < 0 && fps_mode == 2)))
        return 19500000ull;  // 15.60 ms effective @ fixed 60 FPS on non-Mali when opt-in relax enabled
    return 16666667ull;
}

// Adaptive Late Input Latching for 40 FPS (1:3 @ 120.00 Hz) mode:
// Converts idle pre-VSync frame headroom into lower stick/gyro-to-photon latency with asymmetric
// fast-attack/slow-decay tracking and instantaneous safety clamps on spikes, compiles, or slips.
typedef struct {
    double ewma_cost_ms;
    double current_offset_ms;
    uint32_t cooldown_frames;
    uint64_t total_frames;
    uint64_t active_frames;
    uint64_t panic_clamps;
    double sum_offset_ms;
} LateLatchPacer;

static inline void late_latch_reset(LateLatchPacer *l, uint32_t cooldown_frames, bool count_panic) {
    if (!l) return;
    l->current_offset_ms = 0.0;
    if (l->cooldown_frames < cooldown_frames) l->cooldown_frames = cooldown_frames;
    if (count_panic) l->panic_clamps++;
}

static inline uint64_t late_latch_update(LateLatchPacer *l, bool eligible, double frame_period_ms,
                                         double active_cost_ms, bool had_compile_or_skip, bool pacer_slipped,
                                         double safety_guard_ms, double max_offset_ms) {
    if (!l) return 0;
    if (!eligible || frame_period_ms < 23.0 || frame_period_ms > 27.0) {
        l->current_offset_ms = 0.0;
        if (l->cooldown_frames < 8u) l->cooldown_frames = 8u;
        return 0;
    }
    double guard = safety_guard_ms > 0.0 ? safety_guard_ms : 4.5;
    double cap = max_offset_ms > 0.0 ? max_offset_ms : 7.5;
    double cost = active_cost_ms > 0.0 ? active_cost_ms : 0.0;

    l->total_frames++;
    if (l->ewma_cost_ms <= 0.0) {
        l->ewma_cost_ms = cost;
    } else if (cost > l->ewma_cost_ms) {
        l->ewma_cost_ms += (cost - l->ewma_cost_ms) * 0.50;
    } else {
        l->ewma_cost_ms += (cost - l->ewma_cost_ms) * 0.05;
    }

    if (had_compile_or_skip || pacer_slipped || frame_period_ms >= 26.0 || cost >= 21.5) {
        late_latch_reset(l, 16u, true);
        return 0;
    }
    if (l->cooldown_frames > 0u) {
        l->cooldown_frames--;
        l->current_offset_ms = 0.0;
        return 0;
    }

    double worst = l->ewma_cost_ms > cost ? l->ewma_cost_ms : cost;
    double want = 25.0 - worst - guard;
    if (want < 0.0) want = 0.0;
    if (want > cap) want = cap;

    if (want <= l->current_offset_ms) {
        l->current_offset_ms = want;
    } else {
        double next = l->current_offset_ms + 0.25;
        l->current_offset_ms = next < want ? next : want;
    }
    if (l->current_offset_ms > 0.0) {
        l->active_frames++;
        l->sum_offset_ms += l->current_offset_ms;
    }
    return (uint64_t)(l->current_offset_ms * 1e6 + 0.5);
}
