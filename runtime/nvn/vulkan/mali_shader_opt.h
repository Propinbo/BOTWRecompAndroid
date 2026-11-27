// Policy helpers for shader/SPIR-V cleanup and no-op blend LRZ optimization.
// Included from device.c, exec.c, and unit tests.
#pragma once

#include <stdbool.h>

#include "device_compat.h"

static inline bool vk_mali_spirv_cleanup_want(bool has_bc, bool has_astc_ldr, bool is_turnip,
                                              const char *env, const char **why_out) {
    bool mali_gate = !has_bc && has_astc_ldr;
    if (!mali_gate && !is_turnip) {
        if (why_out) *why_out = has_bc ? "BC supported (!BC && ASTC gate)" : "no ASTC_LDR (!BC && ASTC gate)";
        return false;
    }
    bool env_val = false;
    if (vk_compat_parse_env_bool(env, &env_val)) {
        if (why_out) *why_out = env_val ? "LARECOMP_MALI_SPIRV_CLEANUP=1" : "LARECOMP_MALI_SPIRV_CLEANUP=0";
        return env_val;
    }
    if (why_out) *why_out = mali_gate ? "!BC && ASTC default" : "Turnip default";
    return true;
}

static inline bool vk_mali_discard_opt_want(bool has_bc, bool has_astc_ldr, bool is_turnip,
                                            const char *env, const char **why_out) {
    bool mali_gate = !has_bc && has_astc_ldr;
    if (!mali_gate && !is_turnip) {
        if (why_out) *why_out = has_bc ? "BC supported (!BC && ASTC gate)" : "no ASTC_LDR (!BC && ASTC gate)";
        return false;
    }
    bool env_val = false;
    if (vk_compat_parse_env_bool(env, &env_val)) {
        if (why_out) *why_out = env_val ? "LARECOMP_MALI_DISCARD_OPT=1" : "LARECOMP_MALI_DISCARD_OPT=0";
        return env_val;
    }
    if (why_out) *why_out = mali_gate ? "!BC && ASTC default" : "Turnip default";
    return true;
}

static inline bool vk_noop_blend_opt_want(bool is_turnip, const char *env, const char **why_out) {
    if (!is_turnip) {
        if (why_out) *why_out = "non-Turnip default";
        return false;
    }
    bool env_val = false;
    if (vk_compat_parse_env_bool(env, &env_val)) {
        if (why_out) *why_out = env_val ? "LARECOMP_VK_NOOP_BLEND_OPT=1" : "LARECOMP_VK_NOOP_BLEND_OPT=0";
        return env_val;
    }
    if (why_out) *why_out = "Turnip default";
    return true;
}

static inline bool vk_blend_is_noop(uint8_t write_mask, VkBlendFactor sc, VkBlendFactor dc, VkBlendOp cop,
                                    VkBlendFactor sa, VkBlendFactor da, VkBlendOp aop) {
    if (write_mask == 0) return true;
    return sc == VK_BLEND_FACTOR_ONE && dc == VK_BLEND_FACTOR_ZERO && cop == VK_BLEND_OP_ADD &&
           sa == VK_BLEND_FACTOR_ONE && da == VK_BLEND_FACTOR_ZERO && aop == VK_BLEND_OP_ADD;
}
