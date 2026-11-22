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

    // Large command list (>512 KB) must NOT be retained in snap_pool even when pool is completely empty.
    pthread_mutex_lock(&snap_pool_lock);
    while (snap_pool_n > 0) free(snap_pool[--snap_pool_n].buf);
    pthread_mutex_unlock(&snap_pool_lock);

    size_t chunk_sz = 64u << 10;  // 9 * 64 KB = 576 KB > 512 KB limit
    uint8_t *big = calloc(10, chunk_sz);
    assert(big != NULL);
    for (int i = 0; i < 9; i++) {
        CmdRec *r = (CmdRec *)(big + (size_t)i * chunk_sz);
        r->op = CMD_BARRIER;
        r->size = (uint32_t)chunk_sz;
    }
    CmdRec *end_r = (CmdRec *)(big + 9 * chunk_sz);
    end_r->op = CMD_END;
    end_r->size = sizeof(CmdRec);

    atomic_store(&g_exec_count, 0);
    vk_async_execute((uint64_t)(uintptr_t)big);
    vk_async_drain();
    free(big);
    assert(atomic_load(&g_exec_count) == 1);
    assert(g_exec_handles[0] != 0);
    pthread_mutex_lock(&snap_pool_lock);
    assert(snap_pool_n == 0);
    pthread_mutex_unlock(&snap_pool_lock);
}

// 2. LARECOMP_NO_SNAP_POOL=1 disables pooling cleanly
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
    assert(snap_pool_n == 0);
    pthread_mutex_unlock(&snap_pool_lock);
}

int main(void) {
    printf("test_async_snap_pool:\n");
    run_in_fork(case_snap_pool_reuse_and_limits, "snapshot pool reuse, 16-slot cap, >512KB free, and REQ_CALL safety");
    run_in_fork(case_no_snap_pool_env, "LARECOMP_NO_SNAP_POOL rollback");
    puts("PASS: async command-list snapshot pooling");
    return 0;
}
