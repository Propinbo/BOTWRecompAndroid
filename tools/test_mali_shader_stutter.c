// Native Android unit tests for Mali shader/pipeline stutter reduction:
//   1. Urgent queue + work-stealing + multi-threaded worker/stealer race checks
//   2. Binary Maxwell stage metadata cache (.smeta) key & persistence validation
//   3. Dynamic vertex input (VK_EXT_vertex_input_dynamic_state) descriptor builder
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan_core.h>

#include "nvn/vulkan/shader_meta_cache.h"

static void dummy_fn(void *arg) { (void)arg; }

typedef struct {
    _Atomic int count;
    int idx;
} RaceItem;

typedef struct {
    pthread_mutex_t *lock;
    pthread_cond_t *cond;
    MwJob **head;
    MwJob **tail;
    _Atomic bool *stop;
    _Atomic int *completed;
} RaceCtx;

static void race_job(void *arg) {
    RaceItem *it = (RaceItem *)arg;
    int prev = atomic_fetch_add_explicit(&it->count, 1, memory_order_relaxed);
    assert(prev == 0);
}

static void *race_worker(void *arg) {
    RaceCtx *ctx = (RaceCtx *)arg;
    for (;;) {
        pthread_mutex_lock(ctx->lock);
        while (!*ctx->head && !atomic_load_explicit(ctx->stop, memory_order_acquire))
            pthread_cond_wait(ctx->cond, ctx->lock);
        if (!*ctx->head && atomic_load_explicit(ctx->stop, memory_order_acquire)) {
            pthread_mutex_unlock(ctx->lock);
            return NULL;
        }
        MwJob *j = *ctx->head;
        *ctx->head = j->next;
        if (!*ctx->head) *ctx->tail = NULL;
        pthread_mutex_unlock(ctx->lock);
        j->fn(j->arg);
        free(j);
        atomic_fetch_add_explicit(ctx->completed, 1, memory_order_release);
    }
}

static void test_job_queue_and_steal(void) {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    MwJob *head = NULL, *tail = NULL;

    int a = 1, b = 2, c = 3, d = 4, e = 5, missing = 99;

    // Empty steal
    assert(mw_job_steal(&lock, &head, &tail, &a) == NULL);
    assert(head == NULL && tail == NULL);

    // 1-item steal
    mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &a, false);
    assert(mw_job_steal(&lock, &head, &tail, &missing) == NULL);
    MwJob *s = mw_job_steal(&lock, &head, &tail, &a);
    assert(s != NULL && s->arg == &a);
    free(s);
    assert(head == NULL && tail == NULL);

    // Head / middle / tail steals + urgent prepend
    mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &a, false);
    mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &b, false);
    mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &c, false);
    mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &d, false);
    // Queue: [a, b, c, d]
    s = mw_job_steal(&lock, &head, &tail, &a);
    assert(s && s->arg == &a);
    free(s);
    // Queue: [b, c, d]
    s = mw_job_steal(&lock, &head, &tail, &c);
    assert(s && s->arg == &c);
    free(s);
    // Queue: [b, d]
    s = mw_job_steal(&lock, &head, &tail, &d);
    assert(s && s->arg == &d);
    free(s);
    // Queue: [b]
    mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &e, true);
    // Queue: [e, b]
    s = mw_job_pop(&lock, &cond, &head, &tail);
    assert(s && s->arg == &e);
    free(s);
    s = mw_job_pop(&lock, &cond, &head, &tail);
    assert(s && s->arg == &b);
    free(s);
    assert(head == NULL && tail == NULL);

    // Concurrent multi-threaded worker + stealer race test
    enum { N_ITEMS = 600, N_WORKERS = 3 };
    RaceItem *items = (RaceItem *)calloc(N_ITEMS, sizeof *items);
    assert(items != NULL);
    for (int i = 0; i < N_ITEMS; i++) items[i].idx = i;

    _Atomic bool stop = false;
    _Atomic int completed = 0;
    RaceCtx ctx = {.lock = &lock, .cond = &cond, .head = &head, .tail = &tail, .stop = &stop, .completed = &completed};
    pthread_t th[N_WORKERS];
    for (int i = 0; i < N_WORKERS; i++) assert(pthread_create(&th[i], NULL, race_worker, &ctx) == 0);

    for (int i = 0; i < N_ITEMS; i++) {
        mw_job_push(&lock, &cond, &head, &tail, race_job, &items[i], (i % 7) == 0);
        if ((i % 3) == 0) {
            int target = i > 4 ? i - 3 : i;
            MwJob *st = mw_job_steal(&lock, &head, &tail, &items[target]);
            if (st) {
                st->fn(st->arg);
                free(st);
                atomic_fetch_add_explicit(&completed, 1, memory_order_release);
            }
        }
    }
    for (int i = 0; i < N_ITEMS; i++) {
        MwJob *st = mw_job_steal(&lock, &head, &tail, &items[i]);
        if (st) {
            st->fn(st->arg);
            free(st);
            atomic_fetch_add_explicit(&completed, 1, memory_order_release);
        }
    }
    while (atomic_load_explicit(&completed, memory_order_acquire) < N_ITEMS) usleep(100);
    pthread_mutex_lock(&lock);
    atomic_store_explicit(&stop, true, memory_order_release);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&lock);
    for (int i = 0; i < N_WORKERS; i++) pthread_join(th[i], NULL);

    for (int i = 0; i < N_ITEMS; i++) assert(atomic_load_explicit(&items[i].count, memory_order_relaxed) == 1);
    free(items);
    assert(head == NULL && tail == NULL);
}

static void build_fake_maxwell_shader(uint8_t *buf, size_t buf_sz, size_t *out_cmax, size_t *out_const_sz,
                                      bool include_brx) {
    memset(buf, 0, buf_sz);
    // 0x50 byte SPH at +0x30
    for (size_t i = 0; i < 0x50; i++) buf[0x30 + i] = (uint8_t)(i + 1);
    uint64_t *code = (uint64_t *)(buf + 0x80);
    // Instruction group: idx 0 = sched, 1..3 = instructions, 4 = sched, 5 = bra self, 6.. = dead padding
    code[0] = 0x001fc00000000000ull;
    code[1] = 0x5c98078000070001ull;
    code[2] = include_brx ? 0xe250000000000000ull : 0x5c98078000070002ull;
    code[3] = 0xe30000000007000full; // EXIT
    code[4] = 0x001fc00000000000ull;
    // bra -8 (offset 0xFFFFFF8 -> bits 20..43 = 0xFFFFF8)
    code[5] = 0xe240000000000000ull | (0xFFFFF8ull << 20);
    code[6] = 0xdeadbeefcafebabeull; // past bra self
    *out_cmax = 0x80 + 16 * 8;
    uint32_t *cb1 = (uint32_t *)(buf + *out_cmax);
    cb1[0] = 0x10;
    cb1[1] = 0x20;
    *out_const_sz = 16;
}

static void test_stage_key_and_smeta(void) {
    uint8_t blob_a[1024], blob_b[1024];
    size_t cmax = 0, const_sz = 0;
    build_fake_maxwell_shader(blob_a, sizeof blob_a, &cmax, &const_sz, false);
    memcpy(blob_b, blob_a, sizeof blob_a);
    // Modify dead padding past bra self: keys MUST remain identical
    ((uint64_t *)(blob_b + 0x80))[6] = 0x123456789abcdef0ull;

    ShaderKey vkey = {.glsl = 1, .flip_y = true, .depth_minus_one = true};
    vkey.attr_kind[0] = 0;
    vkey.attr_kind[1] = 2;

    uint64_t h1a = 0, h2a = 0, h1b = 0, h2b = 0;
    uint32_t tf = MW_TF_BRX_WINDOW | MW_TF_STATIC_STACKS | MW_TF_STRUCTURED;
    assert(mw_stage_key(blob_a, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0x100000000ull, 0x80000000ull, &h1a, &h2a));
    assert(mw_stage_key(blob_b, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    assert(h1a == h1b && h2a == h2b);

    // Modify active instruction: key must change
    ((uint64_t *)(blob_b + 0x80))[1] ^= 1ull;
    assert(mw_stage_key(blob_b, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    assert(h1a != h1b || h2a != h2b);

    // Modify ShaderKey and trans_flags: key must change
    ShaderKey vkey2 = vkey;
    vkey2.flip_x = true;
    assert(mw_stage_key(blob_a, cmax, const_sz, STAGE_VERTEX, &vkey2, tf, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    assert(h1a != h1b && h2a != h2b);
    assert(mw_stage_key(blob_a, cmax, const_sz, STAGE_VERTEX, &vkey, tf | MW_TF_VK_OPT, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    assert(h1a != h1b && h2a != h2b);

    // BRX table sensitivity
    build_fake_maxwell_shader(blob_a, sizeof blob_a, &cmax, &const_sz, true);
    memcpy(blob_b, blob_a, sizeof blob_a);
    ((uint32_t *)(blob_b + cmax))[1] = 0x38;
    assert(mw_stage_key(blob_a, cmax, const_sz, STAGE_FRAGMENT, &vkey, tf, 0x100000000ull, 0x80000000ull, &h1a, &h2a));
    assert(mw_stage_key(blob_b, cmax, const_sz, STAGE_FRAGMENT, &vkey, tf, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    assert(h1a != h1b && h2a != h2b);

    // Invalid inputs
    assert(!mw_stage_key(NULL, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0, 0, &h1a, &h2a));
    assert(!mw_stage_key(blob_a, 0x80, 0, STAGE_VERTEX, &vkey, tf, 0, 0, &h1a, &h2a));
    assert(!mw_stage_key(blob_a, cmax, const_sz, STAGE_COMPUTE, &vkey, tf, 0, 0, &h1a, &h2a));

    // Binary .smeta round-trip and corruption checks
    char dir[] = "/data/local/tmp/botw_smeta_test_XXXXXX";
    assert(mkdtemp(dir) != NULL);

    ShaderInfo src = {0};
    src.stage = STAGE_FRAGMENT;
    src.cbuf_mask = 0x15u;
    src.cbuf_size[0] = 64;
    src.cbuf_size[2] = 512;
    src.cbuf_dynamic = 0x4u;
    src.cbwin_mask = 0x2u;
    src.ntex = 2;
    src.tex[0] = (ShaderTexture){.handle_offset = 0x28, .type = TEXTYPE_2D_ARRAY, .shadow = true, .is_array = true};
    src.tex[1] = (ShaderTexture){.handle_offset = 0x40, .type = TEXTYPE_3D, .is_image = true, .bindless = true,
                                 .bindless_cbuf = 3, .bindless_offset = 0x80};
    src.attr_mask = 0x1fu;
    src.out_mask = 0x0f0fu;
    src.writes_depth = true;
    src.uses_discard = true;
    src.uses_gmem = true;
    src.uses_gmem_map = true;
    src.uses_tint = true;
    src.uses_rscale = true;
    src.local_mem = 128;
    src.num_insts = 42;

    assert(mw_meta_store(dir, h1a, h2a, STAGE_FRAGMENT, &src, 0xfeedface12345678ull, false, 3));

    uint64_t got_spv = 0;
    bool got_vid = true;
    uint8_t got_gather = 0;
    ShaderInfo *loaded = mw_meta_load(dir, h1a, h2a, STAGE_FRAGMENT, &got_spv, &got_vid, &got_gather);
    assert(loaded != NULL);
    assert(loaded->msl == NULL);
    assert(got_spv == 0xfeedface12345678ull);
    assert(!got_vid);
    assert(got_gather == 3);
    assert(loaded->stage == STAGE_FRAGMENT);
    assert(loaded->cbuf_mask == src.cbuf_mask);
    assert(loaded->cbuf_size[0] == 64 && loaded->cbuf_size[2] == 512);
    assert(loaded->cbuf_dynamic == src.cbuf_dynamic && loaded->cbwin_mask == src.cbwin_mask);
    assert(loaded->ntex == 2);
    assert(loaded->tex[0].shadow && loaded->tex[0].is_array && !loaded->tex[0].is_image);
    assert(loaded->tex[1].is_image && loaded->tex[1].bindless && loaded->tex[1].bindless_cbuf == 3);
    assert(loaded->attr_mask == src.attr_mask && loaded->out_mask == src.out_mask);
    assert(loaded->writes_depth && loaded->uses_discard && !loaded->writes_point_size);
    assert(loaded->uses_gmem && loaded->uses_gmem_map && !loaded->uses_gmem_store);
    assert(loaded->uses_tint && loaded->uses_rscale && loaded->local_mem == 128 && loaded->num_insts == 42);
    free(loaded);

    // Mismatched h2 or stage must fail
    assert(mw_meta_load(dir, h1a, h2a ^ 1ull, STAGE_FRAGMENT, &got_spv, &got_vid, &got_gather) == NULL);
    assert(mw_meta_load(dir, h1a, h2a, STAGE_VERTEX, &got_spv, &got_vid, &got_gather) == NULL);

    // Corrupt payload byte must fail
    char path[320];
    snprintf(path, sizeof path, "%s/%016llx.smeta", dir, (unsigned long long)h1a);
    FILE *f = fopen(path, "r+b");
    assert(f != NULL);
    assert(fseek(f, 64, SEEK_SET) == 0);
    uint8_t byte = 0;
    assert(fread(&byte, 1, 1, f) == 1);
    byte ^= 0x5au;
    assert(fseek(f, 64, SEEK_SET) == 0);
    assert(fwrite(&byte, 1, 1, f) == 1);
    fclose(f);
    assert(mw_meta_load(dir, h1a, h2a, STAGE_FRAGMENT, &got_spv, &got_vid, &got_gather) == NULL);

    // Truncated file must fail
    assert(truncate(path, 32) == 0);
    assert(mw_meta_load(dir, h1a, h2a, STAGE_FRAGMENT, &got_spv, &got_vid, &got_gather) == NULL);
    remove(path);
    rmdir(dir);
}

static VkFormat test_fmt_fn(uint32_t f) {
    return f == 0x25 ? VK_FORMAT_R8G8B8A8_UNORM : f == 0x2E ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R32G32B32_SFLOAT;
}

static void test_dyn_vi(void) {
    // Empty mask
    VkDynVi vi;
    uint32_t attribs[4] = {
        0x2Eu | (0u << 8) | (0u << 13),   // loc 0: stream 0, off 0
        0x25u | (1u << 8) | (12u << 13),  // loc 1: stream 1, off 12
        0x00u | (0u << 8) | (0u << 13),   // loc 2: fmt 0 -> ZERO_BINDING
        0x2Eu | (16u << 8) | (4u << 13),  // loc 3: stream 16 -> ZERO_BINDING
    };
    uint32_t streams[2][2] = {
        {32, 0},  // stream 0: stride 32, vertex rate (divisor 0 -> Vk divisor MUST be 1)
        {16, 4},  // stream 1: stride 16, instance rate divisor 4
    };

    vk_build_dyn_vi(0, 4, attribs, 2, streams, true, test_fmt_fn, &vi, 7);
    assert(vi.gen == 7 && vi.na == 0 && vi.nb == 0 && vi.used_streams == 0);

    // With divisor extension enabled
    vk_build_dyn_vi(0x1Fu, 4, attribs, 2, streams, true, test_fmt_fn, &vi, 8);
    assert(vi.gen == 8);
    assert(vi.na == 5);
    assert(vi.nb == 3);  // stream 0, stream 1, ZERO_BINDING (16)
    assert(vi.used_streams == ((1u << 0) | (1u << 1) | (1u << ZERO_BINDING)));
    for (uint32_t i = 0; i < vi.na; i++) {
        assert(vi.va[i].sType == VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT);
        assert(vi.va[i].pNext == NULL);
    }
    assert(vi.va[0].location == 0 && vi.va[0].binding == 0 && vi.va[0].format == VK_FORMAT_R32G32B32A32_SFLOAT && vi.va[0].offset == 0);
    assert(vi.va[1].location == 1 && vi.va[1].binding == 1 && vi.va[1].format == VK_FORMAT_R8G8B8A8_UNORM && vi.va[1].offset == 12);
    assert(vi.va[2].location == 2 && vi.va[2].binding == ZERO_BINDING && vi.va[2].format == VK_FORMAT_R32G32B32A32_SFLOAT && vi.va[2].offset == 0);
    assert(vi.va[3].location == 3 && vi.va[3].binding == ZERO_BINDING);
    assert(vi.va[4].location == 4 && vi.va[4].binding == ZERO_BINDING); // i >= nattrib

    for (uint32_t i = 0; i < vi.nb; i++) {
        assert(vi.vb[i].sType == VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT);
        assert(vi.vb[i].pNext == NULL);
        assert(vi.vb[i].divisor >= 1u); // Vulkan VUID forbids divisor == 0 when inputRate == VERTEX
    }
    assert(vi.vb[0].binding == 0 && vi.vb[0].stride == 32 && vi.vb[0].inputRate == VK_VERTEX_INPUT_RATE_VERTEX && vi.vb[0].divisor == 1);
    assert(vi.vb[1].binding == 1 && vi.vb[1].stride == 16 && vi.vb[1].inputRate == VK_VERTEX_INPUT_RATE_INSTANCE && vi.vb[1].divisor == 4);
    assert(vi.vb[2].binding == ZERO_BINDING && vi.vb[2].stride == 0 && vi.vb[2].inputRate == VK_VERTEX_INPUT_RATE_VERTEX && vi.vb[2].divisor == 1);

    // Without divisor extension: instance divisor > 1 clamps to 1
    vk_build_dyn_vi(0x1Fu, 4, attribs, 2, streams, false, test_fmt_fn, &vi, 9);
    assert(vi.vb[1].inputRate == VK_VERTEX_INPUT_RATE_INSTANCE && vi.vb[1].divisor == 1);
    assert(vi.divisor[1] == 1);
}

int main(void) {
    test_job_queue_and_steal();
    test_stage_key_and_smeta();
    test_dyn_vi();
    puts("PASS: test_mali_shader_stutter (queue/steal, smeta key/store/load, dyn_vi)");
    return 0;
}
