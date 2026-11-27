// Output resolution (runtime/nvn/output_scale.c), shared by the Metal and Vulkan backends.
#pragma once
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    double sx, sy;             // the final image's render-target scale (relative to the game's 1920x1080 sizes)
    double scene_sx, scene_sy; // the 3D scene's render targets' scale (vulkan/textures.c RS_SCENE)
    double shape;              // window aspect / 16:9 when the camera is widened, else 1
    double vx, vy, vw, vh;     // where the image is drawn in the window (pixels)
} OutScale;

// dw x dh: the window's drawable size in pixels; now: seconds; gpu_ms: GPU time of the last frame (< 0: unknown)
void output_scale(double dw, double dh, bool reshape_allowed, double now, double gpu_ms, OutScale *o);
double output_scale_dynamic_factor(void);

// Parse LARECOMP_RENDER_SCALE (e.g. "68" -> 0.68, or "0.68" -> 0.68). Returns 0.0 when unset/invalid/out of range.
static inline double output_scale_parse_render_scale(const char *env) {
    if (!env || !*env) return 0.0;
    char *end = NULL;
    double v = strtod(env, &end);
    if (end == env || !isfinite(v) || v <= 0.0) return 0.0;
    if (v > 2.0) v /= 100.0;
    if (v < 0.1 || v > 2.0) return 0.0;
    return v;
}

// Snap the 3D scene scale so BOTW's 1600x900 base dimensions land on exact 16-pixel multiples.
// Adjusts scene_sx / scene_sy directly so Track 4 full-extent scissor checks and AFBC/mip pyramids stay aligned.
static inline void output_scale_snap_scene_16(double *scene_sx, double *scene_sy) {
    if (!scene_sx || !scene_sy) return;
    double raw_w = (*scene_sx) * 1600.0;
    double raw_h = (*scene_sy) * 900.0;
    double aw = floor((raw_w + 8.0) / 16.0) * 16.0;
    double ah = floor((raw_h + 8.0) / 16.0) * 16.0;
    if (aw < 16.0) aw = 16.0;
    if (ah < 16.0) ah = 16.0;
    *scene_sx = aw / 1600.0;
    *scene_sy = ah / 900.0;
}
