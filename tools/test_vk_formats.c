// Vulkan format probe (no game, no app data), for issue #244: what a driver reports for the formats and features the
// renderer uses without checking, and what it really does in each of those undefined cases. Prints one JSON document.
// Build and run: tools/test_vk_formats.ps1. By hand: test_vk_formats [--driver <path to a Vulkan .so>] [--out <file.json>]
// Every group runs in its own process, so a driver that crashes or hangs in one case costs only that group, and the
// report says so. "expected" is what the spec gives where the format reports the feature; "feature_reported" says
// whether it does, so ok:false with feature_reported:false is the driver going wrong in the undefined case.
#define VK_NO_PROTOTYPES
#include <dlfcn.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/system_properties.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vulkan/vulkan.h>

#define INC(n) static const uint32_t n[] =
INC(vs_full)
#include "vs_full.inc"
;
INC(vs_tex)
#include "vs_tex.inc"
;
INC(vs_cmp)
#include "vs_cmp.inc"
;
INC(vs_point)
#include "vs_point.inc"
;
INC(vs_point1)
#include "vs_point1.inc"
;
INC(vs_atomic)
#include "vs_atomic.inc"
;
INC(fs_tex)
#include "fs_tex.inc"
;
INC(fs_cmp)
#include "fs_cmp.inc"
;
INC(fs_pass)
#include "fs_pass.inc"
;
INC(fs_const)
#include "fs_const.inc"
;
INC(cs_cmp)
#include "cs_cmp.inc"
;
#define SPV(n) n, sizeof n
#define COUNT(a) (sizeof(a) / sizeof *(a))

// --- What a group sends its parent: lines "<tag> <json>" (P passport, C caps, F format, R result, S skipped, E error)
static int report_fd = -1;
static const char *group_name = "";

static void emit(char tag, const char *fmt, ...) {
    char line[4096];
    int n = snprintf(line, sizeof line, "%c ", tag);
    va_list ap;
    va_start(ap, fmt);
    n += vsnprintf(line + n, sizeof line - n - 1, fmt, ap);
    va_end(ap);
    if (n > (int)sizeof line - 2) n = sizeof line - 2;
    line[n++] = '\n';
    if (write(report_fd, line, n) < 0) _exit(3);
}

static const char *json_str(const char *s) {
    static char rings[4][1024];
    static int ring;
    char *o = rings[ring++ % 4], *p = o;
    *p++ = '"';
    for (; *s && p < o + 1016; s++) {
        if (*s == '"' || *s == '\\') { *p++ = '\\'; *p++ = *s; }
        else if ((unsigned char)*s < 32) *p++ = ' ';
        else *p++ = *s;
    }
    *p++ = '"';
    *p = 0;
    return o;
}

static void die(const char *fmt, ...) {
    char m[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(m, sizeof m, fmt, ap);
    va_end(ap);
    emit('E', "%s", json_str(m));
    _exit(2);
}

static void skip_group(const char *why) {
    emit('S', "%s", json_str(why));
    _exit(0);
}

static const char *num(double v) {
    static char rings[4][32];
    static int ring;
    char *o = rings[ring++ % 4];
    if (isfinite(v)) snprintf(o, 32, "%.8g", v);
    else strcpy(o, "null");
    return o;
}

// reported: 1 / 0 = the feature this case leans on is / isn't reported, -1 = none is involved
static void result_field(const char *name, const char *want_json, double got, const char *ok_json, int reported, const char *note) {
    char tail[256] = "";
    int n = 0;
    if (reported >= 0) n += snprintf(tail + n, sizeof tail - n, ",\"feature_reported\":%s", reported ? "true" : "false");
    if (note) snprintf(tail + n, sizeof tail - n, ",\"note\":%s", json_str(note));
    emit('R', "{\"group\":\"%s\",\"case\":%s,\"expected\":%s,\"got\":%s,\"ok\":%s%s}", group_name, json_str(name), want_json, num(got), ok_json, tail);
}

static void result_num(const char *name, double want, double got, double tol, int reported) {
    result_field(name, num(want), got, isfinite(got) && fabs(got - want) <= tol ? "true" : "false", reported, NULL);
}

static void result_text(const char *name, const char *want, double got, bool ok, int reported, const char *note) {
    result_field(name, json_str(want), got, ok ? "true" : "false", reported, note);
}

static void result_unjudged(const char *name, double got, int reported, const char *note) {
    result_field(name, "null", got, "null", reported, note);
}

// --- The driver: through the system's loader by default; a given .so is called directly, as there is no loader to
// install it into from a plain process. That takes an ICD's entry points (Turnip has them), not those of an Android HAL.
#define FUNCS_INSTANCE(X) X(vkEnumeratePhysicalDevices) X(vkGetPhysicalDeviceMemoryProperties) X(vkGetPhysicalDeviceFeatures) X(vkGetPhysicalDeviceFeatures2) \
    X(vkGetPhysicalDeviceProperties) X(vkGetPhysicalDeviceProperties2) X(vkGetPhysicalDeviceQueueFamilyProperties) X(vkGetPhysicalDeviceFormatProperties2) \
    X(vkGetPhysicalDeviceImageFormatProperties) X(vkEnumerateDeviceExtensionProperties) X(vkCreateDevice) X(vkGetDeviceProcAddr)
#define FUNCS_DEVICE(X) X(vkGetDeviceQueue) X(vkCreateCommandPool) X(vkCreateDescriptorSetLayout) X(vkCreatePipelineLayout) X(vkCreateDescriptorPool) \
    X(vkAllocateCommandBuffers) X(vkFreeCommandBuffers) X(vkBeginCommandBuffer) X(vkEndCommandBuffer) X(vkQueueSubmit) X(vkQueueWaitIdle) X(vkDeviceWaitIdle) \
    X(vkCmdPipelineBarrier) X(vkCreateBuffer) X(vkDestroyBuffer) X(vkGetBufferMemoryRequirements) X(vkAllocateMemory) X(vkFreeMemory) X(vkBindBufferMemory) \
    X(vkMapMemory) X(vkCreateImage) X(vkGetImageMemoryRequirements) X(vkBindImageMemory) X(vkCreateImageView) X(vkCmdCopyBufferToImage) X(vkCmdCopyImageToBuffer) \
    X(vkCreateSampler) X(vkCreateShaderModule) X(vkDestroyShaderModule) X(vkCreateRenderPass) X(vkCreateFramebuffer) X(vkDestroyFramebuffer) \
    X(vkCreateGraphicsPipelines) X(vkCreateComputePipelines) X(vkAllocateDescriptorSets) X(vkUpdateDescriptorSets) X(vkCmdBeginRenderPass) X(vkCmdEndRenderPass) \
    X(vkCmdBindPipeline) X(vkCmdBindDescriptorSets) X(vkCmdSetViewport) X(vkCmdSetScissor) X(vkCmdPushConstants) X(vkCmdDraw) X(vkCmdDispatch)
#define DECLARE(f) static PFN_##f f;
FUNCS_INSTANCE(DECLARE)
FUNCS_DEVICE(DECLARE)

static PFN_vkGetInstanceProcAddr gipa;
static const char *loaded_via = "";

static void load_driver(const char *path) {
    void *h = dlopen(path ? path : "libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!h) die("dlopen: %s", dlerror());
    VkResult (VKAPI_PTR *nego)(uint32_t *) = (VkResult (VKAPI_PTR *)(uint32_t *))dlsym(h, "vk_icdNegotiateLoaderICDInterfaceVersion");
    if ((gipa = (PFN_vkGetInstanceProcAddr)dlsym(h, "vk_icdGetInstanceProcAddr"))) {
        uint32_t version = 5;
        if (nego) nego(&version);
        loaded_via = "vk_icdGetInstanceProcAddr";
    } else if ((gipa = (PFN_vkGetInstanceProcAddr)dlsym(h, "vkGetInstanceProcAddr"))) {
        loaded_via = "vkGetInstanceProcAddr";
    }
    if (!gipa) die("no Vulkan entry point (vk_icdGetInstanceProcAddr, vkGetInstanceProcAddr) in the library");
}

#define CK(x) do { VkResult r_ = (x); if (r_ != VK_SUCCESS) die("%s: VkResult %d (line %d)", #x, r_, __LINE__); } while (0)

static VkInstance inst;
static VkPhysicalDevice pd;
static VkPhysicalDeviceProperties props;
static VkDevice dev;
static VkQueue q;
static VkCommandPool pool;
static VkDescriptorSetLayout dsl;
static VkPipelineLayout pl;
static VkDescriptorPool dp;
static VkPhysicalDeviceMemoryProperties mp;

static uint32_t memtype(uint32_t bits, VkMemoryPropertyFlags want) {
    for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
        if ((bits >> i & 1) && (mp.memoryTypes[i].propertyFlags & want) == want) return i;
    die("no memory type with flags %#x", want);
    return 0;
}

static bool has_ext(const char *name) {
    uint32_t n = 0;
    vkEnumerateDeviceExtensionProperties(pd, NULL, &n, NULL);
    VkExtensionProperties *e = malloc(n * sizeof *e);
    vkEnumerateDeviceExtensionProperties(pd, NULL, &n, e);
    bool ok = false;
    for (uint32_t i = 0; i < n; i++) ok |= !strcmp(e[i].extensionName, name);
    free(e);
    return ok;
}

static void init_instance(void) {
    uint32_t version = VK_API_VERSION_1_3;
    PFN_vkEnumerateInstanceVersion enum_version = (PFN_vkEnumerateInstanceVersion)gipa(NULL, "vkEnumerateInstanceVersion");
    if (enum_version) {
        CK(enum_version(&version));
        if (version > VK_API_VERSION_1_3) version = VK_API_VERSION_1_3;
    }
    PFN_vkCreateInstance create = (PFN_vkCreateInstance)gipa(NULL, "vkCreateInstance");
    if (!create) die("no vkCreateInstance");
    VkApplicationInfo ai = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = version};
    VkInstanceCreateInfo ci = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pApplicationInfo = &ai};
    CK(create(&ci, NULL, &inst));
#define LOAD_I(f) if (!(f = (PFN_##f)gipa(inst, #f))) die("no %s", #f);
    FUNCS_INSTANCE(LOAD_I)
    uint32_t n = 1;
    VkResult r = vkEnumeratePhysicalDevices(inst, &n, &pd);
    if ((r != VK_SUCCESS && r != VK_INCOMPLETE) || n == 0) die("no physical device (VkResult %d): the driver found no GPU it can open from this process", r);
    vkGetPhysicalDeviceProperties(pd, &props);
    vkGetPhysicalDeviceMemoryProperties(pd, &mp);
}

static void init_device(bool m5) {
    VkPhysicalDeviceFeatures have;
    vkGetPhysicalDeviceFeatures(pd, &have);
    float pr = 1;
    uint32_t nq = 16, fam = 0;
    VkQueueFamilyProperties qp[16];
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &nq, qp);
    for (uint32_t i = 0; i < nq; i++)
        if ((qp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (qp[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) { fam = i; break; }
    VkDeviceQueueCreateInfo qi = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = fam, .queueCount = 1, .pQueuePriorities = &pr};
    VkPhysicalDeviceMaintenance5FeaturesKHR m5f = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR, .maintenance5 = VK_TRUE};
    VkPhysicalDeviceFeatures en = {.fragmentStoresAndAtomics = have.fragmentStoresAndAtomics, .vertexPipelineStoresAndAtomics = have.vertexPipelineStoresAndAtomics};
    const char *ext[] = {VK_KHR_MAINTENANCE_5_EXTENSION_NAME};
    VkDeviceCreateInfo di = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .pNext = m5 ? &m5f : NULL, .queueCreateInfoCount = 1, .pQueueCreateInfos = &qi,
                             .enabledExtensionCount = m5 ? 1 : 0, .ppEnabledExtensionNames = ext, .pEnabledFeatures = &en};
    CK(vkCreateDevice(pd, &di, NULL, &dev));
#define LOAD_D(f) if (!(f = (PFN_##f)vkGetDeviceProcAddr(dev, #f))) die("no %s", #f);
    FUNCS_DEVICE(LOAD_D)
    vkGetDeviceQueue(dev, fam, 0, &q);
    VkCommandPoolCreateInfo pi = {.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, .queueFamilyIndex = fam};
    CK(vkCreateCommandPool(dev, &pi, NULL, &pool));
    VkDescriptorSetLayoutBinding b[2] = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, NULL},
                                         {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, NULL}};
    VkDescriptorSetLayoutCreateInfo li = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .bindingCount = 2, .pBindings = b};
    CK(vkCreateDescriptorSetLayout(dev, &li, NULL, &dsl));
    VkPushConstantRange pc = {VK_SHADER_STAGE_ALL, 0, 32};
    VkPipelineLayoutCreateInfo pli = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 1, .pSetLayouts = &dsl, .pushConstantRangeCount = 1, .pPushConstantRanges = &pc};
    CK(vkCreatePipelineLayout(dev, &pli, NULL, &pl));
    VkDescriptorPoolSize ps[2] = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 400}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 400}};
    VkDescriptorPoolCreateInfo dpi = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .maxSets = 400, .poolSizeCount = 2, .pPoolSizes = ps};
    CK(vkCreateDescriptorPool(dev, &dpi, NULL, &dp));
}

static VkCommandBuffer cb_begin(void) {
    VkCommandBufferAllocateInfo ai = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = pool, .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount = 1};
    VkCommandBuffer cb;
    CK(vkAllocateCommandBuffers(dev, &ai, &cb));
    VkCommandBufferBeginInfo bi = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    CK(vkBeginCommandBuffer(cb, &bi));
    return cb;
}

static void cb_end(VkCommandBuffer cb) {
    CK(vkEndCommandBuffer(cb));
    VkSubmitInfo si = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &cb};
    CK(vkQueueSubmit(q, 1, &si, VK_NULL_HANDLE));
    CK(vkQueueWaitIdle(q));
    vkFreeCommandBuffers(dev, pool, 1, &cb);
}

static void full_barrier(VkCommandBuffer cb) {
    VkMemoryBarrier m = {.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                         .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
                         .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT};
    vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &m, 0, NULL, 0, NULL);
}

typedef struct { VkBuffer b; VkDeviceMemory m; void *p; } Buf;
static Buf buf_make(VkDeviceSize n) {
    Buf r;
    VkBufferCreateInfo ci = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = n,
                             .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    CK(vkCreateBuffer(dev, &ci, NULL, &r.b));
    VkMemoryRequirements mr;
    vkGetBufferMemoryRequirements(dev, r.b, &mr);
    VkMemoryAllocateInfo ai = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .allocationSize = mr.size,
                               .memoryTypeIndex = memtype(mr.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)};
    CK(vkAllocateMemory(dev, &ai, NULL, &r.m));
    CK(vkBindBufferMemory(dev, r.b, r.m, 0));
    CK(vkMapMemory(dev, r.m, 0, VK_WHOLE_SIZE, 0, &r.p));
    memset(r.p, 0, n);
    return r;
}

typedef struct { VkImage i; VkDeviceMemory m; VkImageView v; VkFormat f; VkImageAspectFlags a; } Img;

// Creating an image with a usage the driver refuses would be undefined in itself, not the case under test
static bool img_possible(VkFormat f, VkImageUsageFlags usage) {
    VkImageFormatProperties ip;
    return vkGetPhysicalDeviceImageFormatProperties(pd, f, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usage, 0, &ip) == VK_SUCCESS;
}

static Img img_make(VkFormat f, uint32_t w, uint32_t h, VkImageUsageFlags usage) {
    Img r = {.f = f};
    bool depth = f == VK_FORMAT_D16_UNORM || f == VK_FORMAT_D32_SFLOAT;
    r.a = depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageCreateInfo ci = {.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, .imageType = VK_IMAGE_TYPE_2D, .format = f, .extent = {w, h, 1},
                            .mipLevels = 1, .arrayLayers = 1, .samples = VK_SAMPLE_COUNT_1_BIT, .tiling = VK_IMAGE_TILING_OPTIMAL, .usage = usage};
    CK(vkCreateImage(dev, &ci, NULL, &r.i));
    VkMemoryRequirements mr;
    vkGetImageMemoryRequirements(dev, r.i, &mr);
    VkMemoryAllocateInfo ai = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .allocationSize = mr.size, .memoryTypeIndex = memtype(mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};
    CK(vkAllocateMemory(dev, &ai, NULL, &r.m));
    CK(vkBindImageMemory(dev, r.i, r.m, 0));
    VkImageViewCreateInfo vi = {.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = r.i, .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = f,
                                .subresourceRange = {r.a, 0, 1, 0, 1}};
    CK(vkCreateImageView(dev, &vi, NULL, &r.v));
    VkCommandBuffer cb = cb_begin();
    VkImageMemoryBarrier b = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, .srcAccessMask = 0, .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
                              .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_GENERAL, .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                              .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .image = r.i, .subresourceRange = {r.a, 0, 1, 0, 1}};
    vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &b);
    cb_end(cb);
    return r;
}

static void img_upload(const Img *im, const void *data, VkDeviceSize n, uint32_t w, uint32_t h) {
    Buf s = buf_make(n);
    memcpy(s.p, data, n);
    VkCommandBuffer cb = cb_begin();
    VkBufferImageCopy c = {.imageSubresource = {im->a, 0, 0, 1}, .imageExtent = {w, h, 1}};
    vkCmdCopyBufferToImage(cb, s.b, im->i, VK_IMAGE_LAYOUT_GENERAL, 1, &c);
    full_barrier(cb);
    cb_end(cb);
    vkDestroyBuffer(dev, s.b, NULL);
    vkFreeMemory(dev, s.m, NULL);
}

static VkSampler sampler(VkFilter fl, bool cmp) {
    VkSamplerCreateInfo si = {.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .magFilter = fl, .minFilter = fl, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
                              .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                              .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .compareEnable = cmp, .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL, .maxLod = 0.25f};
    VkSampler s;
    CK(vkCreateSampler(dev, &si, NULL, &s));
    return s;
}

static VkShaderModule mod(const uint32_t *c, size_t n) {
    VkShaderModuleCreateInfo ci = {.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = n, .pCode = c};
    VkShaderModule m;
    CK(vkCreateShaderModule(dev, &ci, NULL, &m));
    return m;
}

// Plain render passes: a driver without dynamic rendering can still answer
static VkRenderPass render_pass(VkFormat f) {
    static struct { VkFormat f; VkRenderPass rp; } made[8];
    static uint32_t n;
    for (uint32_t i = 0; i < n; i++)
        if (made[i].f == f) return made[i].rp;
    if (n == COUNT(made)) die("too many render passes");
    VkAttachmentDescription a = {.format = f, .samples = VK_SAMPLE_COUNT_1_BIT, .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                 .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                 .initialLayout = VK_IMAGE_LAYOUT_GENERAL, .finalLayout = VK_IMAGE_LAYOUT_GENERAL};
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    VkSubpassDescription sp = {.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, .colorAttachmentCount = 1, .pColorAttachments = &ref};
    VkRenderPassCreateInfo ci = {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, .attachmentCount = 1, .pAttachments = &a, .subpassCount = 1, .pSubpasses = &sp};
    CK(vkCreateRenderPass(dev, &ci, NULL, &made[n].rp));
    made[n].f = f;
    return made[n++].rp;
}

typedef struct { VkFormat fmt; const uint32_t *vs; size_t nvs; const uint32_t *fs; size_t nfs; VkPrimitiveTopology topo; const VkPipelineColorBlendAttachmentState *blend; } GfxDesc;
static VkPipeline gfx_pipeline(const GfxDesc *d) {
    VkShaderModule vm = mod(d->vs, d->nvs), fm = mod(d->fs, d->nfs);
    VkPipelineShaderStageCreateInfo st[2] = {{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vm, .pName = "main"},
                                             {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = fm, .pName = "main"}};
    VkPipelineVertexInputStateCreateInfo vi = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    VkPipelineInputAssemblyStateCreateInfo ia = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, .topology = d->topo};
    VkPipelineViewportStateCreateInfo vp = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};
    VkPipelineRasterizationStateCreateInfo rs = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .polygonMode = VK_POLYGON_MODE_FILL, .lineWidth = 1};
    VkPipelineMultisampleStateCreateInfo ms = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};
    VkPipelineColorBlendAttachmentState off = {.colorWriteMask = 15};
    VkPipelineColorBlendStateCreateInfo cb = {.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .attachmentCount = 1, .pAttachments = d->blend ? d->blend : &off};
    VkDynamicState dy[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .dynamicStateCount = 2, .pDynamicStates = dy};
    VkGraphicsPipelineCreateInfo pi = {.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, .stageCount = 2, .pStages = st, .pVertexInputState = &vi,
                                       .pInputAssemblyState = &ia, .pViewportState = &vp, .pRasterizationState = &rs, .pMultisampleState = &ms,
                                       .pColorBlendState = &cb, .pDynamicState = &dyn, .layout = pl, .renderPass = render_pass(d->fmt)};
    VkPipeline p;
    CK(vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &pi, NULL, &p));
    vkDestroyShaderModule(dev, vm, NULL);
    vkDestroyShaderModule(dev, fm, NULL);
    return p;
}

static VkDescriptorSet set_make(VkImageView v, VkSampler s, VkBuffer sb) {
    VkDescriptorSetAllocateInfo ai = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .descriptorPool = dp, .descriptorSetCount = 1, .pSetLayouts = &dsl};
    VkDescriptorSet ds;
    CK(vkAllocateDescriptorSets(dev, &ai, &ds));
    VkDescriptorImageInfo ii = {s, v, VK_IMAGE_LAYOUT_GENERAL};
    VkDescriptorBufferInfo bi = {sb, 0, VK_WHOLE_SIZE};
    VkWriteDescriptorSet w[2] = {{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = ds, .dstBinding = 0, .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .pImageInfo = &ii},
                                 {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = ds, .dstBinding = 1, .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .pBufferInfo = &bi}};
    if (v) vkUpdateDescriptorSets(dev, 1, &w[0], 0, NULL);
    if (sb) vkUpdateDescriptorSets(dev, 1, &w[1], 0, NULL);
    return ds;
}

// One draw into target (w x h), cleared to clear; its pixels copied to out
static void draw_one(VkPipeline p, VkDescriptorSet ds, const Img *target, uint32_t w, uint32_t h, const float clear[4], const float pc[8],
                     uint32_t nverts, VkBuffer out) {
    VkFramebufferCreateInfo fi = {.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, .renderPass = render_pass(target->f), .attachmentCount = 1, .pAttachments = &target->v,
                                  .width = w, .height = h, .layers = 1};
    VkFramebuffer fb;
    CK(vkCreateFramebuffer(dev, &fi, NULL, &fb));
    VkCommandBuffer cb = cb_begin();
    VkClearValue cv;
    memcpy(cv.color.float32, clear, 16);
    VkRenderPassBeginInfo ri = {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, .renderPass = fi.renderPass, .framebuffer = fb, .renderArea = {{0, 0}, {w, h}}, .clearValueCount = 1, .pClearValues = &cv};
    vkCmdBeginRenderPass(cb, &ri, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, p);
    if (ds) vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pl, 0, 1, &ds, 0, NULL);
    VkViewport vp = {0, 0, (float)w, (float)h, 0, 1};
    VkRect2D sc = {{0, 0}, {w, h}};
    vkCmdSetViewport(cb, 0, 1, &vp);
    vkCmdSetScissor(cb, 0, 1, &sc);
    vkCmdPushConstants(cb, pl, VK_SHADER_STAGE_ALL, 0, 32, pc);
    vkCmdDraw(cb, nverts, 1, 0, 0);
    vkCmdEndRenderPass(cb);
    full_barrier(cb);
    VkBufferImageCopy c = {.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, .imageExtent = {w, h, 1}};
    vkCmdCopyImageToBuffer(cb, target->i, VK_IMAGE_LAYOUT_GENERAL, out, 1, &c);
    full_barrier(cb);
    cb_end(cb);
    vkDestroyFramebuffer(dev, fb, NULL);
}

static const char *fmt_name(VkFormat f) {
    switch (f) {
#define F(x) case VK_FORMAT_##x: return #x;
    F(R16G16B16A16_UNORM) F(R16_UNORM) F(R32_SFLOAT) F(R32G32B32A32_SFLOAT) F(D16_UNORM) F(D32_SFLOAT) F(D32_SFLOAT_S8_UINT) F(R16G16B16A16_SFLOAT)
    F(R8G8B8A8_UNORM) F(R8_UNORM) F(R16G16_UNORM) F(R16G16_SFLOAT) F(R32G32_SFLOAT) F(R16_SFLOAT) F(B10G11R11_UFLOAT_PACK32) F(A2B10G10R10_UNORM_PACK32)
    F(R16G16B16A16_SNORM) F(R16_SNORM) F(R32G32B32A32_UINT) F(R32_UINT)
#undef F
    default: return "?";
    }
}

static bool has_flags2(void) { return props.apiVersion >= VK_API_VERSION_1_3 || has_ext("VK_KHR_format_feature_flags2"); }

typedef struct { VkFormatFeatureFlags2 optimal, linear, buffer; bool flags2; } FormatCaps;
static FormatCaps format_caps(VkFormat f) {
    FormatCaps r = {.flags2 = has_flags2()};
    VkFormatProperties3 p3 = {.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3};
    VkFormatProperties2 p = {.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2, .pNext = r.flags2 ? &p3 : NULL};
    vkGetPhysicalDeviceFormatProperties2(pd, f, &p);
    r.optimal = r.flags2 ? p3.optimalTilingFeatures : p.formatProperties.optimalTilingFeatures;
    r.linear = r.flags2 ? p3.linearTilingFeatures : p.formatProperties.linearTilingFeatures;
    r.buffer = r.flags2 ? p3.bufferFeatures : p.formatProperties.bufferFeatures;
    return r;
}

// 1 / 0, or -1 for a bit that only the flags2 query can tell
static int reported(VkFormat f, VkFormatFeatureFlags2 bit) {
    FormatCaps c = format_caps(f);
    if (bit == VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT && !c.flags2) return -1;
    return (c.optimal & bit) != 0;
}

static void caps(void) {
    bool driver_props = props.apiVersion >= VK_API_VERSION_1_2 || has_ext("VK_KHR_driver_properties");
    bool ext_e3 = has_ext("VK_EXT_extended_dynamic_state3"), ext_m5 = has_ext("VK_KHR_maintenance5");
    VkPhysicalDeviceDriverProperties drv = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES};
    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT e3 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT, .pNext = driver_props ? &drv : NULL};
    VkPhysicalDeviceProperties2 p2 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = ext_e3 ? (void *)&e3 : driver_props ? (void *)&drv : NULL};
    vkGetPhysicalDeviceProperties2(pd, &p2);
    const VkPhysicalDeviceProperties *pp = &p2.properties;
    emit('P', "\"gpu\":%s,\"vendor_id\":\"%#x\",\"device_id\":\"%#x\",\"driver_name\":%s,\"driver_info\":%s,\"driver_version\":\"%#x\",\"vulkan_api\":\"%u.%u.%u\",\"loaded_via\":\"%s\"",
         json_str(pp->deviceName), pp->vendorID, pp->deviceID, json_str(drv.driverName), json_str(drv.driverInfo), pp->driverVersion,
         VK_API_VERSION_MAJOR(pp->apiVersion), VK_API_VERSION_MINOR(pp->apiVersion), VK_API_VERSION_PATCH(pp->apiVersion), loaded_via);
    VkPhysicalDeviceMaintenance5FeaturesKHR m5 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR};
    VkPhysicalDeviceFeatures2 f2 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = ext_m5 ? &m5 : NULL};
    vkGetPhysicalDeviceFeatures2(pd, &f2);
    emit('C', "\"features\":{\"vertexPipelineStoresAndAtomics\":%u,\"fragmentStoresAndAtomics\":%u,\"independentBlend\":%u,\"largePoints\":%u,\"maintenance5\":%u},"
              "\"extensions\":{\"VK_KHR_maintenance5\":%d,\"VK_EXT_extended_dynamic_state3\":%d,\"VK_KHR_format_feature_flags2\":%d},"
              "\"properties\":{\"dynamicPrimitiveTopologyUnrestricted\":%u,\"pointSizeRange\":[%g,%g],\"pointSizeGranularity\":%g,\"subTexelPrecisionBits\":%u}",
         f2.features.vertexPipelineStoresAndAtomics, f2.features.fragmentStoresAndAtomics, f2.features.independentBlend, f2.features.largePoints, m5.maintenance5,
         ext_m5, ext_e3, has_ext("VK_KHR_format_feature_flags2"),
         e3.dynamicPrimitiveTopologyUnrestricted, pp->limits.pointSizeRange[0], pp->limits.pointSizeRange[1], pp->limits.pointSizeGranularity, pp->limits.subTexelPrecisionBits);
    static const VkFormat fs[] = {VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16_UNORM, VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT,
                                  VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SFLOAT,
                                  VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R16_SFLOAT, VK_FORMAT_B10G11R11_UFLOAT_PACK32, VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R16_SNORM,
                                  VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32_UINT};
    for (size_t i = 0; i < COUNT(fs); i++) {
        FormatCaps c = format_caps(fs[i]);
        VkFormatFeatureFlags2 o = c.optimal;
        emit('F', "{\"format\":\"%s\",\"sampled\":%d,\"filter_linear\":%d,\"depth_comparison\":%s,\"color_attachment\":%d,\"blend\":%d,\"storage_image\":%d,"
                  "\"optimal_flags\":\"%#llx\",\"linear_flags\":\"%#llx\",\"buffer_flags\":\"%#llx\"}",
             fmt_name(fs[i]), !!(o & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT), !!(o & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT),
             !c.flags2 ? "null" : o & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT ? "1" : "0",
             !!(o & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT), !!(o & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT), !!(o & VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT),
             (unsigned long long)c.optimal, (unsigned long long)c.linear, (unsigned long long)c.buffer);
    }
}

#define RT_USAGE (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
#define TEX_USAGE (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)

static void result_no_image(VkFormat f) {
    char name[96];
    snprintf(name, sizeof name, "%s: no such image on this driver", fmt_name(f));
    result_unjudged(name, NAN, -1, "vkGetPhysicalDeviceImageFormatProperties refused the format and usage; cases skipped");
}

// 2x1 texture, texels (.2 .4 .6 .8) and (.8 .6 .4 .2): sampled at u in FS and VS, linear and nearest. .5 at u=.5 is an
// honest bilinear filter; one of the texels there means it fell back to nearest.
static void filter(void) {
    static const VkFormat fs[] = {VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM};
    static const float us[] = {0.25f, 0.3125f, 0.375f, 0.5f, 0.625f, 0.6875f, 0.75f, 0.2505f};
    Img rt = img_make(VK_FORMAT_R32G32B32A32_SFLOAT, 1, 1, RT_USAGE);
    Buf out = buf_make(16);
    GfxDesc df = {VK_FORMAT_R32G32B32A32_SFLOAT, SPV(vs_full), SPV(fs_tex), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, NULL};
    GfxDesc dv = {VK_FORMAT_R32G32B32A32_SFLOAT, SPV(vs_tex), SPV(fs_pass), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, NULL};
    VkPipeline pf = gfx_pipeline(&df), pv = gfx_pipeline(&dv);
    VkSampler sl = sampler(VK_FILTER_LINEAR, false), sn = sampler(VK_FILTER_NEAREST, false);
    static const float clear[4] = {-1, -1, -1, -1};
    for (size_t f = 0; f < COUNT(fs); f++) {
        if (!img_possible(fs[f], TEX_USAGE)) { result_no_image(fs[f]); continue; }
        Img t = img_make(fs[f], 2, 1, TEX_USAGE);
        if (fs[f] == VK_FORMAT_R16G16B16A16_UNORM) {
            uint16_t d[8] = {13107, 26214, 39321, 52428, 52428, 39321, 26214, 13107};
            img_upload(&t, d, sizeof d, 2, 1);
        } else if (fs[f] == VK_FORMAT_R16G16B16A16_SFLOAT) {
            uint16_t d[8] = {0x3266, 0x3666, 0x3899, 0x3a66, 0x3a66, 0x3899, 0x3666, 0x3266};
            img_upload(&t, d, sizeof d, 2, 1);
        } else {
            uint8_t d[8] = {51, 102, 153, 204, 204, 153, 102, 51};
            img_upload(&t, d, sizeof d, 2, 1);
        }
        int has_linear = reported(fs[f], VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
        for (int stage = 0; stage < 2; stage++)
            for (int lin = 1; lin >= 0; lin--) {
                VkDescriptorSet ds = set_make(t.v, lin ? sl : sn, VK_NULL_HANDLE);
                for (size_t u = 0; u < COUNT(us); u++) {
                    float pc[8] = {us[u], 0.5f};
                    draw_one(stage ? pv : pf, ds, &rt, 1, 1, clear, pc, 3, out.b);
                    float got = ((float *)out.p)[0];
                    char name[96];
                    snprintf(name, sizeof name, "%s %s %s u=%.4f", fmt_name(fs[f]), stage ? "VS" : "FS", lin ? "LINEAR" : "NEAREST", us[u]);
                    // (the half float of .8 reads .7998, hence the tolerance)
                    if (lin) result_num(name, 0.2 + 0.6 * (2 * us[u] - 0.5), got, 0.002, has_linear);
                    else if (us[u] == 0.5f) result_text(name, "0.2 or 0.8", got, fabs(got - 0.2) < 0.002 || fabs(got - 0.8) < 0.002, -1, "exactly between the texels, either is right");
                    else result_num(name, us[u] < 0.5f ? 0.2 : 0.8, got, 0.002, -1);
                }
            }
    }
}

// Precision of that filter on a 64x64 texture (the game's heights): steps of 8 of 65535 along x, far below an 8-bit
// step, read at a texel's centre and halfway to the next. R16_UNORM reports the feature, it is the control.
static void filter2(void) {
    static const VkFormat fs[] = {VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R16_UNORM};
    Img rt = img_make(VK_FORMAT_R32G32B32A32_SFLOAT, 1, 1, RT_USAGE);
    Buf out = buf_make(16);
    GfxDesc dv = {VK_FORMAT_R32G32B32A32_SFLOAT, SPV(vs_tex), SPV(fs_pass), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, NULL};
    GfxDesc df = {VK_FORMAT_R32G32B32A32_SFLOAT, SPV(vs_full), SPV(fs_tex), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, NULL};
    VkPipeline pv = gfx_pipeline(&dv), pf = gfx_pipeline(&df);
    VkSampler sl = sampler(VK_FILTER_LINEAR, false);
    static const float clear[4] = {-1, -1, -1, -1};
    for (size_t f = 0; f < COUNT(fs); f++) {
        if (!img_possible(fs[f], TEX_USAGE)) { result_no_image(fs[f]); continue; }
        uint32_t nc = fs[f] == VK_FORMAT_R16_UNORM ? 1 : fs[f] == VK_FORMAT_R16G16_UNORM ? 2 : 4;
        Img t = img_make(fs[f], 64, 64, TEX_USAGE);
        static uint16_t d[64 * 64 * 4];
        bool sn = fs[f] == VK_FORMAT_R16G16B16A16_SNORM;
        for (uint32_t y = 0; y < 64; y++)
            for (uint32_t x = 0; x < 64; x++)
                for (uint32_t c = 0; c < nc; c++) d[(y * 64 + x) * nc + c] = (uint16_t)((sn ? 12345 : 32768) + x * 8);
        img_upload(&t, d, 64 * 64 * nc * 2, 64, 64);
        VkDescriptorSet ds = set_make(t.v, sl, VK_NULL_HANDLE);
        int has_linear = reported(fs[f], VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
        for (int stage = 0; stage < 2; stage++)
            for (int x = 10; x < 13; x++)
                for (int k = 0; k < 2; k++) {
                    float pc[8] = {(x + 0.5f + k * 0.5f) / 64, 0.5f};
                    draw_one(stage ? pv : pf, ds, &rt, 1, 1, clear, pc, 3, out.b);
                    double scale = sn ? 32767.0 : 65535.0, want = ((sn ? 12345.0 : 32768.0) + (x + k * 0.5) * 8) / scale;
                    char name[96];
                    snprintf(name, sizeof name, "%s %s x=%.1f (1 LSB tolerance)", fmt_name(fs[f]), stage ? "VS" : "FS", x + k * 0.5);
                    result_num(name, want, ((float *)out.p)[0], 1 / scale, has_linear);
                }
    }
}

// 2x1, texels .25 and .75, compared with LESS_OR_EQUAL against dref: 1 where dref <= texel, 0 otherwise (.5 between the
// texels with linear). Colour formats are the undefined case, depth ones the control.
static void dref(void) {
    static const VkFormat fs[] = {VK_FORMAT_R16_UNORM, VK_FORMAT_R32_SFLOAT, VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT};
    static const float us[] = {0.25f, 0.5f, 0.75f}, drefs[] = {0.1f, 0.5f, 0.9f};
    Img rt = img_make(VK_FORMAT_R32G32B32A32_SFLOAT, 1, 1, RT_USAGE);
    Buf out = buf_make(16), ssbo = buf_make(16);
    GfxDesc df = {VK_FORMAT_R32G32B32A32_SFLOAT, SPV(vs_full), SPV(fs_cmp), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, NULL};
    GfxDesc dv = {VK_FORMAT_R32G32B32A32_SFLOAT, SPV(vs_cmp), SPV(fs_pass), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, NULL};
    VkPipeline pf = gfx_pipeline(&df), pv = gfx_pipeline(&dv), pc_;
    VkShaderModule cm = mod(SPV(cs_cmp));
    VkComputePipelineCreateInfo cpi = {.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, .stage = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_COMPUTE_BIT, .module = cm, .pName = "main"}, .layout = pl};
    CK(vkCreateComputePipelines(dev, VK_NULL_HANDLE, 1, &cpi, NULL, &pc_));
    static const float clear[4] = {-1, -1, -1, -1};
    for (size_t f = 0; f < COUNT(fs); f++) {
        if (!img_possible(fs[f], TEX_USAGE)) { result_no_image(fs[f]); continue; }
        Img t = img_make(fs[f], 2, 1, TEX_USAGE);
        if (fs[f] == VK_FORMAT_R16_UNORM || fs[f] == VK_FORMAT_D16_UNORM) {
            uint16_t d[2] = {16384, 49151};
            img_upload(&t, d, sizeof d, 2, 1);
        } else {
            float d[2] = {0.25f, 0.75f};
            img_upload(&t, d, sizeof d, 2, 1);
        }
        int has_cmp = reported(fs[f], VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT);
        int has_linear = reported(fs[f], VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
        for (int cmp = 1; cmp >= 0; cmp--)
            for (int lin = 0; lin < 2; lin++) {
                VkSampler s = sampler(lin ? VK_FILTER_LINEAR : VK_FILTER_NEAREST, cmp);
                VkDescriptorSet ds = set_make(t.v, s, ssbo.b);
                int rep = has_cmp < 0 ? -1 : lin ? has_cmp && has_linear : has_cmp;
                for (int stage = 0; stage < 3; stage++)
                    for (size_t d = 0; d < COUNT(drefs); d++)
                        for (size_t u = 0; u < COUNT(us); u++) {
                            float pc[8] = {us[u], 0.5f, drefs[d]};
                            float got;
                            if (stage < 2) {
                                draw_one(stage ? pv : pf, ds, &rt, 1, 1, clear, pc, 3, out.b);
                                got = ((float *)out.p)[0];
                            } else {
                                VkCommandBuffer cb = cb_begin();
                                vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_COMPUTE, pc_);
                                vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_COMPUTE, pl, 0, 1, &ds, 0, NULL);
                                vkCmdPushConstants(cb, pl, VK_SHADER_STAGE_ALL, 0, 32, pc);
                                vkCmdDispatch(cb, 1, 1, 1);
                                full_barrier(cb);
                                cb_end(cb);
                                got = ((float *)ssbo.p)[0];
                            }
                            char name[96];
                            snprintf(name, sizeof name, "%s compareEnable=%d %s %s dref=%.1f u=%.2f", fmt_name(fs[f]), cmp, lin ? "LINEAR" : "NEAREST", stage == 0 ? "FS" : stage == 1 ? "VS" : "CS", drefs[d], us[u]);
                            if (!cmp) { result_unjudged(name, got, -1, "a shadow sampler in the shader with compareEnable=0 is undefined whatever the format: recorded, not judged"); continue; }
                            float low = drefs[d] <= 0.25f, high = drefs[d] <= 0.75f;
                            if (us[u] != 0.5f) result_num(name, us[u] < 0.5f ? low : high, got, 0.01, rep);
                            else if (lin) result_num(name, (low + high) / 2, got, 0.01, rep);
                            else if (low == high) result_num(name, low, got, 0.01, rep);
                            else result_text(name, "0 or 1", got, got == 0 || got == 1, rep, "exactly between the texels, either is right");
                        }
            }
    }
}

static double half_to_double(uint16_t h) {
    int e = h >> 10 & 31, m = h & 1023;
    double v = e == 0 ? ldexp(m, -24) : e == 31 ? (m ? NAN : INFINITY) : ldexp(1024 + m, e - 25);
    return h & 0x8000 ? -v : v;
}

// Additive blend, clear .25 + draw .5: .75 when the draw blends, .5 when it just overwrites
static void blend(void) {
    static const VkFormat fs[] = {VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT};
    for (size_t f = 0; f < COUNT(fs); f++) {
        Img rt = img_make(fs[f], 1, 1, RT_USAGE);
        Buf out = buf_make(16);
        int has_blend = reported(fs[f], VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT);
        for (int b = 0; b < 2; b++) {
            VkPipelineColorBlendAttachmentState add = {.colorWriteMask = 15, .blendEnable = VK_TRUE, .srcColorBlendFactor = VK_BLEND_FACTOR_ONE, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
                                                       .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE};
            GfxDesc d = {fs[f], SPV(vs_full), SPV(fs_const), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, b ? &add : NULL};
            VkPipeline p = gfx_pipeline(&d);
            static const float clear[4] = {0.25f, 0.25f, 0.25f, 0.25f}, pc[8] = {0.5f, 0.5f, 0.5f, 0.5f};
            draw_one(p, VK_NULL_HANDLE, &rt, 1, 1, clear, pc, 3, out.b);
            double got = f == 0 ? ((float *)out.p)[0] : half_to_double(((uint16_t *)out.p)[0]);
            char name[96];
            snprintf(name, sizeof name, "%s blendEnable=%d draw .5 over clear .25", fmt_name(fs[f]), b);
            result_num(name, b ? 0.75 : 0.5, got, 1e-4, b ? has_blend : -1);
        }
    }
}

// Blends beyond additive on the format without the feature: factors, min / max, and whether 32 bits survive it
static void blend2(void) {
    const VkFormat f = VK_FORMAT_R32G32B32A32_SFLOAT;
    Img rt = img_make(f, 1, 1, RT_USAGE);
    Buf out = buf_make(16);
    int has_blend = reported(f, VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT);
    static const struct { const char *name; VkBlendFactor s, d; VkBlendOp op; float clear, draw, alpha, want; bool game; } T[] = {
        {"one+one", VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, 2.0f, 1.5f, 1.0f, 3.5f},
        {"one+one big", VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, 1000000.0f, 1.0f, 1.0f, 1000001.0f},
        {"srcalpha+1-srcalpha", VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, 0.25f, 0.75f, 0.5f, 0.5f},
        {"one+1-srcalpha", VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, 0.25f, 0.75f, 0.5f, 0.875f},
        {"zero+srccolor", VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_SRC_COLOR, VK_BLEND_OP_ADD, 0.5f, 0.5f, 1.0f, 0.25f},
        {"min", VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_MIN, 0.25f, 0.75f, 1.0f, 0.25f},
        {"max", VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_MAX, 0.25f, 0.75f, 1.0f, 0.75f},
        {"game: srca+one / 0+1-srca", VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, 0.25f, 0.75f, 0.5f, 0.625f, true},
        {"rev-subtract", VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_REVERSE_SUBTRACT, 0.75f, 0.25f, 1.0f, 0.5f},
    };
    for (size_t i = 0; i < COUNT(T); i++) {
        VkPipelineColorBlendAttachmentState ba = {.colorWriteMask = 15, .blendEnable = VK_TRUE, .srcColorBlendFactor = T[i].s, .dstColorBlendFactor = T[i].d, .colorBlendOp = T[i].op,
                                                  .srcAlphaBlendFactor = T[i].game ? VK_BLEND_FACTOR_ZERO : T[i].s, .dstAlphaBlendFactor = T[i].game ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA : T[i].d, .alphaBlendOp = T[i].op};
        GfxDesc d = {f, SPV(vs_full), SPV(fs_const), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, &ba};
        VkPipeline p = gfx_pipeline(&d);
        float clear[4] = {T[i].clear, T[i].clear, T[i].clear, T[i].clear}, pc[8] = {T[i].draw, T[i].draw, T[i].draw, T[i].alpha};
        draw_one(p, VK_NULL_HANDLE, &rt, 1, 1, clear, pc, 3, out.b);
        char name[96];
        snprintf(name, sizeof name, "%s clear %g draw %g alpha %g", T[i].name, T[i].clear, T[i].draw, T[i].alpha);
        result_num(name, T[i].want, ((float *)out.p)[0], T[i].want * 1e-6, has_blend);
        if (T[i].game) {
            snprintf(name, sizeof name, "%s, alpha channel", T[i].name);
            double want_a = T[i].clear * (1 - T[i].alpha);
            result_num(name, want_a, ((float *)out.p)[3], want_a * 1e-6, has_blend);
        }
    }
}

// A point at a pixel's centre of a 32x32 target: how many pixels it covers, with and without a PointSize written.
// Without maintenance5, a point list with no PointSize written is the undefined case.
static void point(bool m5) {
    Img rt = img_make(VK_FORMAT_R8G8B8A8_UNORM, 32, 32, RT_USAGE);
    Buf out = buf_make(32 * 32 * 4);
    for (int w = 0; w < 2; w++) {
        GfxDesc d = {VK_FORMAT_R8G8B8A8_UNORM, w ? vs_point1 : vs_point, w ? sizeof vs_point1 : sizeof vs_point, SPV(fs_const), VK_PRIMITIVE_TOPOLOGY_POINT_LIST, NULL};
        VkPipeline p = gfx_pipeline(&d);
        for (int rep = 0; rep < 3; rep++) {
            static const float clear[4] = {0, 0, 0, 0}, pc[8] = {1, 1, 1, 1};
            draw_one(p, VK_NULL_HANDLE, &rt, 32, 32, clear, pc, 1, out.b);
            int n = 0;
            for (int i = 0; i < 32 * 32; i++) n += ((uint8_t *)out.p)[i * 4] != 0;
            char name[96];
            snprintf(name, sizeof name, "maintenance5=%d PointSize %s, repeat %d: pixels covered", m5, w ? "written 1.0" : "not written", rep);
            result_num(name, 1, n, 0, -1);
        }
    }
}

// The game's miss log: a vertex shader's atomicAdd and store into a storage buffer, the feature not enabled when the GPU
// has none. 3 vertices add at least 3; more is allowed, a driver may run a vertex shader more than once.
static void vsatomic(void) {
    Buf ssbo = buf_make(16), out = buf_make(16);
    Img rt = img_make(VK_FORMAT_R8G8B8A8_UNORM, 1, 1, RT_USAGE);
    VkDescriptorSet ds = set_make(VK_NULL_HANDLE, VK_NULL_HANDLE, ssbo.b);
    GfxDesc d = {VK_FORMAT_R8G8B8A8_UNORM, SPV(vs_atomic), SPV(fs_const), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, NULL};
    VkPipeline p = gfx_pipeline(&d);
    VkPhysicalDeviceFeatures have;
    vkGetPhysicalDeviceFeatures(pd, &have);
    uint32_t before = 0;
    for (int rep = 0; rep < 3; rep++) {
        static const float clear[4] = {0, 0, 0, 0}, pc[8] = {1, 1, 1, 1};
        draw_one(p, ds, &rt, 1, 1, clear, pc, 3, out.b);
        uint32_t *w = ssbo.p, count = w[0];
        char name[96];
        snprintf(name, sizeof name, "3 vertices, repeat %d: atomic increments", rep);
        result_text(name, ">= 3", count - before, count - before >= 3, have.vertexPipelineStoresAndAtomics, NULL);
        snprintf(name, sizeof name, "3 vertices, repeat %d: store landed", rep);
        result_num(name, 0xabcd, w[1] >> 16, 0, have.vertexPipelineStoresAndAtomics);
        before = count;
    }
}

static void group_point_m5(void) {
    if (!has_ext("VK_KHR_maintenance5")) skip_group("no VK_KHR_maintenance5");
    if (props.apiVersion < VK_API_VERSION_1_3) skip_group("maintenance5 here needs Vulkan 1.3");
    init_device(true);
    point(true);
}
static void group_point(void) { init_device(false); point(false); }
static void group_filter(void) { init_device(false); filter(); }
static void group_filter2(void) { init_device(false); filter2(); }
static void group_dref(void) { init_device(false); dref(); }
static void group_blend(void) { init_device(false); blend(); }
static void group_blend2(void) { init_device(false); blend2(); }
static void group_vsatomic(void) { init_device(false); vsatomic(); }

static const struct { const char *name; void (*run)(void); } GROUPS[] = {
    {"caps", caps}, {"filter", group_filter}, {"filter2", group_filter2}, {"dref", group_dref}, {"blend", group_blend}, {"blend2", group_blend2},
    {"point", group_point}, {"point-m5", group_point_m5}, {"vsatomic", group_vsatomic},
};

// --- The parent never touches Vulkan: it only collects what each group got out before it ended
typedef struct { char *s; size_t n, cap; } Str;
static void str_add(Str *o, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int need = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (o->n + need + 1 > o->cap) {
        o->cap = (o->n + need + 1) * 2;
        o->s = realloc(o->s, o->cap);
    }
    va_start(ap, fmt);
    vsnprintf(o->s + o->n, need + 1, fmt, ap);
    va_end(ap);
    o->n += need;
}

static const char *prop(const char *name) {
    static char rings[2][PROP_VALUE_MAX];
    static int ring;
    char *o = rings[ring++ % 2];
    o[0] = 0;
    __system_property_get(name, o);
    return o;
}

int main(int argc, char **argv) {
    const char *driver = NULL, *out_path = NULL;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--driver") && i + 1 < argc) driver = argv[++i];
        else if (!strcmp(argv[i], "--out") && i + 1 < argc) out_path = argv[++i];
        else { fprintf(stderr, "usage: %s [--driver <path to a Vulkan .so>] [--out <file.json>]\n", argv[0]); return 1; }
    }
    Str passport = {0}, cap_fields = {0}, formats = {0}, results = {0}, groups = {0};
    for (size_t g = 0; g < COUNT(GROUPS); g++) {
        int fds[2];
        if (pipe(fds)) { perror("pipe"); return 1; }
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return 1; }
        if (!pid) {
            close(fds[0]);
            report_fd = fds[1];
            group_name = GROUPS[g].name;
            dup2(2, 1);  // whatever a driver prints must not get into the JSON
            alarm(120);
            load_driver(driver);
            init_instance();
            GROUPS[g].run();
            _exit(0);
        }
        close(fds[1]);
        Str got = {0};
        char chunk[4096];
        ssize_t r;
        while ((r = read(fds[0], chunk, sizeof chunk)) > 0) str_add(&got, "%.*s", (int)r, chunk);
        close(fds[0]);
        int st = 0;
        waitpid(pid, &st, 0);
        int cases = 0, differ = 0;
        const char *skipped = NULL, *error = NULL;
        for (char *line = got.s, *next; line && *line; line = next) {
            next = strchr(line, '\n');
            if (!next) break;  // cut off by the group's end
            *next++ = 0;
            const char *body = line + 2;
            switch (line[0]) {
            case 'P': str_add(&passport, "%s", body); break;
            case 'C': str_add(&cap_fields, "%s", body); break;
            case 'F': str_add(&formats, "%s\n      %s", formats.n ? "," : "", body); break;
            case 'R':
                str_add(&results, "%s\n    %s", results.n ? "," : "", body);
                cases++;
                differ += strstr(body, "\"ok\":false") != NULL;
                break;
            case 'S': skipped = body; break;
            case 'E': error = body; break;
            }
        }
        const char *status = "ok";
        char how[64] = "";
        if (WIFSIGNALED(st)) {
            status = WTERMSIG(st) == 14 ? "timeout" : "crashed";
            snprintf(how, sizeof how, ",\"signal\":%d", WTERMSIG(st));
        } else if (WEXITSTATUS(st)) status = "failed";
        else if (skipped) status = "skipped";
        str_add(&groups, "%s\n    {\"name\":\"%s\",\"status\":\"%s\",\"cases\":%d,\"differ\":%d%s", g ? "," : "", GROUPS[g].name, status, cases, differ, how);
        if (error) str_add(&groups, ",\"error\":%s", error);
        if (skipped) str_add(&groups, ",\"reason\":%s", skipped);
        str_add(&groups, "}");
        fprintf(stderr, "%-9s %-8s %3d cases, %d differ%s%s\n", GROUPS[g].name, status, cases, differ, error ? " " : "", error ? error : "");
        free(got.s);
    }
    FILE *o = out_path ? fopen(out_path, "w") : stdout;
    if (!o) { perror(out_path); return 1; }
    const char *slash = driver ? strrchr(driver, '/') : NULL;
    fprintf(o, "{\n  \"probe\": \"test_vk_formats\",\n  \"driver_loaded_from\": %s,\n", driver ? json_str(slash ? slash + 1 : driver) : "\"system\"");
    fprintf(o, "  \"passport\": {%s%s\"android_release\":%s,\"android_sdk\":%s},\n", passport.n ? passport.s : "", passport.n ? "," : "",
            json_str(prop("ro.build.version.release")), json_str(prop("ro.build.version.sdk")));
    fprintf(o, "  \"caps\": {\n    %s%s\"formats\": [%s\n    ]\n  },\n", cap_fields.n ? cap_fields.s : "", cap_fields.n ? ",\n    " : "", formats.n ? formats.s : "");
    fprintf(o, "  \"groups\": [%s\n  ],\n", groups.s);
    fprintf(o, "  \"results\": [%s\n  ]\n}\n", results.n ? results.s : "");
    if (fflush(o) || (out_path && fclose(o))) { perror("write"); return 1; }
    return 0;
}
