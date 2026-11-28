// Unit tests for Mali 40 FPS 120 Hz integer-divisor pacer, anti-burst resync, and 16 px aligned render scales.
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "nvn/output_scale.h"
#include "nvn/vulkan/mali_frame_pacer.h"

static void test_render_scale_parse(void) {
    assert(output_scale_parse_render_scale(NULL) == 0.0);
    assert(output_scale_parse_render_scale("") == 0.0);
    assert(output_scale_parse_render_scale("abc") == 0.0);
    assert(output_scale_parse_render_scale("-10") == 0.0);
    assert(output_scale_parse_render_scale("0") == 0.0);
    assert(output_scale_parse_render_scale("500") == 0.0);

    assert(fabs(output_scale_parse_render_scale("65") - 0.65) < 1e-9);
    assert(fabs(output_scale_parse_render_scale("68") - 0.68) < 1e-9);
    assert(fabs(output_scale_parse_render_scale("70") - 0.70) < 1e-9);
    assert(fabs(output_scale_parse_render_scale("75") - 0.75) < 1e-9);
    assert(fabs(output_scale_parse_render_scale("0.65") - 0.65) < 1e-9);
    assert(fabs(output_scale_parse_render_scale("0.68") - 0.68) < 1e-9);
    assert(fabs(output_scale_parse_render_scale("0.70") - 0.70) < 1e-9);
}

static void check_aligned_dims(double dw, double dh, double share, int exp_w, int exp_h) {
    double shape = (dw / dh) / (16.0 / 9.0);
    if (fabs(shape - 1.0) <= 0.005) shape = 1.0;
    double hs = (dh / 900.0) * share;
    double sx = hs * shape, sy = hs;
    output_scale_snap_scene_16(&sx, &sy);
    int w = (int)lround(sx * 1600.0);
    int h = (int)lround(sy * 900.0);
    assert(w == exp_w && h == exp_h);
    assert(w % 16 == 0 && h % 16 == 0);
    // Verify float32 scaling across full (1600x900), half (800x450), and quarter (400x225) targets
    // and confirm ceilf scissor extent invariant holds (>= extent).
    float fsx = (float)sx, fsy = (float)sy;
    const int gw[3] = {1600, 800, 400}, gh[3] = {900, 450, 225};
    for (int i = 0; i < 3; i++) {
        int tw = (int)lroundf((float)gw[i] * fsx);
        int th = (int)lroundf((float)gh[i] * fsy);
        int cw = (int)ceilf((float)gw[i] * fsx);
        int ch = (int)ceilf((float)gh[i] * fsy);
        assert(tw == w / (1 << i));
        assert(th == h / (1 << i));
        assert(cw >= tw && ch >= th);
    }
}

static void test_aligned_scales(void) {
    // SM-X730 2560x1600 (16:10)
    check_aligned_dims(2560.0, 1600.0, 0.65, 1664, 1040);
    check_aligned_dims(2560.0, 1600.0, 0.68, 1744, 1088);
    check_aligned_dims(2560.0, 1600.0, 0.70, 1792, 1120);
    check_aligned_dims(2560.0, 1600.0, 0.75, 1920, 1200);
    // 2560x1440 (16:9)
    check_aligned_dims(2560.0, 1440.0, 0.65, 1664, 944);
    check_aligned_dims(2560.0, 1440.0, 0.68, 1744, 976);
    check_aligned_dims(2560.0, 1440.0, 0.70, 1792, 1008);
    check_aligned_dims(2560.0, 1440.0, 0.75, 1920, 1088);
    // 1920x1080 (16:9, Odin 3 native panel)
    check_aligned_dims(1920.0, 1080.0, 0.65, 1248, 704);
    check_aligned_dims(1920.0, 1080.0, 0.68, 1312, 736);
    check_aligned_dims(1920.0, 1080.0, 0.70, 1344, 752);
    check_aligned_dims(1920.0, 1080.0, 0.85, 1632, 912);
}

static void test_pacer_divisors(void) {
    const uint64_t p120 = 8333333ull;
    const uint64_t p90 = 11111111ull;
    const uint64_t p60 = 16666667ull;
    assert(mali_pacer_divisor(0, 40) == 0);
    assert(mali_pacer_divisor(p120, 0) == 0);
    assert(mali_pacer_divisor(p120, 40) == 3);
    assert(mali_pacer_divisor(p120, 30) == 4);
    assert(mali_pacer_divisor(p120, 60) == 2);
    assert(mali_pacer_divisor(p120, 20) == 6);
    assert(mali_pacer_divisor(p90, 45) == 2);
    assert(mali_pacer_divisor(p90, 30) == 3);
    assert(mali_pacer_divisor(p60, 30) == 2);
}

static void test_pacer_steady_and_menu_transition(void) {
    const uint64_t P = 8333333ull;
    MaliFramePacer p = {0};
    uint64_t base = 100000000000ull;  // 100 s

    // Bootstrap first frame at 40 FPS (3 * P = 24,999,999 ns)
    uint64_t d0 = mali_pacer_next_present(&p, base, P, 40, true);
    assert(p.last_divisor == 3);
    assert(p.next_vsync_ns == base + 3 * P);
    assert(d0 == p.next_vsync_ns - P / 2);

    // Steady 40 FPS frames arrive ~25 ms apart
    uint64_t prev_d = d0;
    for (int i = 1; i < 10; i++) {
        uint64_t now = base + (uint64_t)i * 3 * P + 200000ull;
        uint64_t d = mali_pacer_next_present(&p, now, P, 40, true);
        assert(d - prev_d == 3 * P);
        prev_d = d;
    }
    assert(p.resync_late_count == 0);
    assert(p.divisor_switches == 0);

    // Menu transition to 30 FPS while panel stays at 120 Hz -> divisor 4 (33.333 ms)
    uint64_t now_menu = base + 10 * 3 * P + 200000ull;
    uint64_t dm = mali_pacer_next_present(&p, now_menu, P, 30, true);
    assert(p.last_divisor == 4);
    assert(p.divisor_switches == 1);
    assert(dm - prev_d == 4 * P);

    // Return to gameplay 40 FPS -> divisor 3 (25.000 ms)
    uint64_t now_game = now_menu + 4 * P;
    uint64_t dg = mali_pacer_next_present(&p, now_game, P, 40, true);
    assert(p.last_divisor == 3);
    assert(p.divisor_switches == 2);
    assert(dg - dm == 3 * P);
}

static void test_pacer_antiburst_and_phase_lock(void) {
    const uint64_t P = 8333333ull;
    MaliFramePacer p = {0};
    uint64_t t = 200000000000ull;

    uint64_t d0 = mali_pacer_next_present(&p, t, P, 40, true);
    (void)d0;
    uint64_t d1 = mali_pacer_next_present(&p, t + 3 * P, P, 40, true);
    uint64_t edge1 = p.next_vsync_ns;

    // Simulate a 65 ms render stall -> cand = edge1 + 3*P is far behind now + P/2
    uint64_t stall_now = t + 3 * P + 65000000ull;
    uint64_t d_late = mali_pacer_next_present(&p, stall_now, P, 40, true);
    assert(p.resync_late_count == 1);
    assert(p.next_vsync_ns >= stall_now + P / 2);
    assert(p.next_vsync_ns < stall_now + P / 2 + P);
    assert((p.next_vsync_ns - edge1) % P == 0);  // stayed on exact integer VSync lattice
    assert(d_late == p.next_vsync_ns - P / 2);

    // Next frame MUST be a full 3*P later (no 8.33 ms burst!)
    uint64_t d_next = mali_pacer_next_present(&p, stall_now + 20000000ull, P, 40, true);
    assert(d_next - d_late == 3 * P);
    assert(p.resync_late_count == 1);
    (void)d1;

    // Observe hardware actualPresentTime shifted by +1.8 ms -> next_vsync_ns steers smoothly toward hardware phase
    uint64_t before = p.next_vsync_ns;
    mali_pacer_observe_actual(&p, before + 1800000ull, P);
    assert(p.next_vsync_ns > before);
    assert(p.next_vsync_ns - before <= P / 8);

    // Giant +5 s jump (suspend) re-anchors cleanly without looping
    uint64_t suspend_now = stall_now + 5000000000ull;
    uint64_t d_wake = mali_pacer_next_present(&p, suspend_now, P, 40, true);
    assert(d_wake >= suspend_now);
    assert(p.next_vsync_ns <= suspend_now + 4 * 3 * P);

    // Uncapped target_fps == 0 returns 0
    assert(mali_pacer_next_present(&p, suspend_now, P, 0, true) == 0);
}

static void test_pacer_slip_no_snapback(void) {
    const uint64_t P = 8333333ull;
    MaliFramePacer p = {0};
    uint64_t t = 300000000000ull;

    // Frame 10 schedules V1 = t + 3*P with present_id=10
    uint64_t d10 = mali_pacer_next_present_ex(&p, t, P, 40, true, 10, true);
    uint64_t V1 = p.next_vsync_ns;
    assert(V1 == t + 3 * P);
    assert(d10 == V1 - P / 2);

    // Case 1: delta_frames == 0 (Frame 10 slipped +1 VSync late to V1 + P before Frame 11 was queued)
    mali_pacer_observe_present(&p, 10, V1 + P, P, true);
    assert(p.slip_resync_count == 1);
    assert(p.next_vsync_ns == V1 + P);

    // Frame 11 must target V1 + 4*P (3*P after V1 + P), NOT V1 + 3*P (no 2*P snap-back!)
    uint64_t d11 = mali_pacer_next_present_ex(&p, V1 + 2 * P, P, 40, true, 11, true);
    assert(p.next_vsync_ns == V1 + 4 * P);
    assert(d11 == V1 + 4 * P - P / 2);

    // Case 2: lock_120hz=true ignores transient 20.833 ms refresh period report
    mali_pacer_observe_present(&p, 10, V1 + P, 20833333ull, true);
    assert(p.vsync_period_ns == P);

    // Case 3: delta_frames == 1 (Frame 12 already in-flight for V1 + 7*P when Frame 11 slips +1*P to V1 + 5*P)
    uint64_t d12 = mali_pacer_next_present_ex(&p, V1 + 5 * P, P, 40, true, 12, true);
    assert(p.next_vsync_ns == V1 + 7 * P);
    (void)d12;
    uint64_t slips_before = p.slip_resync_count;
    mali_pacer_observe_present(&p, 11, V1 + 5 * P, P, true);
    assert(p.slip_resync_count == slips_before);
    assert(p.next_vsync_ns == V1 + 7 * P);  // no double shift for in-flight frame
}

static void test_adpf_target_ns(void) {
    // 40 FPS mode -> 27.50 ms raw (22.00 ms effective after 20% Android ADPF margin) on both Mali and non-Mali
    assert(mali_pacer_adpf_target_ns(40, 4, -1, false, false, false, false, false, 0.0) == 27500000ull);
    assert(mali_pacer_adpf_target_ns(40, 4, -1, false, false, false, true, false, 0.0) == 27500000ull);
    assert(mali_pacer_adpf_target_ns(40, 0, 40, false, false, false, false, false, 0.0) == 27500000ull);
    assert(mali_pacer_adpf_target_ns(45, 4, -1, false, false, false, false, false, 0.0) == 24444444ull);
    // Menu/cutscene 30 FPS fallback inside 40 FPS mode: upstream 25 ms default, or 37 ms on non-Mali with relax
    assert(mali_pacer_adpf_target_ns(30, 4, -1, false, false, false, false, false, 0.0) == 25000000ull);
    assert(mali_pacer_adpf_target_ns(30, 4, -1, false, false, false, false, true, 0.0) == 37000000ull);
    assert(mali_pacer_adpf_target_ns(30, 0, 40, false, false, false, false, true, 0.0) == 37000000ull);
    assert(mali_pacer_adpf_target_ns(30, 4, -1, false, false, false, true, true, 0.0) == 25000000ull);
    // Fixed 30 FPS & 20 FPS: exact upstream behavior when relax=false
    assert(mali_pacer_adpf_target_ns(30, 1, -1, false, false, false, false, false, 0.0) == 25000000ull);
    assert(mali_pacer_adpf_target_ns(30, 1, -1, false, false, false, false, true, 0.0) == 37000000ull);
    assert(mali_pacer_adpf_target_ns(30, 1, -1, false, false, false, true, true, 0.0) == 25000000ull);
    assert(mali_pacer_adpf_target_ns(30, 1, -1, false, false, true, true, false, 0.0) == 16666667ull);
    assert(mali_pacer_adpf_target_ns(20, 3, -1, false, false, false, false, false, 0.0) == 37500000ull);
    // Fixed 60 FPS vs Auto: exact upstream 16.67 ms when relax=false, opt-in 19.5 ms on non-Mali when relax=true
    assert(mali_pacer_adpf_target_ns(60, 2, -1, false, false, false, false, false, 0.0) == 16666667ull);
    assert(mali_pacer_adpf_target_ns(60, 2, -1, false, false, false, false, true, 0.0) == 19500000ull);
    assert(mali_pacer_adpf_target_ns(60, 2, -1, false, false, false, true, true, 0.0) == 16666667ull);
    assert(mali_pacer_adpf_target_ns(60, 0, -1, false, false, false, false, true, 0.0) == 16666667ull);
}

static void test_late_latch_pacer(void) {
    LateLatchPacer l = {0};
    uint64_t wait_ns = 0;

    // 1. Steady 14.0 ms active cost in 40 FPS (25.0 ms window) ramps smoothly up to ~6.5 ms (25 - 14 - 4.5)
    for (int i = 0; i < 40; i++) {
        wait_ns = late_latch_update(&l, true, 25.0, 14.0, false, false, 4.5, 7.5);
    }
    assert(wait_ns >= 6450000ull && wait_ns <= 6550000ull);
    assert(l.panic_clamps == 0);

    // 2. Sudden spike to 19.5 ms immediately cuts offset down (25 - 19.5 - 4.5 = 1.0 ms), no slow decay trap
    wait_ns = late_latch_update(&l, true, 25.0, 19.5, false, false, 4.5, 7.5);
    assert(wait_ns <= 1050000ull);

    // 3. Panic clamp on overload (>= 21.5 ms), slow period (>= 26.0 ms), compile/skip, or slip -> instant 0 + cooldown
    assert(late_latch_update(&l, true, 25.0, 21.6, false, false, 4.5, 7.5) == 0);
    assert(l.current_offset_ms == 0.0 && l.cooldown_frames == 16u && l.panic_clamps == 1);
    // Still clamped during 16 cooldown frames even at low cost
    for (int i = 0; i < 16; i++) {
        assert(late_latch_update(&l, true, 25.0, 13.0, false, false, 4.5, 7.5) == 0);
    }
    // Frame 17 starts smooth ramp up by <= 0.25 ms/frame
    wait_ns = late_latch_update(&l, true, 25.0, 13.0, false, false, 4.5, 7.5);
    assert(wait_ns > 0 && wait_ns <= 260000ull);

    // Compile/skip or pacer slip forces panic clamp too
    assert(late_latch_update(&l, true, 25.0, 13.0, true, false, 4.5, 7.5) == 0);
    assert(l.panic_clamps == 2);
    for (int i = 0; i < 17; i++) late_latch_update(&l, true, 25.0, 13.0, false, false, 4.5, 7.5);
    assert(late_latch_update(&l, true, 26.1, 13.0, false, false, 4.5, 7.5) == 0);
    assert(l.panic_clamps == 3);

    // Ineligible (menu/event 30 FPS -> period 33.33 ms or eligible=false) zeroes wait immediately
    assert(late_latch_update(&l, false, 33.33, 12.0, false, false, 4.5, 7.5) == 0);
    assert(l.current_offset_ms == 0.0);
}

int main(void) {
    test_render_scale_parse();
    test_aligned_scales();
    test_pacer_divisors();
    test_pacer_steady_and_menu_transition();
    test_pacer_antiburst_and_phase_lock();
    test_pacer_slip_no_snapback();
    test_adpf_target_ns();
    test_late_latch_pacer();
    printf("PASS test_mali_40fps_pacer (all assertions)\n");
    return 0;
}
