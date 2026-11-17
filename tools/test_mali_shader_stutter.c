// Host/Bionic unit tests for Mali shader/pipeline stutter reduction:
//   1. Urgent queue, stealing, active-count drain, and multi-threaded worker/stealer race checks
//   2. Binary Maxwell stage metadata cache (.smeta) key, flags, negative controls, & persistence
//   3. Dynamic vertex input (VK_EXT_vertex_input_dynamic_state) descriptor builder
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#define mw_getpid() _getpid()
#define mw_mkdir(p) _mkdir(p)
#define mw_rmdir(p) _rmdir(p)
#else
#include <unistd.h>
#define mw_getpid() getpid()
#define mw_mkdir(p) mkdir((p), 0700)
#define mw_rmdir(p) rmdir(p)
#endif

#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan_core.h>

#include "nvn/vulkan/shader_meta_cache.h"

static void dummy_fn(void *arg) { (void)arg; }

typedef struct {
    _Atomic int count;
    int idx;
    bool forward_to_tjobs;
} RaceItem;

typedef struct {
    pthread_mutex_t *lock;
    pthread_cond_t *cond;
    MwJob **head;
    MwJob **tail;
    int *active;
    bool *stop;
    pthread_mutex_t *tlock;
    pthread_cond_t *tcond;
    MwJob **thead;
    MwJob **ttail;
    int *tactive;
    pthread_mutex_t *done_lock;
    pthread_cond_t *done_cond;
    int *completed;
    int target;
} RaceCtx;

static void forward_fn(void *arg) {
    RaceItem *it = (RaceItem *)arg;
    int prev = atomic_fetch_add_explicit(&it->count, 1, memory_order_relaxed);
    assert(prev == 0);
}

static void race_job_body(RaceCtx *ctx, RaceItem *it) {
    if (it->forward_to_tjobs) {
        it->forward_to_tjobs = false;
        assert(mw_job_push(ctx->tlock, ctx->tcond, ctx->thead, ctx->ttail, forward_fn, it, (it->idx % 5) == 0));
    } else {
        int prev = atomic_fetch_add_explicit(&it->count, 1, memory_order_relaxed);
        assert(prev == 0);
        pthread_mutex_lock(ctx->done_lock);
        (*ctx->completed)++;
        if (*ctx->completed == ctx->target) pthread_cond_broadcast(ctx->done_cond);
        pthread_mutex_unlock(ctx->done_lock);
    }
}

typedef struct {
    RaceCtx *ctx;
    RaceItem *it;
} WrappedItem;

static void wrapped_fn(void *arg) {
    WrappedItem *w = (WrappedItem *)arg;
    race_job_body(w->ctx, w->it);
}

static void *tjob_worker(void *arg) {
    RaceCtx *ctx = (RaceCtx *)arg;
    for (;;) {
        pthread_mutex_lock(ctx->tlock);
        while (!*ctx->thead && !*ctx->stop) pthread_cond_wait(ctx->tcond, ctx->tlock);
        if (!*ctx->thead && *ctx->stop) {
            pthread_mutex_unlock(ctx->tlock);
            return NULL;
        }
        MwJob *j = *ctx->thead;
        *ctx->thead = j->next;
        if (!*ctx->thead) *ctx->ttail = NULL;
        (*ctx->tactive)++;
        pthread_mutex_unlock(ctx->tlock);
        j->fn(j->arg);
        free(j);
        (void)mw_job_finish(ctx->tlock, ctx->thead, ctx->tactive);
        pthread_mutex_lock(ctx->done_lock);
        (*ctx->completed)++;
        if (*ctx->completed == ctx->target) pthread_cond_broadcast(ctx->done_cond);
        pthread_mutex_unlock(ctx->done_lock);
    }
}

static void *race_worker(void *arg) {
    RaceCtx *ctx = (RaceCtx *)arg;
    for (;;) {
        pthread_mutex_lock(ctx->lock);
        while (!*ctx->head && !*ctx->stop) pthread_cond_wait(ctx->cond, ctx->lock);
        if (!*ctx->head && *ctx->stop) {
            pthread_mutex_unlock(ctx->lock);
            return NULL;
        }
        MwJob *j = *ctx->head;
        *ctx->head = j->next;
        if (!*ctx->head) *ctx->tail = NULL;
        (*ctx->active)++;
        pthread_mutex_unlock(ctx->lock);
        j->fn(j->arg);
        free(j);
        (void)mw_job_finish(ctx->lock, ctx->head, ctx->active);
    }
}

static void test_job_queue_and_steal(void) {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    MwJob *head = NULL, *tail = NULL;
    int active = 0;

    int a = 1, b = 2, c = 3, d = 4, e1 = 5, e2 = 6, f = 7, missing = 99;

    assert(mw_job_steal(&lock, &head, &tail, &a) == NULL);
    assert(head == NULL && tail == NULL);

    // 1-item push and steal
    assert(mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &a, false));
    assert(mw_job_steal(&lock, &head, &tail, &missing) == NULL);
    MwJob *s = mw_job_steal(&lock, &head, &tail, &a);
    assert(s != NULL && s->arg == &a);
    free(s);
    assert(head == NULL && tail == NULL);

    // Steal head, middle, tail; then verify non-urgent append, multiple urgent prepends, and pop_active/finish
    assert(mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &a, false));
    assert(mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &b, false));
    assert(mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &c, false));
    assert(mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &d, false));
    // [a, b, c, d] -> steal a, c, d -> [b] with tail == b
    s = mw_job_steal(&lock, &head, &tail, &a);
    assert(s && s->arg == &a);
    free(s);
    s = mw_job_steal(&lock, &head, &tail, &c);
    assert(s && s->arg == &c);
    free(s);
    s = mw_job_steal(&lock, &head, &tail, &d);
    assert(s && s->arg == &d);
    free(s);
    assert(mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &f, false));
    assert(mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &e1, true));
    assert(mw_job_push(&lock, &cond, &head, &tail, dummy_fn, &e2, true));
    // Expected order: [e2, e1, b, f]
    int *expected[4] = {&e2, &e1, &b, &f};
    for (int k = 0; k < 4; k++) {
        s = mw_job_pop_active(&lock, &cond, &head, &tail, &active);
        assert(s && s->arg == expected[k]);
        assert(active == 1);
        free(s);
        bool idle = mw_job_finish(&lock, &head, &active);
        assert(idle == (k == 3));
    }
    assert(head == NULL && tail == NULL && active == 0);

    // Multi-threaded worker + stealer + two-queue forwarding race test (no usleep)
    enum { N_ITEMS = 600, N_WORKERS = 3 };
    RaceItem *items = (RaceItem *)calloc(N_ITEMS, sizeof *items);
    WrappedItem *wrapped = (WrappedItem *)calloc(N_ITEMS, sizeof *wrapped);
    assert(items && wrapped);

    pthread_mutex_t tlock = PTHREAD_MUTEX_INITIALIZER, dlock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t tcond = PTHREAD_COND_INITIALIZER, dcond = PTHREAD_COND_INITIALIZER;
    MwJob *thead = NULL, *ttail = NULL;
    int tactive = 0, completed = 0;
    bool stop = false;
    RaceCtx ctx = {
        .lock = &lock, .cond = &cond, .head = &head, .tail = &tail, .active = &active, .stop = &stop,
        .tlock = &tlock, .tcond = &tcond, .thead = &thead, .ttail = &ttail, .tactive = &tactive,
        .done_lock = &dlock, .done_cond = &dcond, .completed = &completed, .target = N_ITEMS,
    };
    for (int i = 0; i < N_ITEMS; i++) {
        items[i].idx = i;
        items[i].forward_to_tjobs = (i % 4) == 0;
        wrapped[i] = (WrappedItem){.ctx = &ctx, .it = &items[i]};
    }

    pthread_t th[N_WORKERS], tth;
    for (int i = 0; i < N_WORKERS; i++) assert(pthread_create(&th[i], NULL, race_worker, &ctx) == 0);
    assert(pthread_create(&tth, NULL, tjob_worker, &ctx) == 0);

    for (int i = 0; i < N_ITEMS; i++) {
        assert(mw_job_push(&lock, &cond, &head, &tail, wrapped_fn, &wrapped[i], (i % 7) == 0));
        if ((i % 3) == 0) {
            int target = i > 4 ? i - 3 : i;
            MwJob *st = mw_job_steal(&lock, &head, &tail, &wrapped[target]);
            if (st) {
                st->fn(st->arg);
                free(st);
            }
        }
    }
    for (int i = 0; i < N_ITEMS; i++) {
        MwJob *st = mw_job_steal(&lock, &head, &tail, &wrapped[i]);
        if (st) {
            st->fn(st->arg);
            free(st);
        }
    }

    pthread_mutex_lock(&dlock);
    while (completed < N_ITEMS) pthread_cond_wait(&dcond, &dlock);
    pthread_mutex_unlock(&dlock);

    pthread_mutex_lock(&lock);
    pthread_mutex_lock(&tlock);
    stop = true;
    pthread_cond_broadcast(&cond);
    pthread_cond_broadcast(&tcond);
    pthread_mutex_unlock(&tlock);
    pthread_mutex_unlock(&lock);

    for (int i = 0; i < N_WORKERS; i++) pthread_join(th[i], NULL);
    pthread_join(tth, NULL);

    for (int i = 0; i < N_ITEMS; i++) assert(atomic_load_explicit(&items[i].count, memory_order_relaxed) == 1);
    free(wrapped);
    free(items);
    assert(head == NULL && tail == NULL && active == 0 && thead == NULL && ttail == NULL && tactive == 0);
}

static bool dir_exists(const char *path) {
    struct stat st;
    return path && *path && stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static void make_test_dir(char *out, size_t out_sz) {
    const char *base = getenv("TMPDIR");
    if (!dir_exists(base)) base = getenv("TMP");
    if (!dir_exists(base)) base = getenv("TEMP");
    if (!dir_exists(base)) base = dir_exists("/data/local/tmp") ? "/data/local/tmp" : dir_exists("/tmp") ? "/tmp" : ".";
    for (unsigned attempt = 0; attempt < 100; attempt++) {
        int n = snprintf(out, out_sz, "%s/botw_smeta_test_%llu_%u", base, (unsigned long long)mw_getpid(), attempt);
        assert(n > 0 && (size_t)n < out_sz);
        if (mw_mkdir(out) == 0) return;
    }
    assert(!"mw_mkdir failed");
}

static void put_u64(uint8_t *dst, uint64_t v) { memcpy(dst, &v, sizeof v); }
static void put_u32(uint8_t *dst, uint32_t v) { memcpy(dst, &v, sizeof v); }

static void build_fake_maxwell_shader(uint8_t *buf, size_t buf_sz, size_t *out_cmax, size_t *out_const_sz,
                                      bool include_brx) {
    memset(buf, 0, buf_sz);
    for (size_t i = 0; i < 0x50; i++) buf[0x30 + i] = (uint8_t)(i + 1);
    uint8_t *code = buf + 0x80;
    put_u64(code + 0 * 8, 0x001fc00000000000ull);
    put_u64(code + 1 * 8, 0x5c98078000070001ull);
    put_u64(code + 2 * 8, include_brx ? 0xe250000000000000ull : 0x5c98078000070002ull);
    put_u64(code + 3 * 8, 0xe30000000007000full);
    put_u64(code + 4 * 8, 0x001fc00000000000ull);
    put_u64(code + 5 * 8, 0xe240000000000000ull | (0xFFFFF8ull << 20));  // bra -8
    put_u64(code + 6 * 8, 0xdeadbeefcafebabeull);                        // past bra self
    *out_cmax = 0x80 + 16 * 8;
    uint8_t *cb1 = buf + *out_cmax;
    put_u32(cb1 + 0, 0x10);
    put_u32(cb1 + 4, 0x20);
    *out_const_sz = 16;
}

static void assert_key_differs_vs(const uint8_t *blob, size_t cmax, size_t const_sz, int stage,
                                  const ShaderKey *k, uint32_t tf, uint64_t gbase, uint64_t gsize,
                                  uint64_t ref1, uint64_t ref2) {
    uint64_t h1 = 0, h2 = 0;
    assert(mw_stage_key(blob, cmax, const_sz, stage, k, tf, gbase, gsize, &h1, &h2));
    assert(h1 != ref1 || h2 != ref2);
}

static void test_stage_key_and_smeta(void) {
    uint8_t blob_a[1024], blob_b[1024];
    size_t cmax = 0, const_sz = 0;
    build_fake_maxwell_shader(blob_a, sizeof blob_a, &cmax, &const_sz, false);
    memcpy(blob_b, blob_a, sizeof blob_a);
    put_u64(blob_b + 0x80 + 6 * 8, 0x123456789abcdef0ull);  // dead padding past bra self

    ShaderKey vkey = {.glsl = 1, .flip_y = true, .depth_minus_one = true};
    vkey.attr_kind[0] = 0;
    vkey.attr_kind[1] = 2;

    uint64_t h1a = 0, h2a = 0, h1b = 0, h2b = 0;
    uint32_t tf = MW_TF_BRX_WINDOW | MW_TF_STATIC_STACKS | MW_TF_STRUCTURED;
    assert(mw_stage_key(blob_a, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0x100000000ull, 0x80000000ull, &h1a, &h2a));
    assert(mw_stage_key(blob_b, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    assert(h1a == h1b && h2a == h2b);

    // Non-BRX shader must ignore cb1 changes even with MW_TF_BRX_WINDOW set
    put_u32(blob_b + cmax + 4, 0x99);
    assert(mw_stage_key(blob_b, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    assert(h1a == h1b && h2a == h2b);

    // Unaligned blob pointer must hash identically
    uint8_t blob_unaligned[1025];
    memcpy(blob_unaligned + 1, blob_a, sizeof blob_a);
    assert(mw_stage_key(blob_unaligned + 1, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    assert(h1a == h1b && h2a == h2b);

    // Instruction, SPH, gmem bounds, and every ShaderKey / trans_flags field must perturb the hash
    memcpy(blob_b, blob_a, sizeof blob_a);
    put_u64(blob_b + 0x80 + 1 * 8, 0x5c98078000070001ull ^ 1ull);
    assert_key_differs_vs(blob_b, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0x100000000ull, 0x80000000ull, h1a, h2a);

    memcpy(blob_b, blob_a, sizeof blob_a);
    blob_b[0x35] ^= 0x80u;
    assert_key_differs_vs(blob_b, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0x100000000ull, 0x80000000ull, h1a, h2a);

    assert_key_differs_vs(blob_a, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0x200000000ull, 0x80000000ull, h1a, h2a);
    assert_key_differs_vs(blob_a, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0x100000000ull, 0x40000000ull, h1a, h2a);

    ShaderKey vk_var = vkey; vk_var.flip_x = true;
    assert_key_differs_vs(blob_a, cmax, const_sz, STAGE_VERTEX, &vk_var, tf, 0x100000000ull, 0x80000000ull, h1a, h2a);
    vk_var = vkey; vk_var.flip_y = false;
    assert_key_differs_vs(blob_a, cmax, const_sz, STAGE_VERTEX, &vk_var, tf, 0x100000000ull, 0x80000000ull, h1a, h2a);
    vk_var = vkey; vk_var.depth_minus_one = false;
    assert_key_differs_vs(blob_a, cmax, const_sz, STAGE_VERTEX, &vk_var, tf, 0x100000000ull, 0x80000000ull, h1a, h2a);
    vk_var = vkey; vk_var.attr_kind[1] = 1;
    assert_key_differs_vs(blob_a, cmax, const_sz, STAGE_VERTEX, &vk_var, tf, 0x100000000ull, 0x80000000ull, h1a, h2a);
    vk_var = vkey; vk_var.gmem_linear_lookup = true;
    assert_key_differs_vs(blob_a, cmax, const_sz, STAGE_VERTEX, &vk_var, tf, 0x100000000ull, 0x80000000ull, h1a, h2a);

    for (unsigned bit = 0; bit <= 22; bit++)
        assert_key_differs_vs(blob_a, cmax, const_sz, STAGE_VERTEX, &vkey, tf ^ (1u << bit), 0x100000000ull, 0x80000000ull, h1a, h2a);

    // Fragment ShaderKey fields
    ShaderKey fkey = {.glsl = 1, .flip_y = true, .vs_out_mask = 0x3fu};
    fkey.rt_kind[0] = 0;
    uint64_t fh1 = 0, fh2 = 0;
    assert(mw_stage_key(blob_a, cmax, const_sz, STAGE_FRAGMENT, &fkey, tf, 0x100000000ull, 0x80000000ull, &fh1, &fh2));
    ShaderKey fk_var = fkey; fk_var.vs_out_mask = 0x7fu;
    assert_key_differs_vs(blob_a, cmax, const_sz, STAGE_FRAGMENT, &fk_var, tf, 0x100000000ull, 0x80000000ull, fh1, fh2);
    fk_var = fkey; fk_var.rt_kind[0] = 2;
    assert_key_differs_vs(blob_a, cmax, const_sz, STAGE_FRAGMENT, &fk_var, tf, 0x100000000ull, 0x80000000ull, fh1, fh2);
    fk_var = fkey; fk_var.no_ds_write = true;
    assert_key_differs_vs(blob_a, cmax, const_sz, STAGE_FRAGMENT, &fk_var, tf, 0x100000000ull, 0x80000000ull, fh1, fh2);

    // BRX table sensitivity + negative controls
    build_fake_maxwell_shader(blob_a, sizeof blob_a, &cmax, &const_sz, true);
    memcpy(blob_b, blob_a, sizeof blob_a);
    put_u32(blob_b + cmax + 4, 0x38);
    assert(mw_stage_key(blob_a, cmax, const_sz, STAGE_FRAGMENT, &fkey, tf, 0x100000000ull, 0x80000000ull, &h1a, &h2a));
    assert(mw_stage_key(blob_b, cmax, const_sz, STAGE_FRAGMENT, &fkey, tf, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    assert(h1a != h1b && h2a != h2b);

    // Negative controls: without MW_TF_BRX_WINDOW, without glsl, const_bytes < 4, or cmax >= 0x10000 -> cb1 ignored
    assert(mw_stage_key(blob_a, cmax, const_sz, STAGE_FRAGMENT, &fkey, tf & ~MW_TF_BRX_WINDOW, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    assert(mw_stage_key(blob_b, cmax, const_sz, STAGE_FRAGMENT, &fkey, tf & ~MW_TF_BRX_WINDOW, 0x100000000ull, 0x80000000ull, &h1a, &h2a));
    assert(h1a == h1b && h2a == h2b);

    ShaderKey fk_msl = fkey; fk_msl.glsl = 0;
    assert(mw_stage_key(blob_a, cmax, const_sz, STAGE_FRAGMENT, &fk_msl, tf, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    assert(mw_stage_key(blob_b, cmax, const_sz, STAGE_FRAGMENT, &fk_msl, tf, 0x100000000ull, 0x80000000ull, &h1a, &h2a));
    assert(h1a == h1b && h2a == h2b);

    assert(mw_stage_key(blob_a, cmax, 2, STAGE_FRAGMENT, &fkey, tf, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    assert(mw_stage_key(blob_b, cmax, 2, STAGE_FRAGMENT, &fkey, tf, 0x100000000ull, 0x80000000ull, &h1a, &h2a));
    assert(h1a == h1b && h2a == h2b);

    static uint8_t big_a[0x10040], big_b[0x10040];
    memcpy(big_a, blob_a, sizeof blob_a);
    memcpy(big_b, blob_b, sizeof blob_b);
    assert(mw_stage_key(big_a, 0x10000, 16, STAGE_FRAGMENT, &fkey, tf, 0x100000000ull, 0x80000000ull, &h1b, &h2b));
    big_b[0x10004] = 0x77;
    assert(mw_stage_key(big_b, 0x10000, 16, STAGE_FRAGMENT, &fkey, tf, 0x100000000ull, 0x80000000ull, &h1a, &h2a));
    assert(h1a == h1b && h2a == h2b);

    assert(!mw_stage_key(NULL, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0, 0, &h1a, &h2a));
    assert(!mw_stage_key(blob_a, 0x80, 0, STAGE_VERTEX, &vkey, tf, 0, 0, &h1a, &h2a));
    assert(!mw_stage_key(blob_a, cmax, const_sz, STAGE_COMPUTE, &vkey, tf, 0, 0, &h1a, &h2a));

    // Binary .smeta round-trip and validation
    char dir[512];
    make_test_dir(dir, sizeof dir);

    assert(mw_stage_key(blob_a, cmax, const_sz, STAGE_FRAGMENT, &fkey, tf, 0x100000000ull, 0x80000000ull, &h1a, &h2a));
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

    // Invalid store arguments and overlong directory path rejection
    assert(!mw_meta_store(dir, h1a, h2a, STAGE_COMPUTE, &src, 0x1234ull, false, 1));
    assert(!mw_meta_store(dir, h1a, h2a, STAGE_FRAGMENT, &src, 0ull, false, 1));
    src.ntex = -1;
    assert(!mw_meta_store(dir, h1a, h2a, STAGE_FRAGMENT, &src, 0x1234ull, false, 1));
    src.ntex = 65;
    assert(!mw_meta_store(dir, h1a, h2a, STAGE_FRAGMENT, &src, 0x1234ull, false, 1));
    src.ntex = 2;
    char long_dir[300];
    memset(long_dir, 'a', sizeof long_dir - 1);
    long_dir[sizeof long_dir - 1] = 0;
    assert(!mw_meta_store(long_dir, h1a, h2a, STAGE_FRAGMENT, &src, 0x1234ull, false, 1));
    uint64_t got_spv = 0;
    assert(mw_meta_load(long_dir, h1a, h2a, STAGE_FRAGMENT, &got_spv, NULL, NULL) == NULL);

    // STAGE_FRAGMENT store/load
    assert(mw_meta_store(dir, h1a, h2a, STAGE_FRAGMENT, &src, 0xfeedface12345678ull, false, 3));
    bool got_vid = true;
    uint8_t got_gather = 0;
    ShaderInfo *loaded = mw_meta_load(dir, h1a, h2a, STAGE_FRAGMENT, &got_spv, &got_vid, &got_gather);
    assert(loaded && loaded->msl == NULL && got_spv == 0xfeedface12345678ull && !got_vid && got_gather == 3);
    assert(loaded->cbuf_mask == src.cbuf_mask && loaded->cbuf_size[2] == 512 && loaded->ntex == 2);
    assert(loaded->tex[0].shadow && loaded->tex[0].is_array && loaded->tex[1].is_image && loaded->tex[1].bindless);
    assert(loaded->writes_depth && loaded->uses_discard && loaded->uses_gmem && loaded->uses_gmem_map);
    assert(loaded->uses_tint && loaded->uses_rscale && loaded->local_mem == 128 && loaded->num_insts == 42);
    free(loaded);

    // STAGE_VERTEX roundtrip with vs_uses_vid, writes_point_size, uses_gmem_store, ntex = 0 and ntex = 64
    uint64_t vh1 = 0, vh2 = 0;
    assert(mw_stage_key(blob_a, cmax, const_sz, STAGE_VERTEX, &vkey, tf, 0x100000000ull, 0x80000000ull, &vh1, &vh2));
    ShaderInfo vsrc = {0};
    vsrc.stage = STAGE_VERTEX;
    vsrc.ntex = 0;
    vsrc.writes_point_size = true;
    vsrc.uses_gmem = true;
    vsrc.uses_gmem_store = true;
    assert(mw_meta_store(dir, vh1, vh2, STAGE_VERTEX, &vsrc, 0xabcdef0123456789ull, true, 0));
    loaded = mw_meta_load(dir, vh1, vh2, STAGE_VERTEX, &got_spv, &got_vid, NULL);
    assert(loaded && got_vid && loaded->writes_point_size && loaded->uses_gmem_store && loaded->ntex == 0);
    free(loaded);

    vsrc.ntex = 64;
    for (int i = 0; i < 64; i++) vsrc.tex[i] = (ShaderTexture){.handle_offset = (uint32_t)(i * 8), .type = TEXTYPE_2D};
    assert(mw_meta_store(dir, vh1, vh2, STAGE_VERTEX, &vsrc, 0xabcdef0123456789ull, true, 0));
    loaded = mw_meta_load(dir, vh1, vh2, STAGE_VERTEX, &got_spv, &got_vid, NULL);
    assert(loaded && loaded->ntex == 64 && loaded->tex[63].handle_offset == 63u * 8u);
    free(loaded);

    char vpath[600];
    snprintf(vpath, sizeof vpath, "%s/%016llx.smeta", dir, (unsigned long long)vh1);
    remove(vpath);

    // Rejection checks on STAGE_FRAGMENT file: h2, stage, magic, version, spv_hash=0, bit flip, trailing byte, truncation
    assert(mw_meta_load(dir, h1a, h2a ^ 1ull, STAGE_FRAGMENT, &got_spv, &got_vid, &got_gather) == NULL);
    assert(mw_meta_load(dir, h1a, h2a, STAGE_VERTEX, &got_spv, &got_vid, &got_gather) == NULL);

    char path[600];
    snprintf(path, sizeof path, "%s/%016llx.smeta", dir, (unsigned long long)h1a);
    FILE *f = fopen(path, "rb");
    assert(f != NULL);
    uint8_t raw[sizeof(MwMetaHdr) + 64 * sizeof(MwMetaTex)];
    size_t raw_n = fread(raw, 1, sizeof raw, f);
    fclose(f);
    assert(raw_n == sizeof(MwMetaHdr) + 2 * sizeof(MwMetaTex));

    // Bit flip
    raw[64] ^= 0x5au;
    f = fopen(path, "wb"); assert(f && fwrite(raw, 1, raw_n, f) == raw_n); fclose(f);
    assert(mw_meta_load(dir, h1a, h2a, STAGE_FRAGMENT, &got_spv, &got_vid, &got_gather) == NULL);
    raw[64] ^= 0x5au;

    // Bad magic / version / zero spv_hash (recompute payload_hash for version & spv_hash checks)
    raw[0] = 'X';
    f = fopen(path, "wb"); assert(f && fwrite(raw, 1, raw_n, f) == raw_n); fclose(f);
    assert(mw_meta_load(dir, h1a, h2a, STAGE_FRAGMENT, &got_spv, &got_vid, &got_gather) == NULL);
    raw[0] = 'B';

    MwMetaHdr bad_hdr;
    MwMetaTex bad_tex[2];
    memcpy(&bad_hdr, raw, sizeof bad_hdr);
    memcpy(bad_tex, raw + sizeof bad_hdr, sizeof bad_tex);
    bad_hdr.version = MW_META_VERSION + 1;
    bad_hdr.payload_hash = mw_meta_payload_hash(&bad_hdr, bad_tex, 2);
    f = fopen(path, "wb");
    assert(f && fwrite(&bad_hdr, sizeof bad_hdr, 1, f) == 1 && fwrite(bad_tex, sizeof bad_tex, 1, f) == 1);
    fclose(f);
    assert(mw_meta_load(dir, h1a, h2a, STAGE_FRAGMENT, &got_spv, &got_vid, &got_gather) == NULL);

    bad_hdr.version = MW_META_VERSION;
    bad_hdr.spv_hash = 0;
    bad_hdr.payload_hash = mw_meta_payload_hash(&bad_hdr, bad_tex, 2);
    f = fopen(path, "wb");
    assert(f && fwrite(&bad_hdr, sizeof bad_hdr, 1, f) == 1 && fwrite(bad_tex, sizeof bad_tex, 1, f) == 1);
    fclose(f);
    assert(mw_meta_load(dir, h1a, h2a, STAGE_FRAGMENT, &got_spv, &got_vid, &got_gather) == NULL);

    // Trailing extra byte
    f = fopen(path, "wb");
    assert(f && fwrite(raw, 1, raw_n, f) == raw_n && fputc(0, f) != EOF);
    fclose(f);
    assert(mw_meta_load(dir, h1a, h2a, STAGE_FRAGMENT, &got_spv, &got_vid, &got_gather) == NULL);

    // Truncated 32-byte file
    f = fopen(path, "wb");
    assert(f && fwrite(raw, 1, 32, f) == 32);
    fclose(f);
    assert(mw_meta_load(dir, h1a, h2a, STAGE_FRAGMENT, &got_spv, &got_vid, &got_gather) == NULL);

    remove(path);
    mw_rmdir(dir);
}

static VkFormat test_fmt_fn(uint32_t f) {
    return f == 0x25 ? VK_FORMAT_R8G8B8A8_UNORM : f == 0x2E ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R32G32B32_SFLOAT;
}

static void test_dyn_vi(void) {
    VkDynVi vi;
    uint32_t attribs[4] = {
        0x2Eu | (0u << 8) | (0u << 13),   // loc 0: stream 0, off 0
        0x25u | (1u << 8) | (12u << 13),  // loc 1: stream 1, off 12
        0x00u | (0u << 8) | (0u << 13),   // loc 2: fmt 0 -> ZERO_BINDING
        0x2Eu | (16u << 8) | (4u << 13),  // loc 3: stream 16 -> ZERO_BINDING
    };
    uint32_t streams[3][2] = {
        {32, 0},  // stream 0: vertex rate (divisor 0 -> Vk divisor MUST be 1)
        {16, 4},  // stream 1: instance rate divisor 4
        {0, 5},   // stream 2: stride == 0 disables instance rate -> VERTEX rate, divisor 1
    };

    vk_build_dyn_vi(0, 4, attribs, 3, streams, true, test_fmt_fn, &vi, 7);
    assert(vi.gen == 7 && vi.na == 0 && vi.nb == 0 && vi.used_streams == 0);

    vk_build_dyn_vi(0x1Fu, 4, attribs, 3, streams, true, test_fmt_fn, &vi, 8);
    assert(vi.gen == 8 && vi.na == 5 && vi.nb == 3);
    assert(vi.used_streams == ((1u << 0) | (1u << 1) | (1u << ZERO_BINDING)));
    for (uint32_t i = 0; i < vi.nb; i++) {
        assert(vi.vb[i].sType == VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT);
        assert(vi.vb[i].divisor >= 1u);
    }
    assert(vi.vb[0].binding == 0 && vi.vb[0].stride == 32 && vi.vb[0].inputRate == VK_VERTEX_INPUT_RATE_VERTEX && vi.vb[0].divisor == 1);
    assert(vi.vb[1].binding == 1 && vi.vb[1].stride == 16 && vi.vb[1].inputRate == VK_VERTEX_INPUT_RATE_INSTANCE && vi.vb[1].divisor == 4);
    assert(vi.vb[2].binding == ZERO_BINDING && vi.vb[2].stride == 0 && vi.vb[2].inputRate == VK_VERTEX_INPUT_RATE_VERTEX && vi.vb[2].divisor == 1);

    // Without divisor extension: instance divisor > 1 clamps to 1
    vk_build_dyn_vi(0x1Fu, 4, attribs, 3, streams, false, test_fmt_fn, &vi, 9);
    assert(vi.vb[1].inputRate == VK_VERTEX_INPUT_RATE_INSTANCE && vi.vb[1].divisor == 1 && vi.divisor[1] == 1);

    // have_divisor == true with stream[1][1] == 1 keeps divisor == 1
    streams[1][1] = 1;
    vk_build_dyn_vi(0x1Fu, 4, attribs, 3, streams, true, test_fmt_fn, &vi, 10);
    assert(vi.vb[1].inputRate == VK_VERTEX_INPUT_RATE_INSTANCE && vi.vb[1].divisor == 1);

    // nattrib == 0, nstream == 0 -> all attrs map to ZERO_BINDING
    vk_build_dyn_vi(0x7u, 0, NULL, 0, NULL, true, test_fmt_fn, &vi, 11);
    assert(vi.na == 3 && vi.nb == 1 && vi.vb[0].binding == ZERO_BINDING && vi.vb[0].divisor == 1);

    // stream == nstream & stride == 0 -> VERTEX rate, divisor 1
    uint32_t edge_attrs[2] = {0x2Eu | (2u << 8), 0x2Eu | (3u << 8)};
    vk_build_dyn_vi(0x3u, 2, edge_attrs, 3, streams, true, test_fmt_fn, &vi, 12);
    assert(vi.nb == 2 && vi.vb[0].binding == 2 && vi.vb[0].inputRate == VK_VERTEX_INPUT_RATE_VERTEX && vi.vb[0].divisor == 1);
    assert(vi.vb[1].binding == 3 && vi.vb[1].stride == 0 && vi.vb[1].inputRate == VK_VERTEX_INPUT_RATE_VERTEX && vi.vb[1].divisor == 1);

    // Max 32 attributes across all 16 streams + ZERO_BINDING = 17 bindings
    uint32_t max_attrs[32];
    uint32_t max_streams[16][2];
    for (int i = 0; i < 16; i++) {
        max_streams[i][0] = 16;
        max_streams[i][1] = 0;
    }
    for (int i = 0; i < 32; i++)
        max_attrs[i] = (i == 31) ? (0x2Eu | (16u << 8)) : (0x2Eu | ((uint32_t)(i & 15) << 8));
    vk_build_dyn_vi(0xFFFFFFFFu, 32, max_attrs, 16, max_streams, true, test_fmt_fn, &vi, 13);
    assert(vi.na == 32 && vi.nb == 17 && vi.used_streams == 0x1FFFFu);
}

int main(void) {
    test_job_queue_and_steal();
    test_stage_key_and_smeta();
    test_dyn_vi();
    puts("PASS: test_mali_shader_stutter (queue/steal/drain, smeta key/store/load, dyn_vi)");
    return 0;
}
