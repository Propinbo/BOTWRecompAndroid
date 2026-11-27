// Standalone host/Bionic unit test for runtime/platform/cpu_topo.h + live sysfs & pthread inheritance check.
#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __ANDROID__
#include <sched.h>
#include <unistd.h>
#endif

#include "../runtime/platform/cpu_topo.h"

static void test_dimensity_9400_plus_capacity(void) {
    // SM-X730 sysfs cpu_capacity: 4x A720 (502,502,502,472) + 3x X4 (880) + 1x X925 (1024)
    const uint32_t cap[8] = {502, 502, 502, 472, 880, 880, 880, 1024};
    const long khz[8] = {2400000, 2400000, 2400000, 2400000, 3300000, 3300000, 3300000, 3730000};
    PlatCpuTopo t;
    plat_cpu_topo_classify(cap, khz, 8, 0xFFull, &t);

    assert(t.valid);
    assert(t.used_capacity);
    assert(t.allowed_mask == 0xFFull);
    assert(t.little_mask == 0x0Full && t.n_little == 4);
    assert(t.big_mask == 0xF0ull && t.n_big == 4);
    assert(t.mid_big_mask == 0x70ull && t.n_mid_big == 3);
    assert(t.prime_mask == 0x80ull && t.n_prime == 1);

    // Off (mode 0) must return 0 (no-op) for every role.
    assert(plat_cpu_topo_mask_for_role(&t, 0, PLAT_CPU_ROLE_RENDER) == 0);
    assert(plat_cpu_topo_mask_for_role(&t, 0, PLAT_CPU_ROLE_WORKER) == 0);

    // Cluster mode (1): hot threads share 4-7, workers bound strictly to 0-3 (never single-core {7}).
    assert(plat_cpu_topo_mask_for_role(&t, 1, PLAT_CPU_ROLE_RENDER) == 0xF0ull);
    assert(plat_cpu_topo_mask_for_role(&t, 1, PLAT_CPU_ROLE_PRESENT) == 0xF0ull);
    assert(plat_cpu_topo_mask_for_role(&t, 1, PLAT_CPU_ROLE_GUEST_HOT) == 0xF0ull);
    assert(plat_cpu_topo_mask_for_role(&t, 1, PLAT_CPU_ROLE_WORKER) == 0x0Full);
    assert(plat_cpu_topo_mask_for_role(&t, 1, PLAT_CPU_ROLE_ALL) == 0xFFull);

    // Split mode (2): render/present stay 4-7 (non-parkable X4 fallback), hot guests 4-6, workers 0-3.
    assert(plat_cpu_topo_mask_for_role(&t, 2, PLAT_CPU_ROLE_RENDER) == 0xF0ull);
    assert(plat_cpu_topo_mask_for_role(&t, 2, PLAT_CPU_ROLE_PRESENT) == 0xF0ull);
    assert(plat_cpu_topo_mask_for_role(&t, 2, PLAT_CPU_ROLE_GUEST_HOT) == 0x70ull);
    assert(plat_cpu_topo_mask_for_role(&t, 2, PLAT_CPU_ROLE_WORKER) == 0x0Full);

    // Render+Workers mode (3): render/present 4-7, workers 0-3, guests unpinned (0).
    assert(plat_cpu_topo_mask_for_role(&t, 3, PLAT_CPU_ROLE_RENDER) == 0xF0ull);
    assert(plat_cpu_topo_mask_for_role(&t, 3, PLAT_CPU_ROLE_PRESENT) == 0xF0ull);
    assert(plat_cpu_topo_mask_for_role(&t, 3, PLAT_CPU_ROLE_GUEST_HOT) == 0);
    assert(plat_cpu_topo_mask_for_role(&t, 3, PLAT_CPU_ROLE_WORKER) == 0x0Full);

    // Never output a 1-core prime mask standalone in any mode.
    for (int m = 0; m <= 3; m++) {
        for (int r = 0; r <= 4; r++) {
            uint64_t mask = plat_cpu_topo_mask_for_role(&t, m, r);
            assert(mask != 0x80ull);
        }
    }
}

static void test_max_freq_fallback(void) {
    const uint32_t cap[8] = {0};
    const long khz[8] = {2400000, 2400000, 2400000, 2400000, 3300000, 3300000, 3300000, 3730000};
    PlatCpuTopo t;
    plat_cpu_topo_classify(cap, khz, 8, 0xFFull, &t);

    assert(t.valid);
    assert(!t.used_capacity);
    assert(t.little_mask == 0x0Full);
    assert(t.big_mask == 0xF0ull);
    assert(t.mid_big_mask == 0x70ull);
}

static void test_sd8_gen3_and_invalid_topologies(void) {
    // Snapdragon 8 Gen 3: 2 little + 5 big + 1 prime -> valid=true
    const uint32_t cap_8g3[8] = {380, 380, 720, 720, 720, 810, 810, 1024};
    PlatCpuTopo t;
    plat_cpu_topo_classify(cap_8g3, NULL, 8, 0xFFull, &t);
    assert(t.valid);
    assert(t.little_mask == 0x03ull && t.n_little == 2);
    assert(t.big_mask == 0xFCull && t.n_big == 6);

    // Snapdragon 8 Elite (SM8750 Oryon 6+2 L2-only dual-cluster: cpus 0-5 L2#0 + cpus 6-7 L2#1)
    const uint32_t cap_8e[8] = {792, 792, 792, 792, 792, 792, 1024, 1024};
    plat_cpu_topo_classify(cap_8e, NULL, 8, 0xFFull, &t);
    assert(t.valid && t.dual_6plus2);
    assert(t.little_mask == 0x3Full && t.n_little == 6);
    assert(t.big_mask == 0xC0ull && t.n_big == 2);
    assert(t.prime_mask == 0xC0ull && t.n_prime == 2);
    assert(t.worker_mask == 0x03ull && t.n_worker == 2);
    assert(t.guest_hot_mask == 0x3Cull && t.n_guest_hot == 4);
    assert(plat_cpu_topo_mask_for_role(&t, 0, PLAT_CPU_ROLE_RENDER) == 0);
    assert(plat_cpu_topo_mask_for_role(&t, 1, PLAT_CPU_ROLE_RENDER) == 0xC0ull);
    assert(plat_cpu_topo_mask_for_role(&t, 1, PLAT_CPU_ROLE_PRESENT) == 0x3Cull);
    assert(plat_cpu_topo_mask_for_role(&t, 1, PLAT_CPU_ROLE_GUEST_HOT) == 0x3Cull);
    assert(plat_cpu_topo_mask_for_role(&t, 1, PLAT_CPU_ROLE_WORKER) == 0x03ull);
    assert(plat_cpu_topo_mask_for_role(&t, 2, PLAT_CPU_ROLE_PRESENT) == 0x3Cull);
    assert(plat_cpu_topo_mask_for_role(&t, 2, PLAT_CPU_ROLE_GUEST_HOT) == 0x3Cull);
    assert(plat_cpu_topo_mask_for_role(&t, 3, PLAT_CPU_ROLE_RENDER) == 0xC0ull);
    assert(plat_cpu_topo_mask_for_role(&t, 3, PLAT_CPU_ROLE_PRESENT) == 0);
    assert(plat_cpu_topo_mask_for_role(&t, 3, PLAT_CPU_ROLE_GUEST_HOT) == 0);
    assert(plat_cpu_topo_mask_for_role(&t, 3, PLAT_CPU_ROLE_WORKER) == 0x03ull);

    // Symmetric 8-core -> valid=false
    const long khz_sym[8] = {2000000, 2000000, 2000000, 2000000, 2000000, 2000000, 2000000, 2000000};
    plat_cpu_topo_classify(NULL, khz_sym, 8, 0xFFull, &t);
    assert(!t.valid);

    // Offlined prime cpu7 -> allowed 0x7F leaves 3 big cores < 4 -> valid=false
    const uint32_t cap_d9400[8] = {502, 502, 502, 472, 880, 880, 880, 1024};
    plat_cpu_topo_classify(cap_d9400, NULL, 8, 0x7Full, &t);
    assert(!t.valid);

    // Unreadable/empty sysfs -> valid=false
    plat_cpu_topo_classify(NULL, NULL, 8, 0xFFull, &t);
    assert(!t.valid);
}

#ifdef __ANDROID__
typedef struct {
    uint64_t worker_mask;
    uint64_t readback_mask;
} ChildArg;

static void *worker_child_fn(void *p) {
    ChildArg *a = (ChildArg *)p;
    cpu_set_t set;
    CPU_ZERO(&set);
    for (int c = 0; c < 64 && c < CPU_SETSIZE; c++) {
        if (a->worker_mask & (1ull << c)) CPU_SET(c, &set);
    }
    assert(sched_setaffinity(0, sizeof set, &set) == 0);
    CPU_ZERO(&set);
    assert(sched_getaffinity(0, sizeof set, &set) == 0);
    uint64_t got = 0;
    for (int c = 0; c < 64 && c < CPU_SETSIZE; c++) {
        if (CPU_ISSET(c, &set)) got |= 1ull << c;
    }
    a->readback_mask = got;
    return NULL;
}

static void test_live_android_sysfs_and_child_isolation(void) {
    cpu_set_t proc_set;
    CPU_ZERO(&proc_set);
    assert(sched_getaffinity(getpid(), sizeof proc_set, &proc_set) == 0);
    uint64_t allowed = 0;
    for (int c = 0; c < 64 && c < CPU_SETSIZE; c++) {
        if (CPU_ISSET(c, &proc_set)) allowed |= 1ull << c;
    }

    uint32_t cap[64] = {0};
    long khz[64] = {0};
    int max_c = 0;
    for (int c = 0; c < 64; c++) {
        char path[96];
        snprintf(path, sizeof path, "/sys/devices/system/cpu/cpu%d/cpu_capacity", c);
        FILE *fc = fopen(path, "re");
        if (fc) {
            unsigned v = 0;
            if (fscanf(fc, "%u", &v) == 1) cap[c] = v;
            fclose(fc);
            if (c + 1 > max_c) max_c = c + 1;
        }
        snprintf(path, sizeof path, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", c);
        FILE *ff = fopen(path, "re");
        if (ff) {
            long v = 0;
            if (fscanf(ff, "%ld", &v) == 1) khz[c] = v;
            fclose(ff);
            if (c + 1 > max_c) max_c = c + 1;
        }
    }

    PlatCpuTopo t;
    plat_cpu_topo_classify(cap, khz, max_c, allowed, &t);
    printf("[live] valid=%d dual6p2=%d src=%s allowed=0x%llx little=0x%llx big=0x%llx mid=0x%llx prime=0x%llx worker=0x%llx hot=0x%llx\n",
           t.valid, t.dual_6plus2, t.used_capacity ? "cap" : "khz",
           (unsigned long long)t.allowed_mask, (unsigned long long)t.little_mask,
           (unsigned long long)t.big_mask, (unsigned long long)t.mid_big_mask,
           (unsigned long long)t.prime_mask, (unsigned long long)t.worker_mask,
           (unsigned long long)t.guest_hot_mask);
    assert(t.valid);
    if (t.dual_6plus2) {
        assert(t.little_mask == 0x3Full);
        assert(t.big_mask == 0xC0ull);
        assert(t.worker_mask == 0x03ull);
        assert(t.guest_hot_mask == 0x3Cull);
    } else {
        assert(t.little_mask == 0x0Full);
        assert(t.big_mask == 0xF0ull);
        assert(t.mid_big_mask == 0x70ull);
        assert(t.worker_mask == 0x0Full);
    }

    // Pin parent to big/prime cluster, spawn child worker that pins to worker cluster,
    // verify child does not remain on parent's cluster and parent keeps its cluster mask.
    uint64_t render_mask = plat_cpu_topo_mask_for_role(&t, 1, PLAT_CPU_ROLE_RENDER);
    uint64_t worker_mask = plat_cpu_topo_mask_for_role(&t, 1, PLAT_CPU_ROLE_WORKER);
    assert(render_mask != 0 && worker_mask != 0 && (render_mask & worker_mask) == 0);
    cpu_set_t big_set;
    CPU_ZERO(&big_set);
    for (int c = 0; c < 64 && c < CPU_SETSIZE; c++) {
        if (render_mask & (1ull << c)) CPU_SET(c, &big_set);
    }
    assert(sched_setaffinity(0, sizeof big_set, &big_set) == 0);

    ChildArg arg = {.worker_mask = worker_mask};
    pthread_t th;
    assert(pthread_create(&th, NULL, worker_child_fn, &arg) == 0);
    pthread_join(th, NULL);
    assert(arg.readback_mask == worker_mask);

    cpu_set_t after_set;
    CPU_ZERO(&after_set);
    assert(sched_getaffinity(0, sizeof after_set, &after_set) == 0);
    uint64_t after = 0;
    for (int c = 0; c < 64 && c < CPU_SETSIZE; c++) {
        if (CPU_ISSET(c, &after_set)) after |= 1ull << c;
    }
    assert(after == render_mask);
}
#endif

int main(void) {
    test_dimensity_9400_plus_capacity();
    test_max_freq_fallback();
    test_sd8_gen3_and_invalid_topologies();
#ifdef __ANDROID__
    test_live_android_sysfs_and_child_isolation();
#endif
    printf("PASS test_mali_cpu_affinity (all assertions)\n");
    return 0;
}
