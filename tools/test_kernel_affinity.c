// Android regression test of the real core-0 queue and affinity helper; no game files needed.
// Build with -ffunction-sections -fdata-sections -Wl,--gc-sections so unused kernel services are discarded.
#include <assert.h>
#include <stdatomic.h>
#include "../runtime/kernel.c"

volatile int g_fnprof = 1;  // profiling must survive scheduler requests
uint64_t plat_now_ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000000 + t.tv_nsec;
}

static atomic_int ran;
#ifdef __ANDROID__
static int mock_aff_mode;
static int mock_pin_all_calls;
static int mock_pin_hot_calls;
static int mock_adpf_clears;
static int32_t mock_last_adpf_tids[3];
static int mock_last_adpf_count = -1;

int plat_cpu_affinity_mode(void) { return mock_aff_mode; }
bool plat_adpf_guests_enabled(void) { return true; }
bool plat_thread_pin_role(int32_t tid, int role) {
    (void)tid;
    if (role == PLAT_CPU_ROLE_ALL) mock_pin_all_calls++;
    else if (role == PLAT_CPU_ROLE_GUEST_HOT) mock_pin_hot_calls++;
    return true;
}
void plat_adpf_set_guest_tids(const int32_t *tids, int count) {
    mock_last_adpf_count = count;
    for (int i = 0; i < 3; i++) mock_last_adpf_tids[i] = (tids && i < count) ? tids[i] : 0;
    if (count == 0) mock_adpf_clears++;
}
#endif
static void *worker(void *arg) {
    KThread *t = arg;
    t_cur = t;
    core0_acquire(t);
    kernel_preempt_point();
    atomic_fetch_add(&ran, 1);
    core0_release(t);
    return NULL;
}
static void wait_queued(KThread *t) {
    uint64_t end = plat_now_ns() + 2000000000ull;
    for (;;) {
        pthread_mutex_lock(&c0_m);
        bool found = false;
        for (KThread *p = c0_head; p; p = p->core0_next) found |= p == t;
        pthread_mutex_unlock(&c0_m);
        if (found) return;
        assert(plat_now_ns() < end);
        usleep(100);
    }
}
static uint32_t set_mask(KThread *t, int ideal, uint64_t mask) {
    pthread_mutex_lock(&g_klock);
    uint32_t r = thread_set_core_mask(t, ideal, mask);
    pthread_mutex_unlock(&g_klock);
    return r;
}

int main(void) {
    alarm(15);
    setenv("LARECOMP_C0_SLICE_MS", "1", 1);
    KThread owner = {.core = 0, .ideal_core = 0, .core_mask = 1, .started = true};
    KThread t = {.core = 0, .ideal_core = 0, .core_mask = 1};
    assert(set_mask(&t, -1, 2) == R_OK && t.core == 1 && t.ideal_core == -1 && t.core_mask == 2);
    assert(set_mask(&t, 2, 4) == R_OK && t.core == 2);
    assert(set_mask(&t, -3, 6) == R_OK && t.ideal_core == 2 && t.core_mask == 6);
    assert(set_mask(&t, 0, 2) == R_INVALID_COMBINATION && t.core_mask == 6);
    assert(set_mask(&t, -1, 0) == R_INVALID_COMBINATION && t.core_mask == 6);
    assert(set_mask(&t, -4, 1) == R_INVALID_CORE);
    assert(set_mask(&t, -1, 16) == R_INVALID_CORE);
    assert(set_mask(&t, -2, 0) == R_OK && t.core == 0 && t.ideal_core == 0 && t.core_mask == 1);

    for (int i = 0; i < 100; i++) {
        t = (KThread){.core = 0, .ideal_core = 0, .core_mask = 1, .started = true};
        t_cur = &owner;
        core0_acquire(&owner);
        atomic_store(&ran, 0);
        pthread_t th;
        assert(!pthread_create(&th, NULL, worker, &t));
        wait_queued(&t);
        // A waiter reassigned to core 1 must escape even while core 0's owner never yields.
        assert(set_mask(&t, -1, 2) == R_OK);
        pthread_join(th, NULL);
        assert(atomic_load(&ran) == 1 && owner.core0_held && c0_holder == &owner);
        core0_release(&owner);
        assert(!c0_head && !c0_tail && !c0_holder && !(g_fnprof & 2));
    }

    t = (KThread){.core = 0, .ideal_core = 0, .core_mask = 1, .started = true};
    core0_acquire(&owner);
    atomic_store(&ran, 0);
    pthread_t th;
    assert(!pthread_create(&th, NULL, worker, &t));
    wait_queued(&t);
    usleep(20000);  // the slicer has requested preemption, but the old owner hasn't acknowledged it
    assert(atomic_load(&ran) == 0 && c0_holder == &owner && owner.core0_evicted);
    kernel_preempt_point();
    pthread_join(th, NULL);
    assert(atomic_load(&ran) == 1 && owner.core0_held);
    core0_release(&owner);
    assert(g_fnprof == 1 && c0_nevicted == 0);

    // A reboot must let waiters leave the queue even if an owner is still unwinding.
    core0_acquire(&owner);
    atomic_store(&ran, 0);
    assert(!pthread_create(&th, NULL, worker, &t));
    wait_queued(&t);
    pthread_mutex_lock(&c0_m);
    g_rt_unwind = 1;
    pthread_cond_broadcast(&c0_cv);
    pthread_mutex_unlock(&c0_m);
    pthread_join(th, NULL);
    core0_release(&owner);
    assert(!c0_head && !c0_tail && c0_nevicted == 0);

#ifdef __ANDROID__
    // Verify kernel_affinity_tick mode transitions (1 -> 0 and 1 -> 3) cleanly demote promoted guests & clear ADPF.
    KThread promoted = {.started = true, .finished = false, .host_tid = 12345, .aff_promoted = true};
    KThread pending = {.started = true, .finished = false, .host_tid = 0, .aff_promoted = false};
    promoted.anext = &pending;
    g_threads = &promoted;
    g_aff_last_mode = 1;
    g_aff_last_ns = 0;
    g_aff_self_pinned_tid = 999;
    g_aff_self_pinned_mode = 1;
    mock_aff_mode = 0;
    mock_pin_all_calls = 0;
    mock_adpf_clears = 0;
    kernel_affinity_tick();
    assert(!promoted.aff_promoted && mock_pin_all_calls == 2 && mock_adpf_clears == 1 && mock_last_adpf_count == 0);
    assert(g_aff_last_mode == 0 && g_aff_self_pinned_tid == 0);

    // Zero host_tid in dt_ns == 0 must skip safely without crashing on uninitialized t->host.
    mock_aff_mode = 1;
    promoted.aff_promoted = false;
    kernel_affinity_tick();
    promoted.aff_promoted = true;
    mock_aff_mode = 3;
    mock_pin_all_calls = 0;
    kernel_affinity_tick();
    assert(!promoted.aff_promoted && mock_pin_all_calls >= 1 && g_aff_last_mode == 3);
    g_threads = NULL;
    g_aff_last_mode = 0;
#endif
    puts("PASS: affinity validation, 100 queued migrations, safe handoff, reboot wakeup");
}
