// Android Bionic regression test for vulkan/async.c snapshot buffer pooling.
// Build with -ffunction-sections -fdata-sections -Wl,--gc-sections.
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "../runtime/nvn/vulkan/async.c"

// ---- external runtime stubs required by async.c
bool g_benchmark = false;
uint64_t g_guest_lo = 0;

uint64_t plat_now_ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec;
}
void plat_thread_unpin(void) {}
void plat_render_thread_pin(void) {}
void plat_render_work(uint64_t work_ns, uint64_t target_ns) { (void)work_ns, (void)target_ns; }
void rt_fatal_message(const char *fmt, ...) { (void)fmt; abort(); }

void vk_wd_set(uint32_t stage, uint64_t arg) { (void)stage, (void)arg; }
WdMark vk_wd_enter(uint32_t stage, uint64_t arg) { (void)stage, (void)arg; return (WdMark){0}; }
void vk_wd_leave(WdMark m) { (void)m; }
void vk_tl(const char *what, uint64_t a, uint64_t b) { (void)what, (void)a, (void)b; }
void vk_present_now(NvnTexture *t, uint64_t asked) { (void)t, (void)asked; }
uint64_t vk_adpf_budget_ns(void) { return 16666667ull; }
void nvn_sync_set_seq(uint64_t g, uint64_t s) { (void)g, (void)s; }
uint64_t renderer_fence_seq(void) { return 1; }
void kernel_blocking_begin(void) {}
void kernel_blocking_end(void) {}
void bench_handoff(uint64_t ns) { (void)ns; }

static pthread_mutex_t g_exec_gate_m = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_exec_gate_cv = PTHREAD_COND_INITIALIZER;
static bool g_exec_hold = false;

static uint64_t g_exec_handles[64];
static atomic_int g_exec_count;

void vk_execute_now(uint64_t handle) {
    pthread_mutex_lock(&g_exec_gate_m);
    while (g_exec_hold) pthread_cond_wait(&g_exec_gate_cv, &g_exec_gate_m);
    pthread_mutex_unlock(&g_exec_gate_m);

    int idx = atomic_fetch_add(&g_exec_count, 1);
    if (idx < 64) g_exec_handles[idx] = handle;

    const CmdRec *r = (const CmdRec *)(uintptr_t)handle;
    assert(r != NULL);
    assert(r->op == CMD_BARRIER || r->op == CMD_END);
}

static atomic_int g_call_ran;
static void *g_expected_call_arg = NULL;
static void test_call_cb(void *arg) {
    assert(arg == g_expected_call_arg);
    atomic_fetch_add(&g_call_ran, 1);
}

static void init_small_list(CmdRec *recs) {
    memset(recs, 0, sizeof(CmdRec) * 2);
    recs[0] = (CmdRec){.op = CMD_BARRIER, .size = sizeof(CmdRec)};
    recs[1] = (CmdRec){.op = CMD_END, .size = sizeof(CmdRec)};
}

static void run_in_fork(void (*fn)(void), const char *name) {
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        alarm(15);
        fn();
        _exit(0);
    }
    int status = 0;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    printf("  ok: %s\n", name);
}

// 1. Reuse of <=512 KB snapshot buffers, cap at 16 slots, >512 KB freed, REQ_CALL not pooled
static void case_snap_pool_reuse_and_limits(void) {
    unsetenv("LARECOMP_NO_SNAP_POOL");
    unsetenv("LARECOMP_NO_LIST_COPY");

    CmdRec small[2];
    init_small_list(small);

    // First execute + drain allocates one 64 KB buffer and recycles it into snap_pool.
    atomic_store(&g_exec_count, 0);
    vk_async_execute((uint64_t)(uintptr_t)small);
    vk_async_drain();
    assert(atomic_load(&g_exec_count) == 1);
    uint64_t ptr1 = g_exec_handles[0];
    assert(ptr1 != 0);
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == 1 && snap_pool[0].buf == (uint8_t *)(uintptr_t)ptr1 && snap_pool[0].cap == 0x10000);
    pthread_mutex_unlock(&snap_pool_lock);

    // Second execute reuses the exact same pooled buffer pointer.
    vk_async_execute((uint64_t)(uintptr_t)small);
    vk_async_drain();
    assert(atomic_load(&g_exec_count) == 2);
    assert(g_exec_handles[1] == ptr1);
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == 1);
    pthread_mutex_unlock(&snap_pool_lock);

    // Gate render_main so 18 concurrent EXEC snapshots stay in-flight, plus 1 REQ_CALL while busy.
    pthread_mutex_lock(&g_exec_gate_m);
    g_exec_hold = true;
    pthread_mutex_unlock(&g_exec_gate_m);

    atomic_store(&g_exec_count, 0);
    for (int i = 0; i < 18; i++) vk_async_execute((uint64_t)(uintptr_t)small);

    void *call_arg = calloc(1, 64);
    assert(call_arg != NULL);
    g_expected_call_arg = call_arg;
    atomic_store(&g_call_ran, 0);
    vk_async_call(test_call_cb, call_arg);

    pthread_mutex_lock(&g_exec_gate_m);
    g_exec_hold = false;
    pthread_cond_broadcast(&g_exec_gate_cv);
    pthread_mutex_unlock(&g_exec_gate_m);

    vk_async_drain();
    assert(atomic_load(&g_exec_count) == 18);
    assert(atomic_load(&g_call_ran) == 1);
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == SNAP_POOL_MAX);
    for (int i = 0; i < snap_pool_n; i++) assert(snap_pool[i].buf != (uint8_t *)call_arg);
    pthread_mutex_unlock(&snap_pool_lock);

    // Large command list (>512 KB, e.g. ~2 MB -> 4 MB cap) IS pooled and reused by the next large list.
    pthread_mutex_lock(&snap_pool_lock);
    while (snap_pool_n > 0) free(snap_pool[--snap_pool_n].buf);
    snap_pool_bytes = 0;
    pthread_mutex_unlock(&snap_pool_lock);

    size_t chunk_sz = 64u << 10;
    int nchunks = 32;  // 2 MB -> snap_add grows cap to 4 MB
    uint8_t *big = calloc((size_t)(nchunks + 1), chunk_sz);
    assert(big != NULL);
    for (int i = 0; i < nchunks; i++) {
        CmdRec *r = (CmdRec *)(big + (size_t)i * chunk_sz);
        r->op = CMD_BARRIER;
        r->size = (uint32_t)chunk_sz;
    }
    CmdRec *end_r = (CmdRec *)(big + (size_t)nchunks * chunk_sz);
    end_r->op = CMD_END;
    end_r->size = sizeof(CmdRec);

    atomic_store(&g_exec_count, 0);
    vk_async_execute((uint64_t)(uintptr_t)big);
    vk_async_drain();
    assert(atomic_load(&g_exec_count) == 1);
    uint64_t big_ptr1 = g_exec_handles[0];
    assert(big_ptr1 != 0);
    size_t big_cap = 0;
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == 1 && snap_pool[0].buf == (uint8_t *)(uintptr_t)big_ptr1);
    big_cap = snap_pool[0].cap;
    assert(big_cap >= (2u << 20) && snap_pool_bytes == big_cap);
    pthread_mutex_unlock(&snap_pool_lock);

    // Push a small 64 KB buffer BEFORE big executes again so [big, 64KB] or [64KB, big] order tests largest-first pop.
    void *small_extra = malloc(0x10000);
    assert(small_extra != NULL);
    snap_pool_recycle(small_extra, 0x10000);
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == 2 && snap_pool_bytes == big_cap + 0x10000);
    pthread_mutex_unlock(&snap_pool_lock);

    // Next large execute must pop the largest (big_ptr1), not small_extra!
    vk_async_execute((uint64_t)(uintptr_t)big);
    vk_async_drain();
    assert(atomic_load(&g_exec_count) == 2);
    assert(g_exec_handles[1] == big_ptr1);
    free(big);

    // Reset clears pool completely.
    vk_async_pool_reset();
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == 0 && snap_pool_bytes == 0);
    pthread_mutex_unlock(&snap_pool_lock);
}

// 2. Eviction of smaller slots at 16-slot limit, 32 MB total byte cap, >16 MB free, and reset
static void case_large_caps_and_eviction(void) {
    unsetenv("LARECOMP_NO_SNAP_POOL");
    vk_async_pool_reset();

    // Fill all 16 slots with 64 KB buffers (1 MB total).
    for (int i = 0; i < SNAP_POOL_MAX; i++) {
        void *p = malloc(0x10000);
        assert(p != NULL);
        snap_pool_recycle(p, 0x10000);
    }
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == SNAP_POOL_MAX && snap_pool_bytes == (size_t)SNAP_POOL_MAX * 0x10000);
    pthread_mutex_unlock(&snap_pool_lock);

    // Recycling a 2 MB buffer when 16 slots are full must evict one smaller (64 KB) slot and retain the 2 MB buffer.
    void *p2m = malloc(2u << 20);
    assert(p2m != NULL);
    snap_pool_recycle(p2m, 2u << 20);
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == SNAP_POOL_MAX);
    assert(snap_pool_bytes == ((size_t)(SNAP_POOL_MAX - 1) * 0x10000) + (2u << 20));
    bool found_2m = false;
    for (int i = 0; i < snap_pool_n; i++) {
        if (snap_pool[i].buf == (uint8_t *)p2m && snap_pool[i].cap == (2u << 20)) found_2m = true;
    }
    assert(found_2m);
    pthread_mutex_unlock(&snap_pool_lock);

    // Hard 32 MB byte cap: reset, add four 8 MB buffers (= 32 MB), then a fifth 8 MB buffer (same size -> no smaller slots) must be freed not stored.
    vk_async_pool_reset();
    for (int i = 0; i < 4; i++) {
        void *p8m = malloc(8u << 20);
        assert(p8m != NULL);
        snap_pool_recycle(p8m, 8u << 20);
    }
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == 4 && snap_pool_bytes == SNAP_POOL_BYTES_MAX);
    pthread_mutex_unlock(&snap_pool_lock);

    void *p8m_over = malloc(8u << 20);
    assert(p8m_over != NULL);
    snap_pool_recycle(p8m_over, 8u << 20);
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == 4 && snap_pool_bytes == SNAP_POOL_BYTES_MAX);
    for (int i = 0; i < snap_pool_n; i++) assert(snap_pool[i].buf != (uint8_t *)p8m_over);
    pthread_mutex_unlock(&snap_pool_lock);

    // Recycling a 12 MB buffer with two 64 KB + two 12 MB etc.: evict smaller slots to fit within 32 MB.
    vk_async_pool_reset();
    for (int i = 0; i < 3; i++) {
        void *p = malloc(8u << 20);
        assert(p != NULL);
        snap_pool_recycle(p, 8u << 20);
    }
    for (int i = 0; i < 4; i++) {
        void *p = malloc(2u << 20);
        assert(p != NULL);
        snap_pool_recycle(p, 2u << 20);
    }
    // Pool has 3*8MB + 4*2MB = 32 MB. Recycling a 4 MB buffer evicts two 2 MB slots -> 3*8 + 2*2 + 4 = 32 MB.
    void *p4m = malloc(4u << 20);
    assert(p4m != NULL);
    snap_pool_recycle(p4m, 4u << 20);
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_bytes <= SNAP_POOL_BYTES_MAX);
    bool found_4m = false;
    for (int i = 0; i < snap_pool_n; i++) {
        if (snap_pool[i].buf == (uint8_t *)p4m) found_4m = true;
    }
    assert(found_4m);
    pthread_mutex_unlock(&snap_pool_lock);

    // Buffer >16 MB single-buffer cap must be freed immediately even when pool is empty.
    vk_async_pool_reset();
    void *too_big = malloc(17u << 20);
    assert(too_big != NULL);
    snap_pool_recycle(too_big, 17u << 20);
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == 0 && snap_pool_bytes == 0);
    pthread_mutex_unlock(&snap_pool_lock);
}

// 3. LARECOMP_NO_SNAP_POOL=1 disables pooling cleanly
static void case_no_snap_pool_env(void) {
    setenv("LARECOMP_NO_SNAP_POOL", "1", 1);
    unsetenv("LARECOMP_NO_LIST_COPY");
    CmdRec small[2];
    init_small_list(small);
    atomic_store(&g_exec_count, 0);
    vk_async_execute((uint64_t)(uintptr_t)small);
    vk_async_execute((uint64_t)(uintptr_t)small);
    vk_async_drain();
    assert(atomic_load(&g_exec_count) == 2);
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == 0 && snap_pool_bytes == 0);
    pthread_mutex_unlock(&snap_pool_lock);
}

int main(void) {
    printf("test_async_snap_pool:\n");
    run_in_fork(case_snap_pool_reuse_and_limits, "snapshot pool reuse (64KB + 4MB), largest-first pop, and REQ_CALL safety");
    run_in_fork(case_large_caps_and_eviction, "smaller-slot eviction, 32MB total byte cap, >16MB free, and reset");
    run_in_fork(case_no_snap_pool_env, "LARECOMP_NO_SNAP_POOL rollback");
    puts("PASS: async command-list snapshot pooling");
    return 0;
}
