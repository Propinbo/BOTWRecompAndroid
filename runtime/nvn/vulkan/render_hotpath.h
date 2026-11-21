// Pure helpers for Vulkan descriptor-set hot paths: descriptor plan construction,
// active-prefix rscale UBO deduplication, and soft-linear texture candidate masks.
// Included from runtime/nvn/vulkan/exec.c (with RSCALE_SPAN_TYPE / RscaleSpan = VkSpan)
// and standalone host/Bionic unit tests (tools/test_render_hotpath.c).
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if !defined(VK_VERSION_1_0)
#include <vulkan/vulkan_core.h>
#endif

#include "../maxwell/shader.h"

#ifndef VK_BINDING_INFO_DEFINED
#define VK_BINDING_INFO_DEFINED
typedef struct { uint32_t set, binding; VkDescriptorType type; } VkBindingInfo;
#endif

#ifndef MAX_BIND
#define MAX_BIND 96
#endif

#ifndef RSCALE_SPAN_TYPE
#define RSCALE_SPAN_TYPE
typedef struct {
    uint64_t buf;
    uint64_t off;
    void *ptr;
} RscaleSpan;
#endif

static inline bool vk_binding_is_ubo(VkDescriptorType t) {
    return t == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || t == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
}

// Reflection is immutable once a program is published. Partition its bindings once, rather than scanning
// both stages twice per draw. Only metadata is reused: live buffers, texture views and UBO contents are still resolved.
typedef struct {
    uint8_t regular[MAX_BIND];
    uint8_t deferred[MAX_BIND];
    int nregular;
    int ndeferred;
} DescriptorPlan;

static inline void vk_build_descriptor_plan(const VkBindingInfo *binds, int count, uint32_t set, DescriptorPlan *plan) {
    if (!plan) return;
    plan->nregular = 0;
    plan->ndeferred = 0;
    if (!binds || count <= 0) return;
    if (count > MAX_BIND) count = MAX_BIND;
    for (int i = 0; i < count; i++) {
        const VkBindingInfo *b = &binds[i];
        if (b->set != set) continue;
        if ((b->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && b->binding >= 32u) ||
            (b->type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT && b->binding >= 32u) ||
            (vk_binding_is_ubo(b->type) && b->binding < 17u) ||
            b->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            if (plan->nregular < MAX_BIND) plan->regular[plan->nregular++] = (uint8_t)i;
        } else if (vk_binding_is_ubo(b->type) &&
                   (b->binding == 18u || b->binding == 19u || b->binding == GMAP_BUFFER_INDEX ||
                    b->binding == CBPATCH_BUFFER_INDEX)) {
            if (plan->ndeferred < MAX_BIND) plan->deferred[plan->ndeferred++] = (uint8_t)i;
        }
    }
}

static inline uint64_t vk_soft_linear_cand_mask(const ShaderInfo *si) {
    if (!si || si->ntex <= 0) return 0;
    int n = si->ntex < 64 ? si->ntex : 64;
    uint64_t m = 0;
    for (int i = 0; i < n; i++) {
        const ShaderTexture *t_ = &si->tex[i];
        if (t_->sampled && !t_->shadow && !t_->is_image &&
            (t_->type == TEXTYPE_2D || t_->type == TEXTYPE_2D_ARRAY))
            m |= 1ULL << i;
    }
    return m;
}

enum {
    RSCALE_CACHE_SLOTS = 16u,
    RSCALE_ROWS = 66u,
};

typedef struct {
    uint64_t seq;
    uint32_t row0_bits[4];
    uint32_t mirror_w_bits;
    int ntex;
    float sc[64][2];
    RscaleSpan span;
    uint8_t used;
} RscaleEntry;

typedef struct {
    RscaleEntry stage[2][RSCALE_CACHE_SLOTS];
} RscaleDedupe;

static inline void rscale_dedupe_reset(RscaleDedupe *d) {
    if (d) memset(d, 0, sizeof *d);
}

static inline int rscale_clamp_ntex(int ntex, const float (*sc)[2]) {
    if (!sc || ntex <= 0) return 0;
    return ntex > 64 ? 64 : ntex;
}

static inline void rscale_canonical_row0(const float *row0, uint32_t out_bits[4]) {
    static const float def_row0[4] = {1.0f, 1.0f, 0.0f, 0.0f};
    memcpy(out_bits, row0 ? row0 : def_row0, 16u);
}

static inline uint32_t rscale_key_hash(const uint32_t r0[4], uint32_t mw_bits, int ntex, const float (*sc)[2]) {
    uint64_t h = 0xcbf29ce484222325ull;
    for (int i = 0; i < 4; i++) h = (h ^ r0[i]) * 0x100000001b3ull;
    h = (h ^ mw_bits) * 0x100000001b3ull;
    h = (h ^ (uint64_t)(uint32_t)ntex) * 0x100000001b3ull;
    for (int i = 0; i < ntex; i++) {
        uint32_t xy[2];
        memcpy(xy, sc[i], 8u);
        h = (h ^ xy[0]) * 0x100000001b3ull;
        h = (h ^ xy[1]) * 0x100000001b3ull;
    }
    return (uint32_t)(h ^ (h >> 32));
}

static inline void rscale_fill_buffer(float rs[RSCALE_ROWS][4], const float *row0, float mirror_w,
                                      int ntex, const float (*sc)[2]) {
    if (!rs) return;
    memset(rs, 0, sizeof(float[RSCALE_ROWS][4]));
    if (row0) memcpy(rs[0], row0, 16u);
    else rs[0][0] = rs[0][1] = 1.0f;
    rs[65][0] = mirror_w;
    int n = rscale_clamp_ntex(ntex, sc);
    for (int t = 0; t < n; t++) {
        rs[1 + t][0] = sc[t][0];
        rs[1 + t][1] = sc[t][1];
    }
}

static inline bool rscale_dedupe_lookup(const RscaleDedupe *d, int sg, uint64_t seq, const float *row0,
                                        float mirror_w, int ntex, const float (*sc)[2],
                                        uint32_t *out_slot, RscaleSpan *out_span) {
    if (!d || (sg != 0 && sg != 1) || !out_span) return false;
    uint32_t r0[4];
    uint32_t mw_bits = 0;
    rscale_canonical_row0(row0, r0);
    memcpy(&mw_bits, &mirror_w, sizeof mw_bits);
    int n = rscale_clamp_ntex(ntex, sc);
    uint32_t slot = rscale_key_hash(r0, mw_bits, n, sc) & (RSCALE_CACHE_SLOTS - 1u);
    if (out_slot) *out_slot = slot;
    const RscaleEntry *e = &d->stage[sg][slot];
    if (!e->used || e->seq != seq || e->ntex != n || e->mirror_w_bits != mw_bits) return false;
    if (memcmp(e->row0_bits, r0, sizeof r0) != 0) return false;
    if (n > 0 && memcmp(e->sc, sc, (size_t)n * sizeof sc[0]) != 0) return false;
    *out_span = e->span;
    return true;
}

static inline void rscale_dedupe_commit(RscaleDedupe *d, int sg, uint32_t slot, uint64_t seq, const float *row0,
                                        float mirror_w, int ntex, const float (*sc)[2], RscaleSpan span) {
    if (!d || (sg != 0 && sg != 1) || slot >= RSCALE_CACHE_SLOTS) return;
    RscaleEntry *e = &d->stage[sg][slot];
    rscale_canonical_row0(row0, e->row0_bits);
    memcpy(&e->mirror_w_bits, &mirror_w, sizeof e->mirror_w_bits);
    int n = rscale_clamp_ntex(ntex, sc);
    e->seq = seq;
    e->ntex = n;
    if (n > 0) memcpy(e->sc, sc, (size_t)n * sizeof sc[0]);
    e->span = span;
    e->used = 1;
}
