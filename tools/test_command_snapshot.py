"""Compile the production command-copy helper with a source-recycling callback.
Usage: python tools/test_command_snapshot.py <NDK llvm bin directory>
Run android/out/test_command_snapshot through adb.
"""
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
source = (root / 'runtime/nvn/cmdbuf.c').read_text()
start = source.index('typedef struct {\n    uint8_t *buf;')
helper = source[start:source.index('// CallCommands too:', start)]
prefix = r'''
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
typedef struct { uint64_t x[32]; } Cpu;
typedef struct { int unused; } NvnCmdBuf;
typedef struct { uint32_t op, size; uint64_t x[7]; float f[4]; uint8_t extra[]; } CmdRec;
enum { CMD_END, CMD_JUMP, CMD_CALL, CMD_BARRIER, CMD_TEST, CMD_COUNT };
#define FATAL(...) abort()
#define LOG(...) ((void)0)
#define ENV(...) 0
static void hist_report(uint64_t h) { (void)h; }
static CmdRec first[22], continuation[6], mid_cont[125], later[12], reent_list[4], output[192];
static union { CmdRec rec; uint8_t raw[0x100000]; } big_out;
static unsigned recorded, large_rec_count;
static size_t large_rec_bytes;
static uint64_t reent_handle;
static CmdRec *rec(Cpu *c, NvnCmdBuf *cb, uint32_t op, uint32_t extra);
static void copy_snapshot_to_cb(Cpu *c, NvnCmdBuf *cb, const uint8_t *snapshot, size_t used);
'''
suffix = r'''
static CmdRec *rec(Cpu *c, NvnCmdBuf *cb, uint32_t op, uint32_t extra) {
    (void)c; (void)cb; assert(op == CMD_TEST);
    if (extra > 0) {
        assert(sizeof(CmdRec) + extra <= sizeof big_out.raw);
        large_rec_count++;
        large_rec_bytes += sizeof(CmdRec) + extra;
        return &big_out.rec;
    }
    if (!recorded) {
        // Allocation callback reuses both this handle and a later handle's source memory.
        memset(first, 0xa5, sizeof first);
        memset(continuation, 0xa5, sizeof continuation);
        memset(later, 0xa5, sizeof later);
    }
    assert(recorded < 192);
    CmdRec *slot = &output[recorded++];
    if (reent_handle) {
        uint64_t h = reent_handle;
        reent_handle = 0;
        copy_lists(c, cb, 1, &h);
    }
    return slot;
}
static void copy_snapshot_to_cb(Cpu *c, NvnCmdBuf *cb, const uint8_t *snapshot, size_t used) {
    for (size_t offset = 0; offset < used;) {
        const CmdRec *r = (const CmdRec *)(snapshot + offset);
        uint32_t size = r->size;
        CmdRec *d = rec(c, cb, r->op, size - (uint32_t)sizeof(CmdRec));
        memcpy(d, r, size);
        offset += size;
    }
}
static void item(CmdRec *r, unsigned value) {
    *r = (CmdRec){.op=CMD_TEST, .size=sizeof(CmdRec)}; r->x[0]=value;
}
static void init_lists(void) {
    first[0] = (CmdRec){.op=CMD_BARRIER, .size=sizeof(CmdRec)};
    for (unsigned i=0;i<20;i++) item(&first[i+1],i);
    first[21] = (CmdRec){.op=CMD_JUMP, .size=sizeof(CmdRec)};
    first[21].x[0]=(uintptr_t)continuation;
    for (unsigned i=0;i<5;i++) item(&continuation[i],20+i);
    continuation[5].op=CMD_END;
    later[0] = (CmdRec){.op=CMD_BARRIER, .size=sizeof(CmdRec)};
    for (unsigned i=0;i<10;i++) item(&later[i+1],100+i);
    later[11].op=CMD_END;
}
static void verify_output(unsigned expect_n, unsigned off) {
    assert(recorded == off + expect_n);
    for (unsigned i=0;i<25;i++) {
        assert(output[off+i].op==CMD_TEST && output[off+i].size==sizeof(CmdRec));
        assert(output[off+i].x[0]==i);
    }
    for (unsigned i=25;i<expect_n;i++) {
        assert(output[off+i].op==CMD_TEST && output[off+i].size==sizeof(CmdRec));
        assert(output[off+i].x[0]==(100 + i - 25));
    }
}
static void *worker_fn(void *arg) {
    (void)arg;
    Cpu c = {0}; NvnCmdBuf cb = {0};
    uint64_t handles[]={(uintptr_t)first,(uintptr_t)later};
    copy_lists(&c,&cb,2,handles);
    return NULL;
}
static void run_all_cases(void) {
    Cpu c = {0}; NvnCmdBuf cb = {0};
    uint64_t handles[]={(uintptr_t)first,(uintptr_t)later};
    recorded = 0;
    init_lists();
    copy_lists(&c,&cb,2,handles);
    verify_output(35, 0);
    // Second call on same thread reuses CopyScratch heap buffer (~2800 B <= 4096).
    recorded = 0;
    init_lists();
    copy_lists(&c,&cb,2,handles);
    verify_output(35, 0);
    // Larger call (~10.5 KB > 4096) triggers realloc(sc->buf, next) on existing buffer.
    recorded = 0;
    init_lists();
    first[21].x[0] = (uintptr_t)mid_cont;
    for (unsigned i = 0; i < 120; i++) item(&mid_cont[i], 20 + i);
    mid_cont[120].op = CMD_END;
    copy_lists(&c, &cb, 2, handles);
    assert(recorded == 150);
    for (unsigned i = 0; i < 140; i++) assert(output[i].x[0] == i);
    for (unsigned i = 140; i < 150; i++) assert(output[i].x[0] == 100 + (i - 140));
    // Re-entrant copy_lists while sc->busy == true must use stack fallback without corrupting outer snapshot.
    recorded = 0;
    init_lists();
    for (unsigned i = 0; i < 2; i++) item(&reent_list[i], 500 + i);
    reent_list[2].op = CMD_END;
    reent_handle = (uintptr_t)reent_list;
    copy_lists(&c, &cb, 2, handles);
    assert(reent_handle == 0);
    assert(recorded == 37);
    assert(output[0].x[0] == 0);
    assert(output[1].x[0] == 500 && output[2].x[0] == 501);
    for (unsigned i = 1; i < 25; i++) assert(output[2 + i].x[0] == i);
    for (unsigned i = 0; i < 10; i++) assert(output[27 + i].x[0] == 100 + i);
    // Oversized > CMDBUF_SNAP_REUSE_MAX (9 MiB > 8 MiB) must free/reset sc->buf and leave subsequent calls clean.
    size_t n_big = 9, step = 0x100000;
    uint8_t *big_arena = (uint8_t *)calloc(n_big + 1, step);
    assert(big_arena != NULL);
    for (size_t i = 0; i < n_big; i++) {
        CmdRec *r = (CmdRec *)(big_arena + i * step);
        r->op = CMD_TEST;
        r->size = (uint32_t)step;
    }
    ((CmdRec *)(big_arena + n_big * step))->op = CMD_END;
    large_rec_count = 0; large_rec_bytes = 0;
    uint64_t big_h = (uintptr_t)big_arena;
    copy_lists(&c, &cb, 1, &big_h);
    assert(large_rec_count == n_big && large_rec_bytes == n_big * step);
    free(big_arena);
    recorded = 0;
    init_lists();
    copy_lists(&c, &cb, 2, handles);
    verify_output(35, 0);
    // Child thread executes copy_lists and exits cleanly through copy_scratch_dtor.
    recorded = 0;
    init_lists();
    pthread_t th;
    assert(pthread_create(&th, NULL, worker_fn, NULL) == 0);
    assert(pthread_join(th, NULL) == 0);
    verify_output(35, 0);
    copy_lists(&c,&cb,0,NULL); assert(recorded==35);
}
int main(void) {
    run_all_cases();
    if (!getenv("LARECOMP_NO_CMDBUF_SCRATCH")) {
        pid_t pid = fork();
        assert(pid >= 0);
        if (pid == 0) {
            setenv("LARECOMP_NO_CMDBUF_SCRATCH", "1", 1);
            execl("/proc/self/exe", "test_command_snapshot", NULL);
            _exit(127);
        }
        int st = 0;
        assert(waitpid(pid, &st, 0) == pid);
        assert(WIFEXITED(st) && WEXITSTATUS(st) == 0);
    }
    puts("PASS: recycled source, multiple lists, jumps, leading barriers, heap reuse/realloc/trim, reentrancy, thread exit, rollback and empty input");
    return 0;
}
'''
out = root / 'android/out'
out.mkdir(parents=True, exist_ok=True)
cfile = out / 'test_command_snapshot.c'
cfile.write_text(prefix + helper + suffix)
subprocess.run([str(Path(sys.argv[1]) / 'clang.exe'), '--target=aarch64-linux-android30',
                '-O1', str(cfile), '-o', str(out / 'test_command_snapshot')], cwd=root, check=True)
