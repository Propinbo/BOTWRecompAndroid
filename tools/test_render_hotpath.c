// Unit tests for Vulkan descriptor-set hotpath helpers (descriptor plan, rscale dedupe, soft-linear mask).
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan_core.h>

#include "nvn/vulkan/render_hotpath.h"

static void ref_fill_rscale(float rs[RSCALE_ROWS][4], const float *row0, float mirror_w, int ntex, const float (*sc)[2]) {
    memset(rs, 0, sizeof(float[RSCALE_ROWS][4]));
    rs[0][0] = rs[0][1] = 1.0f;
    if (row0) memcpy(rs[0], row0, 16);
    rs[65][0] = mirror_w;
    if (sc) {
        for (int t = 0; t < ntex && t < 64; t++) {
            rs[1 + t][0] = sc[t][0];
            rs[1 + t][1] = sc[t][1];
        }
    }
}

static void test_descriptor_plan(void) {
    assert(vk_binding_is_ubo(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
    assert(vk_binding_is_ubo(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC));
    assert(!vk_binding_is_ubo(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
    assert(!vk_binding_is_ubo(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER));
    assert(!vk_binding_is_ubo(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE));

    const VkBindingInfo binds[] = {
        {0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},   // 0: set 0 reg (drv dynamic UBO)
        {0, 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},           // 1: set 0 reg (static game UBO)
        {0, 4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},   // 2: set 0 reg (dynamic game UBO)
        {0, 16, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},          // 3: set 0 reg (< 17)
        {0, 17, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},          // 4: set 0 reg (GMEM SSBO)
        {0, 21, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},          // 5: set 0 reg (GFast SSBO)
        {0, 18, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},  // 6: set 0 def (rscale dynamic UBO!)
        {0, 23, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},  // 7: set 0 def (CBPATCH dynamic UBO!)
        {0, 20, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},          // 8: set 0 def (GMAP UBO)
        {0, 32, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},  // 9: set 0 reg (tex0)
        {0, 31, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},  // 10: ignored (< 32)
        {1, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},   // 11: set 1 reg
        {1, 18, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},  // 12: set 1 def (rscale)
        {1, 19, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},          // 13: set 1 def (tint)
        {1, 33, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},  // 14: set 1 reg (tex1)
        {1, 22, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},          // 15: set 1 reg (GMiss SSBO)
        {1, 34, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT},        // 16: set 1 reg (local read >= 32)
        {1, 31, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT},        // 17: ignored (< 32)
    };
    DescriptorPlan p0, p1;
    vk_build_descriptor_plan(binds, (int)(sizeof binds / sizeof binds[0]), 0, &p0);
    vk_build_descriptor_plan(binds, (int)(sizeof binds / sizeof binds[0]), 1, &p1);

    assert(p0.nregular == 7);
    assert(p0.regular[0] == 0 && p0.regular[1] == 1 && p0.regular[2] == 2 && p0.regular[3] == 3 &&
           p0.regular[4] == 4 && p0.regular[5] == 5 && p0.regular[6] == 9);
    assert(p0.ndeferred == 3);
    assert(p0.deferred[0] == 6 && p0.deferred[1] == 7 && p0.deferred[2] == 8);

    assert(p1.nregular == 4);
    assert(p1.regular[0] == 11 && p1.regular[1] == 14 && p1.regular[2] == 15 && p1.regular[3] == 16);
    assert(p1.ndeferred == 2);
    assert(p1.deferred[0] == 12 && p1.deferred[1] == 13);

    // Bounds & NULL guards.
    vk_build_descriptor_plan(binds, 1, 0, NULL);
    vk_build_descriptor_plan(NULL, 10, 0, &p0);
    assert(p0.nregular == 0 && p0.ndeferred == 0);
    vk_build_descriptor_plan(binds, -5, 0, &p0);
    assert(p0.nregular == 0 && p0.ndeferred == 0);

    VkBindingInfo big[MAX_BIND + 8];
    for (int i = 0; i < MAX_BIND + 8; i++) big[i] = (VkBindingInfo){0, (uint32_t)(32 + (i & 31)), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};
    vk_build_descriptor_plan(big, MAX_BIND + 8, 0, &p0);
    assert(p0.nregular == MAX_BIND && p0.ndeferred == 0);

    // Deferred overflow guard & binding 17 exclusion.
    for (int i = 0; i < MAX_BIND + 8; i++) big[i] = (VkBindingInfo){0, CBPATCH_BUFFER_INDEX, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC};
    vk_build_descriptor_plan(big, MAX_BIND + 8, 0, &p0);
    assert(p0.nregular == 0 && p0.ndeferred == MAX_BIND);

    const VkBindingInfo b17[] = {
        {0, 17, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
        {0, 17, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},
    };
    vk_build_descriptor_plan(b17, 2, 0, &p0);
    assert(p0.nregular == 0 && p0.ndeferred == 0);
}

static void test_soft_linear_cand_mask(void) {
    assert(vk_soft_linear_cand_mask(NULL) == 0);
    ShaderInfo si;
    memset(&si, 0, sizeof si);
    assert(vk_soft_linear_cand_mask(&si) == 0);
    si.ntex = -1;
    assert(vk_soft_linear_cand_mask(&si) == 0);

    si.ntex = 8;
    si.tex[0] = (ShaderTexture){.sampled = true, .shadow = false, .is_image = false, .type = TEXTYPE_2D};
    si.tex[1] = (ShaderTexture){.sampled = true, .shadow = true, .is_image = false, .type = TEXTYPE_2D};        // excluded: shadow
    si.tex[2] = (ShaderTexture){.sampled = true, .shadow = false, .is_image = true, .type = TEXTYPE_2D};        // excluded: image
    si.tex[3] = (ShaderTexture){.sampled = false, .shadow = false, .is_image = false, .type = TEXTYPE_2D};      // excluded: not sampled
    si.tex[4] = (ShaderTexture){.sampled = true, .shadow = false, .is_image = false, .type = TEXTYPE_3D};       // excluded: 3D
    si.tex[5] = (ShaderTexture){.sampled = true, .shadow = false, .is_image = false, .type = TEXTYPE_2D_ARRAY};
    si.tex[6] = (ShaderTexture){.sampled = true, .shadow = false, .is_image = false, .type = TEXTYPE_1D};       // excluded: 1D
    si.tex[7] = (ShaderTexture){.sampled = true, .shadow = false, .is_image = false, .type = TEXTYPE_CUBE};     // excluded: cube
    assert(vk_soft_linear_cand_mask(&si) == ((1ULL << 0) | (1ULL << 5)));

    // 64-slot limit and bit-63 reachability without shift UB when ntex > 64.
    memset(&si, 0, sizeof si);
    si.ntex = 70;
    si.tex[63] = (ShaderTexture){.sampled = true, .type = TEXTYPE_2D};
    assert(vk_soft_linear_cand_mask(&si) == (1ULL << 63));
}

static void test_rscale_dedupe(void) {
    // Byte-for-byte rscale_fill_buffer identity against naive reference.
    float rs_fast[RSCALE_ROWS][4], rs_ref[RSCALE_ROWS][4];
    float sc[64][2];
    for (int i = 0; i < 64; i++) { sc[i][0] = 0.75f + 0.01f * (float)i; sc[i][1] = 0.5f + 0.02f * (float)i; }
    const float row0[4] = {1.5f, 1.5f, 16.0f, 8.0f};

    rscale_fill_buffer(rs_fast, NULL, 0.0f, 0, NULL);
    ref_fill_rscale(rs_ref, NULL, 0.0f, 0, NULL);
    assert(memcmp(rs_fast, rs_ref, sizeof rs_fast) == 0);

    rscale_fill_buffer(rs_fast, row0, 1280.0f, 70, sc);  // tests ntex > 64 clamp
    ref_fill_rscale(rs_ref, row0, 1280.0f, 70, sc);
    assert(memcmp(rs_fast, rs_ref, sizeof rs_fast) == 0);

    // Cache hit/miss behavior.
    RscaleDedupe d;
    rscale_dedupe_reset(NULL);
    rscale_dedupe_reset(&d);

    RscaleSpan out = {0};
    uint32_t slot = 999;
    // seq == 0 on freshly reset table must miss (used == 0).
    assert(!rscale_dedupe_lookup(&d, 0, 0, NULL, 0.0f, 3, sc, &slot, &out));
    assert(slot < RSCALE_CACHE_SLOTS);

    // Commit and hit with trailing garbage past ntex = 3 (must still hit cleanly).
    RscaleSpan sp1 = {.buf = 0xAAA1, .off = 0x200, .ptr = (void *)(uintptr_t)0x1234};
    rscale_dedupe_commit(&d, 0, slot, 42, NULL, 0.0f, 3, sc, sp1);
    float sc_tail[64][2];
    memcpy(sc_tail, sc, sizeof sc_tail);
    sc_tail[3][0] = 999.0f;
    sc_tail[63][1] = -42.0f;
    assert(rscale_dedupe_lookup(&d, 0, 42, NULL, 0.0f, 3, sc_tail, &slot, &out));
    assert(out.buf == sp1.buf && out.off == sp1.off && out.ptr == sp1.ptr);

    // Stage 1 is isolated from stage 0.
    assert(!rscale_dedupe_lookup(&d, 1, 42, NULL, 0.0f, 3, sc, &slot, &out));

    // Rollover seq misses.
    assert(!rscale_dedupe_lookup(&d, 0, 43, NULL, 0.0f, 3, sc, &slot, &out));

    // Bit-exactness: -0.0f != +0.0f in row0, mirror_w, and sc.
    const float pos_row0[4] = {1.0f, 1.0f, +0.0f, 0.0f};
    const float neg_row0[4] = {1.0f, 1.0f, -0.0f, 0.0f};
    assert(rscale_dedupe_lookup(&d, 0, 42, pos_row0, 0.0f, 3, sc, &slot, &out));  // Explicit {1,1,+0,+0} matches default NULL row0 bit-exact.
    assert(!rscale_dedupe_lookup(&d, 0, 42, neg_row0, 0.0f, 3, sc, &slot, &out));
    assert(!rscale_dedupe_lookup(&d, 0, 42, NULL, -0.0f, 3, sc, &slot, &out));
    sc_tail[1][0] = -0.0f;
    float sc_pos[64][2];
    memcpy(sc_pos, sc, sizeof sc_pos);
    sc_pos[1][0] = +0.0f;
    assert(rscale_dedupe_lookup(&d, 0, 42, NULL, 0.0f, 3, sc_pos, &slot, &out) == false);
    rscale_dedupe_commit(&d, 0, slot, 42, NULL, 0.0f, 3, sc_pos, sp1);
    assert(rscale_dedupe_lookup(&d, 0, 42, NULL, 0.0f, 3, sc_pos, &slot, &out));
    assert(!rscale_dedupe_lookup(&d, 0, 42, NULL, 0.0f, 3, sc_tail, &slot, &out));

    // Bit-exactness on NaN payloads (NAN == NAN only when bit-identical).
    uint32_t nb1 = 0x7fc00001u, nb2 = 0x7fc00002u;
    float nan1 = 0.0f, nan2 = 0.0f;
    memcpy(&nan1, &nb1, sizeof nan1);
    memcpy(&nan2, &nb2, sizeof nan2);
    float nan_row1[4] = {1.0f, 1.0f, nan1, 0.0f}, nan_row2[4] = {1.0f, 1.0f, nan2, 0.0f};
    assert(!rscale_dedupe_lookup(&d, 0, 42, nan_row1, 0.0f, 3, sc_pos, &slot, &out));
    rscale_dedupe_commit(&d, 0, slot, 42, nan_row1, 0.0f, 3, sc_pos, sp1);
    assert(rscale_dedupe_lookup(&d, 0, 42, nan_row1, 0.0f, 3, sc_pos, &slot, &out));
    assert(!rscale_dedupe_lookup(&d, 0, 42, nan_row2, 0.0f, 3, sc_pos, &slot, &out));

    // Any key change (ntex, row0, mirror_w, sc) misses.
    assert(!rscale_dedupe_lookup(&d, 0, 42, NULL, 0.0f, 4, sc_pos, &slot, &out));
    assert(!rscale_dedupe_lookup(&d, 0, 42, row0, 0.0f, 3, sc_pos, &slot, &out));
    assert(!rscale_dedupe_lookup(&d, 0, 42, NULL, 1280.0f, 3, sc_pos, &slot, &out));
    sc_pos[2][1] = 0.999f;
    assert(!rscale_dedupe_lookup(&d, 0, 42, NULL, 0.0f, 3, sc_pos, &slot, &out));

    // Post-population reset invalidates live entries for the same seq.
    rscale_dedupe_reset(&d);
    assert(!rscale_dedupe_lookup(&d, 0, 42, NULL, 0.0f, 3, sc_pos, &slot, &out));

    // Alternating material hits in multi-slot cache (must occupy distinct slots).
    uint32_t s_a = 0, s_b = 0;
    float sc_a[64][2] = {{0.75f, 0.75f}}, sc_b[64][2] = {{0.5f, 0.5f}, {1.0f, 1.0f}};
    rscale_dedupe_lookup(&d, 1, 7, row0, 0.0f, 1, sc_a, &s_a, &out);
    RscaleSpan sp_a = {.buf = 1, .off = 0x100};
    rscale_dedupe_commit(&d, 1, s_a, 7, row0, 0.0f, 1, sc_a, sp_a);
    rscale_dedupe_lookup(&d, 1, 7, row0, 0.0f, 2, sc_b, &s_b, &out);
    RscaleSpan sp_b = {.buf = 2, .off = 0x200};
    rscale_dedupe_commit(&d, 1, s_b, 7, row0, 0.0f, 2, sc_b, sp_b);
    assert(s_a != s_b);
    assert(rscale_dedupe_lookup(&d, 1, 7, row0, 0.0f, 1, sc_a, &slot, &out) && out.buf == 1);
    assert(rscale_dedupe_lookup(&d, 1, 7, row0, 0.0f, 2, sc_b, &slot, &out) && out.buf == 2);

    // Forced same-slot hash collision eviction: committing B into slot(A) evicts A cleanly.
    float sc_col[64][2] = {{0.25f, 0.25f}};
    uint32_t s_col = 999;
    bool found_collision = false;
    for (int probe = 1; probe <= 512; probe++) {
        sc_col[0][0] = (float)probe;
        assert(!rscale_dedupe_lookup(&d, 1, 7, row0, 0.0f, 1, sc_col, &s_col, &out));
        if (s_col == s_a && (sc_col[0][0] != sc_a[0][0])) { found_collision = true; break; }
    }
    assert(found_collision);
    RscaleSpan sp_col = {.buf = 3, .off = 0x300};
    rscale_dedupe_commit(&d, 1, s_col, 7, row0, 0.0f, 1, sc_col, sp_col);
    assert(!rscale_dedupe_lookup(&d, 1, 7, row0, 0.0f, 1, sc_a, &slot, &out));
    assert(rscale_dedupe_lookup(&d, 1, 7, row0, 0.0f, 1, sc_col, &slot, &out) && out.buf == 3);

    // NULL / invalid stage guards.
    assert(!rscale_dedupe_lookup(NULL, 0, 1, NULL, 0.0f, 0, NULL, &slot, &out));
    assert(!rscale_dedupe_lookup(&d, 2, 1, NULL, 0.0f, 0, NULL, &slot, &out));
    assert(!rscale_dedupe_lookup(&d, 0, 1, NULL, 0.0f, 0, NULL, &slot, NULL));
    rscale_dedupe_commit(NULL, 0, 0, 1, NULL, 0.0f, 0, NULL, sp1);
    rscale_dedupe_commit(&d, -1, 0, 1, NULL, 0.0f, 0, NULL, sp1);
    rscale_dedupe_commit(&d, 0, RSCALE_CACHE_SLOTS, 1, NULL, 0.0f, 0, NULL, sp1);
}

int main(void) {
    test_descriptor_plan();
    test_soft_linear_cand_mask();
    test_rscale_dedupe();
    printf("test_render_hotpath: all checks passed\n");
    return 0;
}
