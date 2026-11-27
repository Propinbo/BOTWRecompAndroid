// Pure-C sysfs CPU topology classifier for heterogeneous Android big.LITTLE / tri-cluster SoCs.
// Zero Android/Vulkan dependencies so both platform/android.c and unit tests can include it directly.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifndef PLAT_CPU_ROLE_DEFINED
#define PLAT_CPU_ROLE_DEFINED
enum {
    PLAT_CPU_ROLE_ALL = 0,
    PLAT_CPU_ROLE_RENDER = 1,
    PLAT_CPU_ROLE_PRESENT = 2,
    PLAT_CPU_ROLE_GUEST_HOT = 3,
    PLAT_CPU_ROLE_WORKER = 4,
};
#endif

typedef struct {
    uint64_t allowed_mask;
    uint64_t little_mask;    // efficiency tier (e.g. cpus 0-3 A720 on Mali, cpus 0-5 on Oryon 6+2)
    uint64_t big_mask;       // high-capacity cluster (cpus 4-7 on 4+3+1, cpus 6-7 on Oryon 6+2)
    uint64_t mid_big_mask;   // big_mask without prime (cpus 4-6 X4), or guest_hot_mask on 6+2
    uint64_t prime_mask;     // highest tier (cpu 7 on 4+3+1, cpus 6-7 on 6+2)
    uint64_t worker_mask;    // background worker cluster (cpus 0-3 on 4+3+1, cpus 0-1 on Oryon 6+2)
    uint64_t guest_hot_mask; // promoted hot guest cluster (big_mask/mid_big on 4+3+1, cpus 2-5 on 6+2)
    int n_allowed;
    int n_little;
    int n_big;
    int n_mid_big;
    int n_prime;
    int n_worker;
    int n_guest_hot;
    bool dual_6plus2;        // true on L2-only 6+2 heterogeneous SoCs (e.g. Snapdragon 8 Elite / SM8750)
    bool valid;              // true when heterogeneous 4+3+1 / big+little (n_big >= 4) or Dual-L2 6+2
    bool used_capacity;      // true if cpu_capacity was used, false if cpuinfo_max_freq fallback
} PlatCpuTopo;

static inline int plat_cpu_popcnt64(uint64_t m) {
    return __builtin_popcountll(m);
}

static inline void plat_cpu_topo_classify(const uint32_t *cap, const long *max_khz, int ncpu,
                                          uint64_t allowed_mask, PlatCpuTopo *out) {
    if (!out) return;
    *out = (PlatCpuTopo){0};
    out->allowed_mask = allowed_mask;
    if (ncpu <= 0 || !allowed_mask) return;
    if (ncpu > 64) ncpu = 64;

    // Decide whether cpu_capacity gives >= 2 distinct positive scores across allowed CPUs.
    uint64_t first_cap = 0;
    bool cap_distinct = false;
    if (cap) {
        for (int c = 0; c < ncpu; c++) {
            if (!(allowed_mask & (1ull << c)) || cap[c] == 0) continue;
            if (!first_cap) first_cap = cap[c];
            else if (cap[c] != first_cap) { cap_distinct = true; break; }
        }
    }

    typedef struct {
        int cpu;
        uint64_t score;
    } CpuScore;
    CpuScore items[64];
    int count = 0;
    bool used_cap = cap_distinct;

    for (int c = 0; c < ncpu; c++) {
        if (!(allowed_mask & (1ull << c))) continue;
        uint64_t sc = 0;
        if (used_cap && cap && cap[c] > 0) sc = cap[c];
        else if (!used_cap && max_khz && max_khz[c] > 0) sc = (uint64_t)max_khz[c];
        if (sc > 0) items[count++] = (CpuScore){c, sc};
    }
    if (count < 2) return;

    // Sort ascending by score, breaking ties by CPU index.
    for (int i = 1; i < count; i++) {
        CpuScore key = items[i];
        int j = i - 1;
        while (j >= 0 && (items[j].score > key.score ||
                          (items[j].score == key.score && items[j].cpu > key.cpu))) {
            items[j + 1] = items[j];
            j--;
        }
        items[j + 1] = key;
    }

    // Group into tiers using a 10% relative tolerance (keeps [472,502] +6.4% together while
    // separating X4 vs X925 on both cpu_capacity +16.4% and max_khz +13.0%).
    uint64_t tier_masks[64] = {0};
    int ntiers = 0;
    uint64_t tier_min = items[0].score;
    tier_masks[0] = 1ull << items[0].cpu;
    ntiers = 1;
    for (int i = 1; i < count; i++) {
        if (items[i].score * 100ull > tier_min * 110ull) {
            tier_min = items[i].score;
            ntiers++;
        }
        tier_masks[ntiers - 1] |= 1ull << items[i].cpu;
    }

    out->allowed_mask = 0;
    for (int i = 0; i < count; i++) out->allowed_mask |= 1ull << items[i].cpu;
    out->n_allowed = count;
    out->used_capacity = used_cap;
    out->little_mask = tier_masks[0];
    out->n_little = plat_cpu_popcnt64(out->little_mask);
    if (ntiers < 2) return;

    for (int t = 1; t < ntiers; t++) out->big_mask |= tier_masks[t];
    out->prime_mask = tier_masks[ntiers - 1];
    out->n_big = plat_cpu_popcnt64(out->big_mask);
    out->n_prime = plat_cpu_popcnt64(out->prime_mask);

    uint64_t mid = out->big_mask & ~out->prime_mask;
    if (plat_cpu_popcnt64(mid) >= 2) out->mid_big_mask = mid;
    else out->mid_big_mask = out->big_mask;
    out->n_mid_big = plat_cpu_popcnt64(out->mid_big_mask);

    if (out->n_big >= 4 && out->n_little >= 2) {
        out->worker_mask = out->little_mask;
        out->n_worker = out->n_little;
        out->guest_hot_mask = out->big_mask;
        out->n_guest_hot = out->n_big;
        out->dual_6plus2 = false;
        out->valid = true;
        return;
    }

    // Dual-L2 6+2 topology (e.g. SM8750 Oryon without shared L3: cpus 0-5 L2 #0 + cpus 6-7 L2 #1).
    if (ntiers == 2 && out->n_prime == 2 && out->n_little >= 6) {
        uint64_t m = out->little_mask, w = 0;
        int picked = 0;
        for (int c = 0; c < 64 && picked < 2; c++) {
            if (m & (1ull << c)) {
                w |= 1ull << c;
                picked++;
            }
        }
        out->worker_mask = w;
        out->n_worker = plat_cpu_popcnt64(w);
        out->guest_hot_mask = m & ~w;
        out->n_guest_hot = plat_cpu_popcnt64(out->guest_hot_mask);
        out->mid_big_mask = out->guest_hot_mask;
        out->n_mid_big = out->n_guest_hot;
        out->dual_6plus2 = true;
        out->valid = (out->n_worker == 2 && out->n_guest_hot >= 4 && out->n_big == 2);
    }
}

static inline uint64_t plat_cpu_topo_mask_for_role(const PlatCpuTopo *t, int mode, int role) {
    if (!t || mode <= 0 || !t->valid) return 0;
    if (t->dual_6plus2) {
        switch (role) {
        case PLAT_CPU_ROLE_ALL:
            return t->allowed_mask;
        case PLAT_CPU_ROLE_RENDER:
        case PLAT_CPU_ROLE_PRESENT:
            return t->prime_mask;
        case PLAT_CPU_ROLE_GUEST_HOT:
            return mode == 3 ? 0 : t->guest_hot_mask;
        case PLAT_CPU_ROLE_WORKER:
            return t->worker_mask;
        default:
            return 0;
        }
    }
    switch (role) {
    case PLAT_CPU_ROLE_ALL:
        return t->allowed_mask;
    case PLAT_CPU_ROLE_RENDER:
    case PLAT_CPU_ROLE_PRESENT:
        // Always include non-parked X4 cores alongside X925 (never pin solely to 1 prime core).
        return t->big_mask;
    case PLAT_CPU_ROLE_GUEST_HOT:
        if (mode == 3) return 0;
        if (mode == 2 && t->n_mid_big >= 3) return t->mid_big_mask;
        return t->big_mask;
    case PLAT_CPU_ROLE_WORKER:
        return t->little_mask;
    default:
        return 0;
    }
}
