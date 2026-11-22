// Android Bionic regression test for nvn/cmdbuf.c copy_lists bulk copy path.
// Build with -ffunction-sections -fdata-sections -Wl,--gc-sections.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../runtime/nvn/cmdbuf.c"

// ---- external runtime stubs required by cmdbuf.c
uint64_t g_guest_lo = 0;
int g_game_version = 190;

void rt_fatal_message(const char *fmt, ...) {
    (void)fmt;
    abort();
}
void vk_ubo_inline_mark(uint64_t addr, uint64_t size) { (void)addr, (void)size; }
int nvn_watch_texid(void) { return -1; }

static bool g_cb_add_ctrl_on_callback = false;
static uint64_t g_cb_next_ctrl_addr = 0;
static uint64_t g_cb_next_ctrl_size = 0;
static int g_callback_calls = 0;

void guest_call(Cpu *c, uint64_t addr) {
    (void)addr;
    g_callback_calls++;
    if (g_cb_add_ctrl_on_callback && g_cb_next_ctrl_addr) {
        uint64_t ctrl = g_cb_next_ctrl_addr;
        uint64_t sz = g_cb_next_ctrl_size;
        g_cb_next_ctrl_addr = 0;
        NvnCmdBuf *cb = cb_of(c->x[0]);
        set_chunk(cb, ctrl, 0, sz);
    }
}

typedef struct {
    uint32_t op, size;
    uint64_t x[7];
    float f[4];
    uint8_t extra[256];
} WalkedRec;

static int walk_list(uint64_t handle, WalkedRec *out, int max_out, int *jumps_out) {
    int n = 0, jumps = 0;
    uint64_t p = handle;
    for (int guard = 0; guard < 100000; guard++) {
        const CmdRec *r = (const CmdRec *)(uintptr_t)p;
        if (r->op == CMD_JUMP) {
            jumps++;
            p = r->x[0];
            continue;
        }
        assert(n < max_out);
        WalkedRec *w = &out[n++];
        memset(w, 0, sizeof *w);
        w->op = r->op;
        w->size = r->size;
        memcpy(w->x, r->x, sizeof w->x);
        memcpy(w->f, r->f, sizeof w->f);
        if (r->size > sizeof(CmdRec)) {
            uint32_t ex = r->size - (uint32_t)sizeof(CmdRec);
            assert(ex <= sizeof w->extra);
            memcpy(w->extra, r->extra, ex);
        }
        if (r->op == CMD_END) break;
        p += r->size;
    }
    if (jumps_out) *jumps_out = jumps;
    return n;
}

typedef struct {
    uint64_t end_used;
    uint64_t host_chunks;
    bool has_gmem;
    int jumps;
    int nrecs;
    WalkedRec recs[2048];
} Capture;

static void build_source_lists(uint64_t *handles, uint8_t **backing_out) {
    // 3 diverse source lists with CMD_BARRIER headers, mixed extra sizes, floats, args, and a CMD_JUMP.
    size_t stride = 128u << 10;
    uint8_t *buf = calloc(4, stride);
    assert(buf != NULL);
    *backing_out = buf;

    // List 0: 120 mixed records (~16 KB)
    uint8_t *p = buf;
    CmdRec *b0 = raw_rec((uint64_t)(uintptr_t)p, CMD_BARRIER, sizeof(CmdRec));
    b0->x[0] = 0x1111;
    p += sizeof(CmdRec);
    for (int i = 0; i < 120; i++) {
        uint32_t ex = (uint32_t)((i % 5) * 16);
        uint32_t rsz = (uint32_t)(sizeof(CmdRec) + ex);
        CmdRec *r = raw_rec((uint64_t)(uintptr_t)p, CMD_DRAW_ELEMENTS + (uint32_t)(i % 3), rsz);
        for (int k = 0; k < 7; k++) r->x[k] = ((uint64_t)i << 16) ^ (uint64_t)k;
        for (int k = 0; k < 4; k++) r->f[k] = (float)i * 0.25f + (float)k;
        for (uint32_t j = 0; j < ex; j++) r->extra[j] = (uint8_t)(i ^ j);
        p += rsz;
    }
    raw_rec((uint64_t)(uintptr_t)p, CMD_END, sizeof(CmdRec));
    handles[0] = (uint64_t)(uintptr_t)buf;

    // List 1: chained via CMD_JUMP inside source
    uint8_t *l1a = buf + stride;
    uint8_t *l1b = buf + 2 * stride;
    raw_rec((uint64_t)(uintptr_t)l1a, CMD_BARRIER, sizeof(CmdRec));
    CmdRec *m1 = raw_rec((uint64_t)(uintptr_t)(l1a + sizeof(CmdRec)), CMD_DRAW_ARRAYS, sizeof(CmdRec) + 8);
    m1->x[0] = 42;
    m1->extra[0] = 0xAB;
    CmdRec *jmp = raw_rec((uint64_t)(uintptr_t)(l1a + sizeof(CmdRec) * 2 + 8), CMD_JUMP, sizeof(CmdRec));
    jmp->x[0] = (uint64_t)(uintptr_t)l1b;
    CmdRec *m2 = raw_rec((uint64_t)(uintptr_t)l1b, CMD_SET_BLEND_COLOR, sizeof(CmdRec) + 16);
    m2->f[0] = 1.0f;
    m2->f[3] = 0.5f;
    for (int j = 0; j < 16; j++) m2->extra[j] = (uint8_t)(0xF0 + j);
    raw_rec((uint64_t)(uintptr_t)(l1b + sizeof(CmdRec) + 16), CMD_END, sizeof(CmdRec));
    handles[1] = (uint64_t)(uintptr_t)l1a;

    // List 2: 400 medium records (~48 KB in host shadow = 6 KB game bytes) to cross SHADOW_PAGE boundaries.
    uint8_t *l2 = buf + 3 * stride;
    p = l2;
    raw_rec((uint64_t)(uintptr_t)p, CMD_BARRIER, sizeof(CmdRec));
    p += sizeof(CmdRec);
    for (int i = 0; i < 400; i++) {
        uint32_t ex = (uint32_t)((i % 4) * 8);
        uint32_t rsz = (uint32_t)(sizeof(CmdRec) + ex);
        CmdRec *r = raw_rec((uint64_t)(uintptr_t)p, CMD_BIND_UNIFORM_BUFFERS, rsz);
        r->x[0] = (uint64_t)(i & 1);
        r->x[1] = (uint64_t)(i & 15);
        r->x[2] = 0xCAFE0000ull + (uint64_t)i;
        for (uint32_t j = 0; j < ex; j++) r->extra[j] = (uint8_t)((i * 7 + j) & 0xFF);
        p += rsz;
    }
    raw_rec((uint64_t)(uintptr_t)p, CMD_END, sizeof(CmdRec));
    handles[2] = (uint64_t)(uintptr_t)l2;
}

static void run_capture_child(int scenario, bool no_bulk, const char *fifo_path) {
    if (no_bulk) setenv("LARECOMP_NO_CMDBUF_BULK", "1", 1);
    else unsetenv("LARECOMP_NO_CMDBUF_BULK");

    g_cmd_host_chunks = 0;
    g_callback_calls = 0;
    uint64_t handles[3] = {0};
    uint8_t *src_buf = NULL;
    build_source_lists(handles, &src_buf);

    uint64_t guest_cb = 0;
    NvnCmdBuf *cb = calloc(1, sizeof *cb);
    assert(cb != NULL);
    cb->guest = (uint64_t)(uintptr_t)&guest_cb;
    SET_HOST(cb->guest, cb);

    Cpu cpu;
    memset(&cpu, 0, sizeof cpu);

    if (scenario == 0) {
        // Multi-list copy inside a spacious single control chunk
        set_chunk(cb, 0x80010000ull, 0, 0x20000);
        cb->has_ctrl = true;
    } else if (scenario == 1) {
        // Starts right near a 64 KB SHADOW_PAGE boundary so copy_lists crosses SHADOW_PAGE twice
        set_chunk(cb, 0x8002FF00ull, 0, 0x30000);
        cb->has_ctrl = true;
    } else if (scenario == 2) {
        // Small initial chunk + callback adding a 2nd control chunk, then exhausted again -> add_host_memory
        set_chunk(cb, 0x80060000ull, 0, 0x800);
        cb->has_ctrl = true;
        cb->callback = 0x1234;
        cb->callback_data = 0x5678;
        g_cb_add_ctrl_on_callback = true;
        g_cb_next_ctrl_addr = 0x80070000ull;
        g_cb_next_ctrl_size = 0x1000;
    }

    cb->recording = true;
    cb->start = (uint64_t)(uintptr_t)rec(&cpu, cb, CMD_BARRIER, 0);

    copy_lists(&cpu, cb, 3, handles);
    rec(&cpu, cb, CMD_END, 0);
    cb->recording = false;

    Capture *cap = calloc(1, sizeof *cap);
    assert(cap != NULL);
    cap->end_used = cb->used;
    cap->host_chunks = (uint64_t)g_cmd_host_chunks;
    cap->has_gmem = cb->gmem != 0;
    cap->nrecs = walk_list(cb->start, cap->recs, 2048, &cap->jumps);

    FILE *f = fopen(fifo_path, "wb");
    assert(f != NULL);
    assert(fwrite(cap, sizeof *cap, 1, f) == 1);
    fclose(f);
    free(cap);
    free(src_buf);
}

static void capture_in_fork(int scenario, bool no_bulk, Capture *out) {
    char tmpl[256];
    snprintf(tmpl, sizeof tmpl, "/data/local/tmp/botwrecompandroid-tests/cmdbuf_%d_%d.bin", (int)getpid(), scenario * 2 + (no_bulk ? 1 : 0));
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        alarm(15);
        run_capture_child(scenario, no_bulk, tmpl);
        _exit(0);
    }
    int status = 0;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    FILE *f = fopen(tmpl, "rb");
    assert(f != NULL);
    assert(fread(out, sizeof *out, 1, f) == 1);
    fclose(f);
    unlink(tmpl);
}

static void verify_parity(int scenario, const char *label) {
    Capture *bulk = calloc(1, sizeof *bulk);
    Capture *ref = calloc(1, sizeof *ref);
    assert(bulk && ref);
    capture_in_fork(scenario, false, bulk);
    capture_in_fork(scenario, true, ref);

    assert(bulk->nrecs == ref->nrecs);
    assert(bulk->nrecs > 520);
    assert(bulk->end_used == ref->end_used);
    assert(bulk->jumps == ref->jumps);
    assert(bulk->host_chunks == ref->host_chunks);
    assert(bulk->has_gmem == ref->has_gmem);
    if (scenario == 1) assert(bulk->jumps >= 1);
    if (scenario == 2) assert(bulk->host_chunks >= 1 && bulk->jumps >= 2);
    for (int i = 0; i < bulk->nrecs; i++) {
        assert(memcmp(&bulk->recs[i], &ref->recs[i], sizeof(WalkedRec)) == 0);
    }
    printf("  ok: %s (%d recs, %d jumps, end_used=%llu)\n", label, bulk->nrecs, bulk->jumps, (unsigned long long)bulk->end_used);
    free(bulk);
    free(ref);
}

int main(void) {
    printf("test_cmdbuf_copy:\n");
    verify_parity(0, "multi-list bulk copy vs LARECOMP_NO_CMDBUF_BULK=1");
    verify_parity(1, "shadow-page boundary crossing and CMD_JUMP parity");
    verify_parity(2, "chunk exhaustion callback + add_host_memory fallback parity");
    puts("PASS: cmdbuf copy_lists bulk copy");
    return 0;
}
