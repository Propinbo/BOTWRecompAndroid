// Features already requested unconditionally by this backend. Diagnose them before vkCreateDevice.
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#if !defined(VK_VERSION_1_0)
#include <vulkan/vulkan_core.h>
#endif

#define VK_BACKEND_PIPELINE_CACHE_MAX (64u * 1024u * 1024u)
static inline uint32_t vk_backend_read_le32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
// Only the public header is checked; opaque driver payload integrity is not established here.
static inline VkBool32 vk_backend_cache_compatible(const uint8_t *p, size_t size, const VkPhysicalDeviceProperties *gpu) {
    if (!p || size < 32 || size > VK_BACKEND_PIPELINE_CACHE_MAX) return VK_FALSE;
    if (vk_backend_read_le32(p) != 32 || vk_backend_read_le32(p + 4) != VK_PIPELINE_CACHE_HEADER_VERSION_ONE ||
        vk_backend_read_le32(p + 8) != gpu->vendorID || vk_backend_read_le32(p + 12) != gpu->deviceID) return VK_FALSE;
    for (unsigned i = 0; i < VK_UUID_SIZE; i++) if (p[16 + i] != gpu->pipelineCacheUUID[i]) return VK_FALSE;
    return VK_TRUE;
}

static inline VkCompositeAlphaFlagBitsKHR vk_backend_composite_alpha(VkCompositeAlphaFlagsKHR supported) {
    const VkCompositeAlphaFlagBitsKHR preferred[] = {VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR};
    for (unsigned i = 0; i < sizeof preferred / sizeof *preferred; i++)
        if (supported & preferred[i]) return preferred[i];
    return (VkCompositeAlphaFlagBitsKHR)0;
}

static inline VkImageUsageFlags vk_backend_swapchain_usage(VkImageUsageFlags supported) {
    if (!(supported & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) return 0;
    // Rendering requires color attachment; transfer-source is only for window screenshots. No transfer-dst commands.
    return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (supported & VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
}

static inline VkBool32 vk_backend_queue_supported(const VkQueueFamilyProperties *queue, VkResult result, VkBool32 present) {
    const VkQueueFlags required = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
    return result == VK_SUCCESS && present && queue->queueCount && (queue->queueFlags & required) == required;
}

// Samplers use VK_FORMAT_UNDEFINED, so both bits are needed for the existing custom-border path.
static inline VkBool32 vk_backend_border_supported(const VkPhysicalDeviceCustomBorderColorFeaturesEXT *features) {
    return features->customBorderColors && features->customBorderColorWithoutFormat;
}

static inline const char *vk_missing_backend_feature(const VkPhysicalDeviceVulkan11Features *v11,
                                                     const VkPhysicalDeviceVulkan12Features *v12,
                                                     const VkPhysicalDeviceVulkan13Features *v13) {
    (void)v11;  // (16-bit storage is optional: no shader declares it, and the Adreno 610 under Turnip lacks it)
    if (!v12->timelineSemaphore) return "timelineSemaphore";
    if (!v12->samplerMirrorClampToEdge) return "samplerMirrorClampToEdge";
    if (!v12->scalarBlockLayout) return "scalarBlockLayout";
    // (dynamicRendering: optional - classic render passes without it, rpcompat.c; synchronization2, demote and
    // terminate optional too: device.c OPT_SYNC2 / OPT_DEMOTE / OPT_TERM)
    if (!v13->maintenance4) return "maintenance4";
    return 0;
}

static inline bool vk_compat_is_digit(char c) { return c >= '0' && c <= '9'; }
static inline bool vk_compat_is_alnum(char c) {
    return vk_compat_is_digit(c) || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// Parse ARM proprietary Mali major revision token ("v1.r49p1..." -> 49, "r38p1..." -> 38); -1 if absent.
static inline int vk_arm_driver_major(const char *info) {
    if (!info) return -1;
    for (const char *p = info; *p; p++) {
        if (*p != 'r') continue;
        if (p > info && vk_compat_is_alnum(p[-1])) continue;
        if (!vk_compat_is_digit(p[1])) continue;
        unsigned long ver = 0;
        const char *q = p + 1;
        while (vk_compat_is_digit(*q)) {
            if (ver < 100000ul) ver = ver * 10ul + (unsigned long)(*q - '0');
            q++;
        }
        if (*q == 'p' && vk_compat_is_digit(q[1])) return (int)ver;
    }
    return -1;
}

static inline const char *vk_compat_skip_ws(const char *s) {
    while (s && (*s == ' ' || (*s >= '\t' && *s <= '\r'))) s++;
    return s;
}

// Parse an optional integer env switch ("0" -> false, nonzero digits -> true). Rejects empty or trailing garbage.
static inline bool vk_compat_parse_env_bool(const char *s, bool *out) {
    if (!s || !out) return false;
    s = vk_compat_skip_ws(s);
    if (!*s) return false;
    if (*s == '+' || *s == '-') s++;
    if (!vk_compat_is_digit(*s)) return false;
    bool nonzero = false;
    while (vk_compat_is_digit(*s)) {
        if (*s != '0') nonzero = true;
        s++;
    }
    s = vk_compat_skip_ws(s);
    if (*s) return false;
    *out = nonzero;
    return true;
}

static inline bool vk_dyn_render_auto_default(uint32_t driver_id, const char *driver_info, bool have_driver_props) {
    return !(have_driver_props && driver_id == VK_DRIVER_ID_ARM_PROPRIETARY && vk_arm_driver_major(driver_info) >= 49);
}

// Default dynamic rendering off on ARM Mali proprietary r49+ (vkCmdBeginRendering triggers
// per-pass Scudo mmap/munmap churn in libGLES_mali.so; classic VkRenderPass avoids it).
// Precedence: valid LARECOMP_NO_DYNAMIC_RENDERING env > settings (0=off, 1=on, -1=auto) > driver auto default.
static inline bool vk_dyn_render_want(bool supported, uint32_t driver_id, const char *driver_info,
                                      bool have_driver_props, const char *no_dr_env, int setting,
                                      const char **reason_out) {
    if (!supported) {
        if (reason_out) *reason_out = "the driver lacks it";
        return false;
    }
    bool off = false;
    if (vk_compat_parse_env_bool(no_dr_env, &off)) {
        if (reason_out) *reason_out = off ? "LARECOMP_NO_DYNAMIC_RENDERING=1" : "LARECOMP_NO_DYNAMIC_RENDERING=0";
        return !off;
    }
    if (setting >= 0) {
        if (reason_out) *reason_out = "settings";
        return setting != 0;
    }
    if (!vk_dyn_render_auto_default(driver_id, driver_info, have_driver_props)) {
        if (reason_out) *reason_out = "Mali r49+ default";
        return false;
    }
    if (reason_out) *reason_out = "default";
    return true;
}

static inline int vk_compat_atoi(const char *s) {
    if (!s) return 0;
    while (*s == ' ' || *s == '\t') s++;
    int sign = 1;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    int v = 0;
    while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); s++; }
    return sign * v;
}

static inline const char *vk_compat_strstr(const char *hay, const char *needle) {
    if (!hay || !needle) return NULL;
    if (!*needle) return hay;
    for (; *hay; hay++) {
        const char *h = hay, *n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return hay;
    }
    return NULL;
}

// Enable IR3_SHADER_DEBUG=nouboopt only on Adreno 840..899 (Z Fold 8 red shadow workaround, #32);
// Adreno 830 renders shadows cleanly without it and loses ~1.5 ms / ~1.0 W if forced.
static inline bool vk_adreno_needs_nouboopt(const char *model) {
    const char *a = vk_compat_strstr(model, "Adreno");
    if (!a) return false;
    while (*a && (*a < '0' || *a > '9')) a++;
    int n = vk_compat_atoi(a);
    return n >= 840 && n < 900;
}

// Restrict forced anisotropic filtering and sharp LOD bias strictly to 3D mipmapped linear non-compare samplers,
// keeping HUD/UI textures (maxLod <= 1 or nearest) and shadow comparison samplers untouched.
static inline bool vk_sampler_aniso_override_ok(VkFilter min_f, VkFilter mag_f, VkSamplerMipmapMode mip_m,
                                                float max_lod, VkBool32 compare_enable, bool allow_mip_nearest) {
    if (compare_enable != VK_FALSE) return false;
    if (min_f != VK_FILTER_LINEAR || mag_f != VK_FILTER_LINEAR) return false;
    if (mip_m != VK_SAMPLER_MIPMAP_MODE_LINEAR && !allow_mip_nearest) return false;
    return max_lod > 1.0f;
}

// Identify BOTW sun shadow cascade array (896x896 D16 x 3 layers) and its gather downsample (448x448 R16 x 3 layers).
static inline bool vk_is_botw_shadow_target(uint32_t target, uint32_t width, uint32_t height, uint32_t layers,
                                            uint32_t samples, uint32_t format) {
    if (target != 4 || layers != 3 || samples > 1) return false;
    return (width == 896 && height == 896 && format == 0x32) ||
           (width == 448 && height == 448 && format == 0x6);
}

static inline bool vk_gpl_want(bool ext_and_fast_link_ok, uint32_t driver_id, bool has_driver_props, const char *env) {
    if (!ext_and_fast_link_ok) return false;
    if (env && *env) return vk_compat_atoi(env) != 0;
    return has_driver_props && driver_id == 18u;
}
