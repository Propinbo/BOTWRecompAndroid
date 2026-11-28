#!/usr/bin/env python3
"""Check Android Vulkan preflight decisions with production C on the PC (NDK + Node)."""
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
out = root / 'build' / 'vulkan-compat-tests'
out.mkdir(parents=True, exist_ok=True)
bin_dir = Path(sys.argv[1])
features = [
    ('v12', 'timelineSemaphore'),
    ('v12', 'samplerMirrorClampToEdge'),
    ('v12', 'scalarBlockLayout'),
    ('v13', 'maintenance4'),
]
source = '''
#include "device_compat.h"
void *memset(void *p, int c, __SIZE_TYPE__ n) {
    unsigned char *b = p;
    for (__SIZE_TYPE__ i = 0; i < n; i++) b[i] = (unsigned char)c;
    return p;
}
static int equal(const char *a, const char *b) {
    if (!a) return 0;
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}
int check(void) {
    VkPhysicalDeviceVulkan11Features v11 = {0};
    VkPhysicalDeviceVulkan12Features v12 = {0};
    VkPhysicalDeviceVulkan13Features v13 = {0};
'''
source += '\n'.join(f'    {group}.{field} = VK_TRUE;' for group, field in features)
source += '''
    // Optional 8-bit, 16-bit storage (v11: the Adreno 610 under Turnip), dynamic rendering, synchronization2,
    // demote and terminate (v13: other ways, device.c) and int64 flags are absent: do not reject this at preflight.
    if (vk_missing_backend_feature(&v11, &v12, &v13)) return 1;
'''
for index, (group, field) in enumerate(features, 2):
    source += f'''
    {group}.{field} = VK_FALSE;
    if (!equal(vk_missing_backend_feature(&v11, &v12, &v13), "{field}")) return {index};
    {group}.{field} = VK_TRUE;
'''
source += '''
    // Extension presence alone must not permit the formatless custom-border path.
    VkPhysicalDeviceCustomBorderColorFeaturesEXT border = {0};
    if (vk_backend_border_supported(&border)) return 11;
    border.customBorderColors = VK_TRUE;
    if (vk_backend_border_supported(&border)) return 12;
    border.customBorderColors = VK_FALSE;
    border.customBorderColorWithoutFormat = VK_TRUE;
    if (vk_backend_border_supported(&border)) return 13;
    border.customBorderColors = VK_TRUE;
    if (!vk_backend_border_supported(&border)) return 14;
    VkQueueFamilyProperties queue = {.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, .queueCount = 1};
    if (!vk_backend_queue_supported(&queue, VK_SUCCESS, VK_TRUE)) return 15;
    queue.queueCount = 0;
    if (vk_backend_queue_supported(&queue, VK_SUCCESS, VK_TRUE)) return 16;
    queue.queueCount = 1; queue.queueFlags = VK_QUEUE_GRAPHICS_BIT;
    if (vk_backend_queue_supported(&queue, VK_SUCCESS, VK_TRUE)) return 17;
    queue.queueFlags = VK_QUEUE_COMPUTE_BIT;
    if (vk_backend_queue_supported(&queue, VK_SUCCESS, VK_TRUE)) return 18;
    queue.queueFlags = VK_QUEUE_TRANSFER_BIT;
    if (vk_backend_queue_supported(&queue, VK_SUCCESS, VK_TRUE)) return 19;
    queue.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
    if (vk_backend_queue_supported(&queue, VK_SUCCESS, VK_FALSE)) return 20;
    if (vk_backend_queue_supported(&queue, VK_ERROR_SURFACE_LOST_KHR, VK_TRUE)) return 21;

    // Aniso/LOD override gate: only 3D non-compare mipmapped linear samplers (maxLod > 1)
    if (!vk_sampler_aniso_override_ok(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, 8.0f, VK_FALSE, false)) return 22;
    if (!vk_sampler_aniso_override_ok(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, 8.0f, VK_FALSE, true)) return 23;
    if (vk_sampler_aniso_override_ok(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, 1.0f, VK_FALSE, false)) return 24;
    if (vk_sampler_aniso_override_ok(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, 8.0f, VK_TRUE, true)) return 25;

    // BOTW shadow cascade & gather target detection
    if (!vk_is_botw_shadow_target(4, 896, 896, 3, 1, 0x32)) return 26;
    if (!vk_is_botw_shadow_target(4, 448, 448, 3, 1, 0x6)) return 27;
    if (vk_is_botw_shadow_target(4, 1920, 1080, 1, 1, 0x3d)) return 28;

    // BOTW 800x450 half-scene & 400x225 quarter-res SSAO target detection
    if (!vk_is_botw_half_scene_target(1, 800, 450, 1, 8)) return 29;
    if (!vk_is_botw_half_scene_target(4, 800, 450, 1, 8)) return 30;
    if (vk_is_botw_half_scene_target(1, 800, 450, 1, 9)) return 31;
    if (vk_is_botw_half_scene_target(1, 800, 450, 2, 8)) return 32;
    if (vk_is_botw_half_scene_target(1, 640, 360, 1, 8)) return 33;
    if (vk_is_botw_half_scene_target(1, 960, 540, 1, 8)) return 34;
    if (vk_is_botw_half_scene_target(1, 1600, 900, 1, 8)) return 35;
    if (!vk_is_botw_ssao_quarter_target(1, 400, 225, 1, 0x1)) return 36;
    if (vk_is_botw_ssao_quarter_target(4, 400, 225, 1, 0x1)) return 37;
    if (vk_is_botw_ssao_quarter_target(1, 400, 225, 1, 0x25)) return 38;
    if (vk_is_botw_ssao_quarter_target(1, 800, 450, 1, 0x1)) return 39;
    if (vk_is_botw_ssao_quarter_target(1, 400, 225, 2, 0x1)) return 40;

    // 10-bit SDR swapchain format selection & clean fallback
    const VkSurfaceFormatKHR odin_fmts[] = {
        {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_COLOR_SPACE_HDR10_ST2084_EXT},
        {VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
    };
    VkSurfaceFormatKHR pick10 = vk_pick_swapchain_format(odin_fmts, 4, true);
    if (pick10.format != VK_FORMAT_A2B10G10R10_UNORM_PACK32 || pick10.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return 41;
    VkSurfaceFormatKHR pick8 = vk_pick_swapchain_format(odin_fmts, 4, false);
    if (pick8.format != VK_FORMAT_B8G8R8A8_UNORM || pick8.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return 42;
    const VkSurfaceFormatKHR no10_fmts[] = {
        {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
    };
    VkSurfaceFormatKHR fb8 = vk_pick_swapchain_format(no10_fmts, 2, true);
    if (fb8.format != VK_FORMAT_B8G8R8A8_UNORM || fb8.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return 43;
    const VkSurfaceFormatKHR rgba_only[] = {{VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
    VkSurfaceFormatKHR fb_rgba = vk_pick_swapchain_format(rgba_only, 1, true);
    if (fb_rgba.format != VK_FORMAT_R8G8B8A8_UNORM) return 44;
    const VkSurfaceFormatKHR undef_only[] = {{VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
    VkSurfaceFormatKHR fb_undef = vk_pick_swapchain_format(undef_only, 1, true);
    if (fb_undef.format != VK_FORMAT_B8G8R8A8_UNORM) return 45;
    const VkSurfaceFormatKHR hdr_only_10[] = {
        {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_COLOR_SPACE_HDR10_ST2084_EXT},
    };
    VkSurfaceFormatKHR no_pq = vk_pick_swapchain_format(hdr_only_10, 2, true);
    if (no_pq.format != VK_FORMAT_R8G8B8A8_UNORM || no_pq.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return 46;
    VkSurfaceFormatKHR fb_empty = vk_pick_swapchain_format(NULL, 0, true);
    if (fb_empty.format != VK_FORMAT_B8G8R8A8_UNORM) return 47;

    // 10-bit FP16 display RT promotion & IEC 61966-2-1 OETF flag packing
    if (!vk_format_is_srgb(VK_FORMAT_R8G8B8A8_SRGB)) return 48;
    if (!vk_format_is_srgb(VK_FORMAT_B8G8R8A8_SRGB)) return 49;
    if (vk_format_is_srgb(VK_FORMAT_R8G8B8A8_UNORM)) return 50;
    if (!vk_display_promote_fp16_ok(VK_FORMAT_A2B10G10R10_UNORM_PACK32, false, VK_FORMAT_UNDEFINED, false, 1u, 0x25u, 1u, true)) return 51;
    if (vk_display_promote_fp16_ok(VK_FORMAT_B8G8R8A8_UNORM, false, VK_FORMAT_UNDEFINED, false, 1u, 0x25u, 1u, true)) return 52;
    if (vk_display_promote_fp16_ok(VK_FORMAT_A2B10G10R10_UNORM_PACK32, false, VK_FORMAT_UNDEFINED, false, 8u, 0x25u, 1u, true)) return 53;
    if (vk_display_promote_fp16_ok(VK_FORMAT_A2B10G10R10_UNORM_PACK32, false, VK_FORMAT_UNDEFINED, false, 1u, 0x3du, 1u, true)) return 54;
    if (vk_display_promote_fp16_ok(VK_FORMAT_A2B10G10R10_UNORM_PACK32, false, VK_FORMAT_UNDEFINED, false, 1u, 0x25u, 2u, true)) return 55;
    if (vk_display_promote_fp16_ok(VK_FORMAT_A2B10G10R10_UNORM_PACK32, false, VK_FORMAT_UNDEFINED, false, 1u, 0x25u, 1u, false)) return 56;
    if (vk_present_flags_pack(false, false) != 0.0f) return 57;
    if (vk_present_flags_pack(true, false) != 1.0f) return 58;
    if (vk_present_flags_pack(false, true) != 2.0f) return 59;
    if (vk_present_flags_pack(true, true) != 3.0f) return 60;
    return 0;
}
'''
(out / 'test.c').write_text(source)
subprocess.run([str(bin_dir / 'clang.exe'), '--target=wasm32', '-ffreestanding', '-O2', '-nostdlib',
                '-I', str(root / 'runtime/nvn/vulkan'), '-I', str(root / 'android/deps/out/include'),
                '-c', str(out / 'test.c'), '-o', str(out / 'test.o')], check=True)
subprocess.run([str(bin_dir / 'ld.lld.exe'), '-flavor', 'wasm', '--no-entry', '--export=check',
                str(out / 'test.o'), '-o', str(out / 'test.wasm')], check=True)
(out / 'run.cjs').write_text('''
const fs = require('fs');
WebAssembly.instantiate(fs.readFileSync(process.argv[2])).then(({instance}) => {
  const code = instance.exports.check();
  if (code) throw new Error(`Vulkan preflight check ${code} failed`);
  console.log('Vulkan compatibility checks passed (production C)');
}).catch(e => {console.error(e); process.exit(1);});
''')
subprocess.run(['node', str(out / 'run.cjs'), str(out / 'test.wasm')], check=True)
