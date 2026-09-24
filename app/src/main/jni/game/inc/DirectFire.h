/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include <cstdint>
#include <cmath>
#include <cstring>
#include <csignal>
#include <setjmp.h>
#include "include/bug_log.h"
#include "include/random_defs.h"

namespace DirectFire {

inline constexpr uintptr_t OFF_BUSY       = 0x1A9B658;
inline constexpr uintptr_t OFF_SLOT_GET  = 0x1A9CAA8;
inline constexpr uintptr_t OFF_OBJ_VALID = 0x20ED8C4;
inline constexpr uintptr_t OFF_FIRE1      = 0x1A9283C;
inline constexpr uintptr_t OFF_FIRE2      = 0x1A92924;
inline constexpr uintptr_t OFF_RES_DBL    = 0x18698C0;
inline constexpr uintptr_t OFF_RES_INT    = 0x190DC74;
inline constexpr uintptr_t OFF_AIMCHANGE  = 0x1A968D0;
inline constexpr uintptr_t OFF_RANGES     = 0x1A96914;
inline constexpr uintptr_t OFF_APPLY_DIR  = 0x1A9B38C;

inline constexpr uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325ULL;
inline constexpr uint64_t FNV_PRIME         = 0x100000001b3ULL;
inline constexpr double   POWER_MIN         = 0.0005;
inline constexpr uint64_t FRESH_TICKS        = 300;

struct ShotRequest {
    uintptr_t gameObj;
    uintptr_t shotCtx;
    uintptr_t ctxObj;
    uintptr_t aimObj;
    uint64_t  expectSeed;
    uint64_t  tick;
    int32_t   index;
    int32_t   pad34;
    double    dirX;
    double    dirY;
    double    dirZ;
    double    power;
};
static_assert(sizeof(ShotRequest) == 0x58, "ShotRequest must mirror .so 0x58 layout");

struct FireState {
    volatile uint64_t tick = 1;
    ShotRequest req = {};
    volatile int32_t pending = 0;
    volatile int32_t result = 0;

    uintptr_t gameObj = 0;
    uintptr_t shotCtx = 0;
    uintptr_t ctxObj = 0;
    uintptr_t aimObj = 0;
    uintptr_t transform = 0;
};
inline FireState g_df = {};

inline sigjmp_buf* g_dfActiveJmp = nullptr;
inline void dfSegvHandler(int) { if (g_dfActiveJmp) siglongjmp(*g_dfActiveJmp, 1); }

template <typename Fn>
inline int dfGuarded(Fn fn) {
    static sigjmp_buf jmp;
    static struct sigaction oldSa;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = dfSegvHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, &oldSa) != 0) return 2;
    g_dfActiveJmp = &jmp;
    volatile int rc = 1;
    if (sigsetjmp(jmp, 1) == 0) {
        if (!fn()) rc = 2;
    } else {
        rc = 0;
    }
    g_dfActiveJmp = nullptr;
    sigaction(SIGSEGV, &oldSa, nullptr);
    return (int)rc;
}

struct DfRanges { uintptr_t t[6]; };
struct DfWalker {
    uintptr_t t1[3];
    uintptr_t t2[3];
    uintptr_t aimChangeFn;
};
static_assert(sizeof(DfWalker) == 0x38, "walker must match .so 0x136818 layout");
struct DfDbl2 { double a, b; };

inline uint64_t fnvByte(uint64_t seed, uint64_t b) {
    seed ^= (b & 0xff);
    seed *= FNV_PRIME;
    return seed;
}

inline bool TriplesValid(const uintptr_t* t) {
    if (!t[0] && !t[1] && !t[2]) return true;
    return t[0] && t[1] && t[2] && t[0] <= t[1] && t[1] <= t[2];
}

inline bool ValidatedInvoke(uint64_t* seed, uintptr_t objA, uintptr_t vecPtr,
                             uintptr_t fnAddr, uint64_t maxCount,
                             uint64_t opcode, uint64_t state) {
    uintptr_t begin = F(uintptr_t, vecPtr);
    uintptr_t end   = F(uintptr_t, vecPtr + 8);
    uintptr_t cap   = F(uintptr_t, vecPtr + 0x10);
    if (!begin || !end || !cap) return false;
    if (begin > end || end > cap) return false;
    if (!fnAddr) return false;
    uint64_t byteSize = end - begin;
    if (byteSize % 0x60 != 0) return false;
    uint64_t count = byteSize / 0x60;
    if (count > maxCount) return false;

    *seed = fnvByte(*seed, opcode);
    for (int b = 0; b < 8; ++b) *seed = fnvByte(*seed, count >> (8 * b));
    if (count == 0) return true;
    for (uint64_t i = 0; i < count; ++i) {
        int32_t id = F(int32_t, begin + i * 0x60);
        if (id < 0) return false;
        for (int b = 0; b < 4; ++b) *seed = fnvByte(*seed, (uint32_t)id >> (8 * b));
        if (opcode == 1 && (uint64_t)(uint32_t)id == state) continue;

        uintptr_t res = M(uintptr_t, fnAddr, uintptr_t, int)(objA, id);
        if (!res) return false;
        int intRes = M(int, libmain + OFF_RES_INT, uintptr_t)(res);
        DfDbl2 dd  = M(DfDbl2, libmain + OFF_RES_DBL, uintptr_t)(res);
        if (!std::isfinite(dd.a) || !std::isfinite(dd.b)) return false;
        for (int b = 0; b < 4; ++b) *seed = fnvByte(*seed, (uint32_t)intRes >> (8 * b));
        uint64_t da, db;
        memcpy(&da, &dd.a, 8);
        memcpy(&db, &dd.b, 8);
        for (int b = 0; b < 8; ++b) *seed = fnvByte(*seed, da >> (8 * b));
        for (int b = 0; b < 8; ++b) *seed = fnvByte(*seed, db >> (8 * b));
    }
    return true;
}

inline bool ExecuteShotPhase(uintptr_t shotCtx, int32_t index, uint64_t* outSeed) {
    *outSeed = 0;
    if (!shotCtx || !libmain) return false;

    uintptr_t rangesFn = g_pAimChangeVar2 ? g_pAimChangeVar2 : (libmain + OFF_RANGES);
    DfRanges ranges = M(DfRanges, rangesFn, uintptr_t)(shotCtx);

    DfWalker walker;
    walker.t1[0] = ranges.t[0]; walker.t1[1] = ranges.t[1]; walker.t1[2] = ranges.t[2];
    walker.t2[0] = ranges.t[3]; walker.t2[1] = ranges.t[4]; walker.t2[2] = ranges.t[5];
    walker.aimChangeFn = libmain + OFF_AIMCHANGE;
    bool ok = false;
    uint64_t seed = FNV_OFFSET_BASIS;

    if (TriplesValid(walker.t1) && TriplesValid(walker.t2)) {

        uint64_t st = (uint64_t)(uint32_t)index;
        if (ValidatedInvoke(&seed, shotCtx, (uintptr_t)&walker,
                            libmain + OFF_FIRE1, 4, 1, st)

            && ValidatedInvoke(&seed, shotCtx, (uintptr_t)&walker.t2,
                               libmain + OFF_FIRE2, 0x20, 2, st)) {
            ok = (seed != 0);
        }
    }

    if (walker.aimChangeFn && (walker.t1[0] || walker.t2[0])) {
        M(void, walker.aimChangeFn, uintptr_t)((uintptr_t)&walker);
    }
    if (ok) *outSeed = seed;
    return ok;
}

inline uintptr_t dfRuntimeGetter(uintptr_t, uintptr_t) { return g_df.ctxObj; }

inline uintptr_t AimGetter(uintptr_t gameObj, int32_t index) {
    if (!libmain || !gameObj) return 0;

    uintptr_t slot = M(uintptr_t, libmain + OFF_SLOT_GET,
                       uintptr_t, uintptr_t, int)(gameObj, 0, index);
    if (!slot) return 0;

    uintptr_t getter = (uintptr_t)&dfRuntimeGetter;
    uintptr_t r1 = M(uintptr_t, libmain + OFF_OBJ_VALID,
                     uintptr_t, uintptr_t)(slot, getter);
    if (!r1) return 0;

    uintptr_t r2 = M(uintptr_t, r1, uintptr_t, uintptr_t)(slot, getter);
    if (r2 & 1) return 0;

    if (F(int32_t, slot + 8) != index) return 0;
    if (!F(uintptr_t, slot + 0x70)) return 0;
    if (!F(uintptr_t, slot + 0x40)) return 0;
    if (!F(uint8_t, slot + 0xc)) return 0;
    return slot;
}

inline bool ValidateRequest(const ShotRequest& r) {
    if (!r.gameObj || !r.shotCtx || !r.ctxObj || !r.aimObj) return false;
    if (r.index < 0) return false;
    if (!std::isfinite(r.dirX) || !std::isfinite(r.dirY) || !std::isfinite(r.dirZ))
        return false;
    if (!(r.power > POWER_MIN && r.power <= 1.0)) return false;
    return true;
}

inline int32_t RunChain(const ShotRequest& r) {
    if (!ValidateRequest(r)) return 2;
    uintptr_t gm = g_df.gameObj;
    uintptr_t shotCtx = g_df.shotCtx;
    if (!gm || !shotCtx) return 2;
    if (r.gameObj != gm) return 2;
    if (r.shotCtx != shotCtx) return 2;
    if (r.ctxObj != g_df.ctxObj) return 2;
    if (r.aimObj != g_df.aimObj) return 2;
    if (!g_df.transform) return 2;
    uint64_t now = g_df.tick;
    if (now < r.tick || (now - r.tick) > FRESH_TICKS) return 2;

    int g = dfGuarded([&]() -> bool {

        int busy = M(int, libmain + OFF_BUSY, uintptr_t, uintptr_t)(gm, 0);
        if (busy & 1) return false;

        uint64_t outSeed = 0;
        if (!ExecuteShotPhase(shotCtx, r.index, &outSeed)) return false;

        if (!AimGetter(gm, r.index)) return false;

        uintptr_t fireRes = M(uintptr_t, libmain + OFF_FIRE1,
                              uintptr_t, int)(shotCtx, r.index);
        if (!fireRes) return false;

        DfDbl2 dd = M(DfDbl2, libmain + OFF_RES_DBL, uintptr_t)(fireRes);
        if (!std::isfinite(dd.a) || !std::isfinite(dd.b)) return false;
        return true;
    });
    if (g != 1) {
        bug_log_write("DFIRE: chain FAILED (guard=%d idx=%d pw=%.3f)", g, r.index, r.power);
        return 2;
    }

    dfGuarded([&]() -> bool {
        M(void, libmain + OFF_APPLY_DIR, uintptr_t, uintptr_t, int,
          double, double, int)(gm, 0, r.index, r.dirX, r.dirY, 0);
        return true;
    });
    bug_log_write("DFIRE: FIRED idx=%d pw=%.3f dir=(%.3f,%.3f)",
                  r.index, r.power, r.dirX, r.dirY);
    return 1;
}

inline void ProcessTick(uintptr_t gameObj) {

    __atomic_store_n(&g_df.tick,
        __atomic_load_n(&g_df.tick, __ATOMIC_RELAXED) + 1, __ATOMIC_RELAXED);
    g_df.gameObj = gameObj;
    g_df.shotCtx = g_cgs.board;
    g_df.ctxObj  = g_cgs.physicsObj;
    g_df.aimObj  = g_cgs.physicsObj;
    g_df.transform = (g_cgs.dbgTrValid == 1) ? 1 : 0;
    if (__atomic_load_n(&g_df.pending, __ATOMIC_ACQUIRE) != 1) return;
    int32_t res = RunChain(g_df.req);
    g_df.req = ShotRequest{};
    __atomic_store_n(&g_df.result, res, __ATOMIC_RELAXED);
    __atomic_store_n(&g_df.pending, 0, __ATOMIC_RELEASE);
}

inline bool Submit(double angle, double power, int pieceId) {
    if (__atomic_load_n(&g_df.pending, __ATOMIC_ACQUIRE) != 0) return false;
    if (!libmain) return false;
    if (pieceId < 0 || (uint32_t)pieceId > 0x7FFFFFFFu) return false;
    if (!std::isfinite(angle)) return false;

    uintptr_t gm = g_cgs.gameManager;
    if (!gm || !g_cgs.board || !g_cgs.playerTurn || !g_cgs.aimValid) return false;
    uintptr_t ctx = g_cgs.physicsObj;
    if (!ctx) return false;
    if (g_cgs.dbgTrValid != 1) return false;
    double p = power;
    if (!std::isfinite(p)) return false;
    if (p <= POWER_MIN) p = 0.001;
    if (p > 1.0) p = 1.0;
    ShotRequest r{};
    r.gameObj = gm;
    r.shotCtx = g_cgs.board;
    r.ctxObj  = ctx;
    r.aimObj  = ctx;
    r.expectSeed = 0;
    r.tick = __atomic_load_n(&g_df.tick, __ATOMIC_ACQUIRE);
    r.index = pieceId;
    r.dirX = std::cos(angle);
    r.dirY = std::sin(angle);
    r.dirZ = 0.0;
    r.power = p;
    if (!ValidateRequest(r)) return false;
    g_df.req = r;
    __atomic_store_n(&g_df.result, 0, __ATOMIC_RELAXED);
    __atomic_store_n(&g_df.pending, 1, __ATOMIC_RELEASE);
    return true;
}

inline int PollResult() {
    if (__atomic_load_n(&g_df.pending, __ATOMIC_ACQUIRE) != 0) return 0;
    return __atomic_load_n(&g_df.result, __ATOMIC_ACQUIRE);
}

}
