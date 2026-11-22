// Android Bionic regression test for kernel lock fast paths, SleepThread(0) fast path,
// targeted WaitSynchronization signaling, and HLE TLS-avoidance counters.
// Build with -ffunction-sections -fdata-sections -Wl,--gc-sections.
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "../runtime/kernel.c"

volatile int g_fnprof = 1;
int g_game_version = 190;
uint64_t g_guest_lo = 0x1000000000ull;
Module g_modules[4];
int g_nmodules = 0;

static atomic_uint_fast64_t g_now_calls;

uint64_t plat_now_ns(void) {
    atomic_fetch_add_explicit(&g_now_calls, 1, memory_order_relaxed);
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec;
}
void plat_commit(uint64_t a, uint64_t s) { (void)a, (void)s; }
void plat_thread_unpin(void) {}
void plat_guest_thread_affinity(void) {}
void plat_print_backtrace(void) {}
const char *plat_symbol(uint64_t a, uint64_t *o) { (void)a, (void)o; return NULL; }
void rt_fatal_message(const char *fmt, ...) { (void)fmt; abort(); }
void crash_handler_install(void) {}
bool native_on(void) { return false; }
uint64_t native_ticks(void) { return 0; }
void native_thread_end(void) {}
void guest_call(Cpu *c, uint64_t addr) { (void)c, (void)addr; }
const char *hle_slot_name(uint64_t pc) { (void)pc; return NULL; }
uint32_t ipc_connect_named_port(const char *n) { (void)n; return 0; }
int ipc_handle_request(Cpu *c, uint32_t h) { (void)c, (void)h; return 0; }
int vk_ab(const char *what) { (void)what; return -1; }
bool vk_heap_place(uint64_t size) { (void)size; return false; }
uint32_t vi_vsync_handle(void) { return 0; }

typedef struct {
    void (*fn)(void);
    const char *name;
} ForkCase;

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

static atomic_int g_handoff_ran;
static void *core0_waiter_fn(void *arg) {
    KThread *t = (KThread *)arg;
    t_cur = t;
    core0_acquire(t);
    atomic_fetch_add(&g_handoff_ran, 1);
    core0_release(t);
    return NULL;
}

static void wait_core_waiters(int core, int want) {
    uint64_t end = plat_now_ns() + 2000000000ull;
    for (;;) {
        if (__atomic_load_n(&cq_nwaiters[core], __ATOMIC_ACQUIRE) == want) return;
        assert(plat_now_ns() < end);
        usleep(100);
    }
}

// 1. Core 1-3 uncontended fast path + migration onto core 0
static void case_off_core0_fastpath_and_migrate(void) {
    unsetenv("LARECOMP_CORE_LOCKS");
    unsetenv("LARECOMP_NO_CORE0");
    unsetenv("LARECOMP_NO_FAST_YIELD");
    unsetenv("LARECOMP_PROF_SVC");
    unsetenv("LARECOMP_TRACE_SVC");
    kernel_init();
    assert(__atomic_load_n(&c0_ready, __ATOMIC_ACQUIRE) == true);
    assert(__atomic_load_n(&g_core_turns_past0, __ATOMIC_ACQUIRE) == false);

    for (int core = 1; core <= 3; core++) {
        KThread t = {.core = core, .ideal_core = core, .core_mask = 1ull << core, .started = true};
        t.cpu.thread = &t;
        t_cur = &t;

        uint64_t calls0 = atomic_load(&g_now_calls);
        core0_acquire(&t);
        core0_release(&t);
        assert(!t.core0_held && !c0_holder);
        assert(atomic_load(&g_now_calls) == calls0);

        // SleepThread(0) on parallel core hits fast return without touching clock or core lock.
        t.cpu.x[0] = 0;
        rt_svc(&t.cpu, 0x0B, 0x1000);
        assert(atomic_load(&g_now_calls) == calls0);
        assert(t.svc_since == 0 && t.svc_now == 0);
    }

    // Migrate a thread from core 2 onto locked core 0 via SetThreadCoreMask + kernel_preempt_point.
    KThread m = {.core = 2, .ideal_core = 2, .core_mask = 0x5, .started = true};
    m.cpu.thread = &m;
    t_cur = &m;
    pthread_mutex_lock(&g_klock);
    assert(thread_set_core_mask(&m, 0, 1) == R_OK);
    pthread_mutex_unlock(&g_klock);
    assert(__atomic_load_n(&m.core, __ATOMIC_ACQUIRE) == 0);
    assert(__atomic_load_n(&m.core0_evicted, __ATOMIC_ACQUIRE) == true);

    kernel_preempt_point();
    assert(m.core0_held && m.lock_core == 0 && c0_holder == &m);
    core0_release(&m);
    assert(!m.core0_held && !c0_holder);
}

// 2. Core 0 uncontended SleepThread(0) fast path vs contended handoff + LARECOMP_NO_FAST_YIELD rollback
static void case_core0_yield_fast_vs_contended(void) {
    unsetenv("LARECOMP_CORE_LOCKS");
    unsetenv("LARECOMP_NO_CORE0");
    unsetenv("LARECOMP_NO_FAST_YIELD");
    unsetenv("LARECOMP_PROF_SVC");
    unsetenv("LARECOMP_TRACE_SVC");
    kernel_init();

    KThread owner = {.core = 0, .ideal_core = 0, .core_mask = 1, .started = true, .id = 10};
    owner.cpu.thread = &owner;
    t_cur = &owner;
    core0_acquire(&owner);
    assert(owner.core0_held && c0_holder == &owner);

    uint64_t takes0 = c0_stat.takes;
    uint64_t calls0 = atomic_load(&g_now_calls);
    owner.cpu.x[0] = 0;
    rt_svc(&owner.cpu, 0x0B, 0x2000);
    // Zero clock lookups and zero core drop/acquire cycles when uncontended!
    assert(atomic_load(&g_now_calls) == calls0);
    assert(c0_stat.takes == takes0);
    assert(owner.core0_held && c0_holder == &owner);

    // Non-blocking SVC (GetCurrentProcessorNumber, 0x10) must not read clock or set svc_since.
    rt_svc(&owner.cpu, 0x10, 0x2004);
    assert(owner.cpu.x[0] == 0);
    assert(atomic_load(&g_now_calls) == calls0);
    assert(owner.svc_since == 0 && owner.svc_now == 0);

    // Fallback test when c->thread is NULL (uses t_cur).
    Cpu no_thread_cpu = {.x = {0}, .thread = NULL};
    rt_svc(&no_thread_cpu, 0x0B, 0x2008);
    assert(atomic_load(&g_now_calls) == calls0);

    // Contended SleepThread(0): waiter queued in cq[0] -> cq_nwaiters[0] == 1 -> handoff must happen.
    KThread waiter = {.core = 0, .ideal_core = 0, .core_mask = 1, .started = true, .id = 11};
    waiter.cpu.thread = &waiter;
    atomic_store(&g_handoff_ran, 0);
    pthread_t th;
    assert(pthread_create(&th, NULL, core0_waiter_fn, &waiter) == 0);
    wait_core_waiters(0, 1);

    calls0 = atomic_load(&g_now_calls);
    owner.cpu.x[0] = 0;
    rt_svc(&owner.cpu, 0x0B, 0x200C);
    pthread_join(th, NULL);
    assert(atomic_load(&g_handoff_ran) == 1);
    assert(atomic_load(&g_now_calls) > calls0);
    assert(owner.core0_held && c0_holder == &owner);
    assert(__atomic_load_n(&cq_nwaiters[0], __ATOMIC_ACQUIRE) == 0);
    core0_release(&owner);
}

static void case_no_fast_yield_env(void) {
    unsetenv("LARECOMP_CORE_LOCKS");
    unsetenv("LARECOMP_NO_CORE0");
    setenv("LARECOMP_NO_FAST_YIELD", "1", 1);
    unsetenv("LARECOMP_PROF_SVC");
    unsetenv("LARECOMP_TRACE_SVC");
    kernel_init();

    KThread owner = {.core = 0, .ideal_core = 0, .core_mask = 1, .started = true};
    owner.cpu.thread = &owner;
    t_cur = &owner;
    core0_acquire(&owner);

    uint64_t takes0 = c0_stat.takes;
    uint64_t calls0 = atomic_load(&g_now_calls);
    owner.cpu.x[0] = 0;
    rt_svc(&owner.cpu, 0x0B, 0x3000);
    // Slow path releases and reacquires core 0, incrementing c0_stat.takes and plat_now_ns calls.
    assert(atomic_load(&g_now_calls) > calls0);
    assert(c0_stat.takes > takes0);
    core0_release(&owner);
}

// 3. Targeted WaitSynchronization wakeup + >8 handle overflow
typedef struct {
    uint32_t handles[12];
    int n;
    int32_t out_idx;
    uint32_t out_r;
    atomic_int done;
    KThread *t;
} SyncWaitArgs;

static void *sync_wait_worker(void *arg) {
    SyncWaitArgs *w = (SyncWaitArgs *)arg;
    t_cur = w->t;
    w->out_r = svc_wait_synchronization((uint64_t)(uintptr_t)w->handles, w->n, -1, &w->out_idx);
    atomic_store(&w->done, 1);
    return NULL;
}

static void wait_in_sync(KThread *t) {
    uint64_t end = plat_now_ns() + 2000000000ull;
    for (;;) {
        pthread_mutex_lock(&g_klock);
        bool waiting = t->in_sync_wait;
        pthread_mutex_unlock(&g_klock);
        if (waiting) return;
        assert(plat_now_ns() < end);
        usleep(100);
    }
}

static void case_targeted_wait_sync(void) {
    kernel_init();
    KEvent ev[10];
    uint32_t hs[10];
    for (int i = 0; i < 10; i++) {
        memset(&ev[i], 0, sizeof ev[i]);
        ev[i].type = KOBJ_EVENT;
        hs[i] = kernel_new_handle(&ev[i]);
    }

    KThread tA = {.core = 1, .core_mask = 2, .id = 20};
    KThread tB = {.core = 1, .core_mask = 2, .id = 21};
    kcond_init(&tA.cond);
    kcond_init(&tB.cond);

    SyncWaitArgs wA = {.handles = {hs[0]}, .n = 1, .out_idx = -1, .t = &tA};
    SyncWaitArgs wB = {.handles = {hs[1]}, .n = 1, .out_idx = -1, .t = &tB};
    pthread_t thA, thB;
    assert(pthread_create(&thA, NULL, sync_wait_worker, &wA) == 0);
    assert(pthread_create(&thB, NULL, sync_wait_worker, &wB) == 0);
    wait_in_sync(&tA);
    wait_in_sync(&tB);

    // Mark ev[1]->signaled directly without signaling tB: if kernel_signal(ev[0]) falsely broadcasts
    // to g_syncwaiters, tB would see ev[1]->signaled and wake! With targeted signaling, tB must stay asleep.
    pthread_mutex_lock(&g_klock);
    assert(tA.sync_nobjs == 1 && !tA.sync_overflow && tA.sync_objs[0] == &ev[0]);
    assert(tB.sync_nobjs == 1 && !tB.sync_overflow && tB.sync_objs[0] == &ev[1]);
    ev[1].signaled = true;
    kernel_signal(&ev[0]);
    pthread_mutex_unlock(&g_klock);

    pthread_join(thA, NULL);
    assert(atomic_load(&wA.done) == 1 && wA.out_r == R_OK && wA.out_idx == 0);
    usleep(10000);
    assert(atomic_load(&wB.done) == 0);

    pthread_mutex_lock(&g_klock);
    kernel_signal(&ev[1]);
    pthread_mutex_unlock(&g_klock);
    pthread_join(thB, NULL);
    assert(atomic_load(&wB.done) == 1 && wB.out_r == R_OK && wB.out_idx == 0);

    // >8 handles overflow: wait on 10 handles, signal ev[9].
    for (int i = 0; i < 10; i++) ev[i].signaled = false;
    KThread tO = {.core = 2, .core_mask = 4, .id = 22};
    kcond_init(&tO.cond);
    SyncWaitArgs wO = {.n = 10, .out_idx = -1, .t = &tO};
    memcpy(wO.handles, hs, sizeof hs);
    pthread_t thO;
    assert(pthread_create(&thO, NULL, sync_wait_worker, &wO) == 0);
    wait_in_sync(&tO);

    pthread_mutex_lock(&g_klock);
    assert(tO.sync_nobjs == 8 && tO.sync_overflow == true);
    kernel_signal(&ev[9]);
    pthread_mutex_unlock(&g_klock);
    pthread_join(thO, NULL);
    assert(wO.out_r == R_OK && wO.out_idx == 9 && !tO.sync_overflow && tO.sync_nobjs == 0);

    // kernel_reset resets sync waiters & cq_nwaiters cleanly.
    kernel_reset();
    for (int i = 0; i < 4; i++) assert(__atomic_load_n(&cq_nwaiters[i], __ATOMIC_ACQUIRE) == 0);
}

// 4. g_core_turns_past0 off vs on (hle_depth / turn_owed)
static void case_hle_turns_past0_off(void) {
    unsetenv("LARECOMP_CORE_LOCKS");
    unsetenv("LARECOMP_NO_CORE0");
    kernel_init();
    assert(kernel_turns_past_core0() == false);
    assert(__atomic_load_n(&g_core_turns_past0, __ATOMIC_ACQUIRE) == false);

    KThread t = {.core = 1, .ideal_core = 1, .core_mask = 2, .started = true};
    t_cur = &t;
    kernel_hle_enter_t(&t);
    assert(t.hle_depth == 0 && !t.turn_owed);
    kernel_hle_leave_t(&t);
    assert(t.hle_depth == 0 && !t.turn_owed && !t.core0_held);
}

static void case_hle_turns_past0_on(void) {
    setenv("LARECOMP_CORE_LOCKS", "7", 1);
    unsetenv("LARECOMP_NO_CORE0");
    kernel_init();
    assert(kernel_turns_past_core0() == true);
    assert(__atomic_load_n(&g_core_turns_past0, __ATOMIC_ACQUIRE) == true);

    KThread t = {.core = 1, .ideal_core = 1, .core_mask = 2, .started = true};
    t_cur = &t;
    core0_acquire(&t);
    assert(t.core0_held && t.lock_core == 1 && cq[1].holder == &t);

    kernel_hle_enter_t(&t);
    assert(t.hle_depth == 1 && !t.turn_owed);
    kernel_blocking_begin();
    assert(!t.core0_held && t.blocking_depth == 1);
    kernel_blocking_end();
    // Inside HLE with past-0 turns active: must defer reacquire until kernel_hle_leave_t!
    assert(!t.core0_held && t.turn_owed && t.blocking_depth == 0);
    assert(t.blocked_ns > 0);
    assert(kernel_blocked_ns() == t.blocked_ns);

    kernel_hle_leave_t(&t);
    assert(t.hle_depth == 0 && !t.turn_owed && t.core0_held && cq[1].holder == &t);
    core0_release(&t);
    assert(!t.core0_held && !cq[1].holder);
}

int main(void) {
    printf("test_kernel_fastpath:\n");
    run_in_fork(case_off_core0_fastpath_and_migrate, "off-core0 fast path and migration");
    run_in_fork(case_core0_yield_fast_vs_contended, "core0 uncontended SleepThread(0) vs contended handoff");
    run_in_fork(case_no_fast_yield_env, "LARECOMP_NO_FAST_YIELD rollback");
    run_in_fork(case_targeted_wait_sync, "targeted WaitSynchronization and >8 handle overflow");
    run_in_fork(case_hle_turns_past0_off, "g_core_turns_past0 off skips hle_depth");
    run_in_fork(case_hle_turns_past0_on, "g_core_turns_past0 on tracks hle_depth/turn_owed/blocked_ns");
    puts("PASS: kernel fast paths, targeted WaitSync, and HLE turn tracking");
    return 0;
}
