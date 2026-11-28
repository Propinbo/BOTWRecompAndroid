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

    // Adreno 830 regression guard: keep nouboopt OFF on a830, ON only on a840..a899
    if (vk_adreno_needs_nouboopt("Adreno (TM) 830")) return 22;
    if (!vk_adreno_needs_nouboopt("Adreno (TM) 840")) return 23;
    if (vk_adreno_needs_nouboopt("Mali-G925")) return 24;

    // Turnip-only GPL gate (strictly off on Mali/unknown, respects env override when supported)
    if (!vk_gpl_want(true, 18u, true, 0)) return 25;
    if (vk_gpl_want(true, 0u, true, 0)) return 26;
    if (vk_gpl_want(true, 18u, false, 0)) return 27;
    if (vk_gpl_want(false, 18u, true, "1")) return 28;
    if (vk_gpl_want(true, 18u, true, "0")) return 29;
    if (!vk_gpl_want(true, 99u, true, "1")) return 30;

    // VkImageFormatListCreateInfo / sRGB-UNORM twin policy (Mali AFBC + Turnip UBWC)
    if (vk_format_srgb_twin(VK_FORMAT_R8G8B8A8_UNORM) != VK_FORMAT_R8G8B8A8_SRGB) return 31;
    if (vk_format_srgb_twin(VK_FORMAT_R8G8B8A8_SRGB) != VK_FORMAT_R8G8B8A8_UNORM) return 32;
    if (vk_format_srgb_twin(VK_FORMAT_B8G8R8A8_UNORM) != VK_FORMAT_B8G8R8A8_SRGB) return 33;
    if (vk_format_srgb_twin(VK_FORMAT_A8B8G8R8_SRGB_PACK32) != VK_FORMAT_A8B8G8R8_UNORM_PACK32) return 34;
    if (vk_format_srgb_twin(VK_FORMAT_B10G11R11_UFLOAT_PACK32) != VK_FORMAT_UNDEFINED) return 35;
    const char *why = 0;
    if (!vk_rt_format_list_want(true, false, 0, 0, &why)) return 36;
    if (!vk_rt_format_list_want(false, true, 0, 0, &why)) return 37;
    if (vk_rt_format_list_want(false, false, 0, 0, &why)) return 38;
    if (!vk_rt_format_list_want(false, false, 0, "1", &why)) return 39;
    if (vk_rt_format_list_want(true, true, "0", "1", &why)) return 40;
    if (!vk_rt_format_list_want(false, false, "1", 0, &why)) return 41;
    if (!vk_rt_use_format_list(true, false, 8u, VK_FORMAT_R8G8B8A8_SRGB, true)) return 42;
    if (vk_rt_use_format_list(true, false, 8u, VK_FORMAT_R8G8B8A8_SRGB, false)) return 43;
    if (!vk_rt_use_format_list(true, false, 1u, VK_FORMAT_UNDEFINED, false)) return 44;
    if (vk_rt_use_format_list(true, true, 8u, VK_FORMAT_UNDEFINED, true)) return 45;
    if (vk_rt_use_format_list(true, false, 0u, VK_FORMAT_R8G8B8A8_SRGB, true)) return 46;
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
  console.log('16 Vulkan compatibility checks passed (production C)');
}).catch(e => {console.error(e); process.exit(1);});
''')
subprocess.run(['node', str(out / 'run.cjs'), str(out / 'test.wasm')], check=True)
