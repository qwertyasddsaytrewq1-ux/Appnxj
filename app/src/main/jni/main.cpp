#include <signal.h>

#include "include/includes.h"
#include "include/hook.h"
#include "include/crash_log.h"

#include <android/log.h>
#include <stdint.h>
#include <cmath>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "include/input.h"
#include "include/java.h"
#include "include/obfuscation.h"
#include "include/manual_dlsym.h"
#include "include/random_defs.h"

#include <cstddef>

char g_espLastReason[160] = "";

static uint64_t now_ms() {
    return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

struct CachedGameState {
    ptr gameManager;
    ptr board;
    int32_t stateId;
    int32_t lastStateId;
    uint64_t lastUpdateMs;
    bool inGame;

    bool playerTurn = false;

    ptr physicsObj = 0;
    
    int32_t lastTierId = -1;      
    ptr lastMatchSelf = 0;         
    double lastMatchParam1 = 0.0;  
    double lastMatchParam2 = 0.0;  
    
    void (*startMatchFn)(ptr, int, double, double) = nullptr;

    static const int MAX_FRAME_PIECES = 32;
    int32_t framePieceIds[32] = {};
    int32_t framePieceStates[32] = {};

    double framePiecePos[32][2] = {};
    bool framePiecePosValid[32] = {};
    
    uintptr_t framePieceObjs[32] = {};
    volatile int soFireIdx = -1;     
    volatile int soFireState = 0;    

    volatile bool pendingShot = false;
    double pendingShotAngle = 0.0;
    double pendingShotPower = 0.0;

    uint64_t dbgEntry0[11] = {};      
    int32_t dbgEntry0Count = -1;      

    int32_t dbgPosIds[20] = {};       
    int32_t dbgPosSts[20] = {};
    double dbgPosX[20] = {};          
    double dbgPosY[20] = {};          
    int32_t dbgPosN = -1;

    uint64_t dbgPhys[16] = {};
    int32_t dbgPhysOk = -1;   
    
    uint64_t dbgAimCbArgs[8] = {};
    double dbgAimCbD[8] = {};     
    int32_t dbgAimCbCount = -1;   
    
    double dbgTrOrigin[2] = {};
    double dbgTrVecX[2] = {};
    double dbgTrVecY[2] = {};
    double dbgTrScale = 0.0;
    int32_t dbgTrValid = 0;
    int32_t dbgPiecesN = -1;
    int32_t dbgStrikerSrc = -2;
    int32_t dbgNormalized = -1;
    int32_t dbgPiecesBuilt = -1;
    double dbgMaxAbs = -1.0;
    int32_t framePieceCount = 0;
    uint64_t framePieceGen = 0;
    bool frameDataValid = false;

    double aimAngle = 0.0;
    double aimPower = 0.0;
    bool aimValid = false;

    ptr rulesPtr = 0;
    bool queenCoverRequired = false;
    uint32_t pottedPocketId = 0;
    int32_t targetPiece = 0;       
    double shotSpinX = 0.0;
    double shotSpinY = 0.0;
    bool rulesValid = false;
};

CachedGameState g_cgs = []{ CachedGameState c{}; c.pottedPocketId = 0xFFFFFFFFu; return c; }();

volatile bool g_autoQueueEnabled = false;

static ptr g_lobbySelf = 0;                   
static ptr g_lobbySelfTime = 0;                
static volatile bool g_bridgeActive1 = false; 
static volatile bool g_bridgeActive2 = false; 

inline uintptr_t g_pShotSearch        = 0;
inline uintptr_t g_pAimChange         = 0;
inline uintptr_t g_pAimChangeVar2     = 0;

inline uintptr_t g_pPhysicsEvent      = 0;
inline uintptr_t g_pPhysicsEvent2     = 0;
inline uintptr_t g_pPhysicsStep       = 0;
inline uintptr_t g_pMatchStart        = 0;
inline uintptr_t g_pMatchLifecycle1   = 0;
inline uintptr_t g_pShotAssistant1   = 0;
inline uintptr_t g_pShotAssistant2   = 0;
inline uintptr_t g_pShotAssistant3   = 0;
inline uintptr_t g_pSimTable         = 0;
inline uintptr_t g_pSimCleanup       = 0;
inline uintptr_t g_pSharedDirector    = 0;
inline uintptr_t g_pSharedMainManager = 0;
inline uintptr_t g_pSharedMenuManager = 0;

inline uintptr_t g_pAimVar991 = 0;      
inline uintptr_t g_pResolver2 = 0;      
#include "menu.h"

struct MatchState {
    uint8_t  pad0[0x28];
    int64_t  counter;    
    int32_t  state;      
    int32_t  pad34;
    double   float1;     
    double   float2;     
    uint8_t  flag;       
    uint8_t  pad49[0x60];
};
static_assert(offsetof(MatchState, counter) == 0x28);
static_assert(offsetof(MatchState, float1)  == 0x38);
static_assert(offsetof(MatchState, float2)  == 0x40);
static_assert(offsetof(MatchState, flag)    == 0x48);

struct PredictionState {
    uint8_t  pad0[0x28];
    int64_t  counter;    
    int32_t  state;      
    int32_t  pad34;
    uint8_t  pad38[0x10];
    int32_t  shotId;     
    uint8_t  pad4c[0x5c];
    uint8_t  dataBuf[0x30]; 
};

struct OwnershipResult { uint8_t data[48]; };

inline volatile int g_hookLock = 0;
inline volatile bool g_hooksPaused = false;

inline void hookLock()   { __atomic_fetch_add(&g_hookLock, 1, __ATOMIC_ACQ_REL); }
inline void hookUnlock() { __atomic_fetch_add(&g_hookLock, -1, __ATOMIC_ACQ_REL); }

inline MatchState       g_matchStateData    = {};
inline MatchState*      g_matchState        = &g_matchStateData;
inline PredictionState  g_predictionStateData = {};
inline PredictionState* g_predictionState   = &g_predictionStateData;

inline bool g_managersInited = false;

static void* g_SEL_isRemote  = nullptr;
static void* g_SEL_tableNode = nullptr;
#define KILL_H_SEL_GLOBALS_DEFINED 1

#define LOG_MATCH_TYPE "Match type tierId=%d mode=%s"
#define LOG_OWNERSHIP_CAPTURED "Player puck ownership captured"

inline void initManagersFromSelf(ptr self) {
    if (g_managersInited || !self || !libmain) return;
    g_managersInited = true;
    if (!sharedCarromGameManager.instance) sharedCarromGameManager.instance = self;
    if (!sharedMenuManager.instance)        sharedMenuManager.instance     = self;
}

static void dispatchMatchEvent(bool matchStart) {
    if (!g_predictionState) return;
    uintptr_t ps = (uintptr_t)g_predictionState;
    *(int32_t*)(ps + 0x30) = 3;     
    *(int32_t*)(ps + 0x48) = -1;    
    memset((void*)(ps + 0x38), 0, 0x10);
    memset((void*)(ps + 0x50), 0, 0x30);
}

static volatile int32_t g_aimCbCount = -1;
static volatile uint64_t g_aimCbArgs[8] = {};
static volatile double g_aimCbD[8] = {};   

typedef uintptr_t (*AimCbFull)(double, double, double, double, double, double,
                               double, double,
                               uintptr_t, uintptr_t, uintptr_t, uintptr_t,
                               uintptr_t, uintptr_t, uintptr_t, uintptr_t);
static AimCbFull g_realAimChangeFn = nullptr;
static uintptr_t aimChangeThunk(double d0v, double d1v, double d2v, double d3v,
                                double d4v, double d5v, double d6v, double d7v,
                                uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3,
                                uintptr_t a4, uintptr_t a5, uintptr_t a6, uintptr_t a7) {
    g_aimCbCount++;
    g_aimCbArgs[0] = a0; g_aimCbArgs[1] = a1;
    g_aimCbArgs[2] = a2; g_aimCbArgs[3] = a3;
    g_aimCbArgs[4] = a4; g_aimCbArgs[5] = a5;
    g_aimCbArgs[6] = a6; g_aimCbArgs[7] = a7;
    g_aimCbD[0] = d0v; g_aimCbD[1] = d1v;
    g_aimCbD[2] = d2v; g_aimCbD[3] = d3v;
    g_aimCbD[4] = d4v; g_aimCbD[5] = d5v;
    g_aimCbD[6] = d6v; g_aimCbD[7] = d7v;
    if (g_realAimChangeFn)
        return g_realAimChangeFn(d0v, d1v, d2v, d3v, d4v, d5v, d6v, d7v,
                                 a0, a1, a2, a3, a4, a5, a6, a7);
    return 0;
}

DEFINES(void, gameUpdateHook, ptr self, ptr arg1, double dt) {
    hookLock();
    if (_gameUpdateHook) _gameUpdateHook(self, arg1, dt);
    if (!g_hooksPaused && self) {
        static bool s_loggedOnce = false;
        if (!s_loggedOnce) {
            s_loggedOnce = true;
            bug_log_write("gameUpdateHook: FIRST FIRE self=0x%lx", (unsigned long)self);
        }

        g_cgs.gameManager = self;                    
        g_cgs.board = F(ptr, self + 0x3b8);         
        g_cgs.lastUpdateMs = now_ms();
        g_cgs.inGame = true;

        if (g_cgs.board && g_pShotSearch) {
            bool matchFlag = g_matchState ? (g_matchState->flag != 0) : false;
            int turnResult = M(int, g_pShotSearch, ptr, int)(self, 0);
            bool bit0Clear = !(turnResult & 1);   

            double msD0 = g_matchState ? g_matchState->float1 : 0.0;
            double msD1 = g_matchState ? g_matchState->float2 : 0.0;
            int msState = g_matchState ? g_matchState->state : 0;
            bool autoplayValid = matchFlag && bit0Clear
                && (msState >= 0)
                && std::isfinite(msD0) && std::isfinite(msD1);
            if (autoplayValid) {
                
                g_cgs.playerTurn = true;
                if (g_cgs.stateId != 3) { g_cgs.lastStateId = g_cgs.stateId; g_cgs.stateId = 3; }

                bool autoplayOk = false;
                if (g_pPhysicsEvent) {
                    g_cgs.physicsObj = M(ptr, g_pPhysicsEvent, ptr, int)(g_cgs.board, msState);
                    
                    if (g_cgs.physicsObj && g_pSharedMainManager) {

                        struct AimVec2 { double d0, d1; };
                        AimVec2 aim = M(AimVec2, g_pSharedMainManager, ptr)(g_cgs.physicsObj);
                        
                        if (std::isfinite(aim.d0) && std::isfinite(aim.d1)) {

                            g_cgs.aimAngle = aim.d0;   
                            g_cgs.aimPower = aim.d1;   
                            g_cgs.aimValid = true;
                            autoplayOk = true;

                            g_cgs.dbgTrValid = 0;
                            if (libmain) {
                                struct Dbl2 { double x, y; };  
                                auto trFn = (Dbl2(*)(ptr, double, double))(
                                    libmain + 0x1A991A8);
                                float scl = F(float, libmain + 0x2C3C108);
                                if (scl == scl && scl >= 0.25f && scl <= 8.0f) {
                                    Dbl2 c1 = trFn(g_cgs.gameManager, 0.0, 0.0);
                                    Dbl2 c2 = trFn(g_cgs.gameManager, 1.0, 0.0);
                                    Dbl2 c3 = trFn(g_cgs.gameManager, 0.0, 1.0);
                                    double sc = (double)scl;
                                    double ox = c1.x * sc, oy = c1.y * sc;
                                    double vxx = (c2.x - c1.x) * sc;
                                    double vxy = (c2.y - c1.y) * sc;
                                    double vyx = (c3.x - c1.x) * sc;
                                    double vyy = (c3.y - c1.y) * sc;
                                    bool trOk = std::isfinite(ox) && std::isfinite(oy)
                                              && std::isfinite(vxx) && std::isfinite(vxy)
                                              && std::isfinite(vyx) && std::isfinite(vyy);

                                    if (trOk) {
                                        g_cgs.dbgTrOrigin[0] = ox; g_cgs.dbgTrOrigin[1] = oy;
                                        g_cgs.dbgTrVecX[0] = vxx;  g_cgs.dbgTrVecX[1] = vxy;
                                        g_cgs.dbgTrVecY[0] = vyx;  g_cgs.dbgTrVecY[1] = vyy;
                                        g_cgs.dbgTrScale = sc;
                                        g_cgs.dbgTrValid = 1;

                                        g_GameTrOrigin[0] = ox; g_GameTrOrigin[1] = oy;
                                        g_GameTrVecX[0] = vxx; g_GameTrVecX[1] = vxy;
                                        g_GameTrVecY[0] = vyx; g_GameTrVecY[1] = vyy;
                                        g_GameTrValid = 1;
                                    }
                                }
                            }
                        }
                    }
                }
                if (!autoplayOk) {
                    
                    g_cgs.playerTurn = false;
                    if (g_cgs.stateId != 4) { g_cgs.lastStateId = g_cgs.stateId; g_cgs.stateId = 4; }
                    g_cgs.physicsObj = 0;
                    g_cgs.aimValid = false;
                }

                g_cgs.pendingShot = false;
                if (g_cgs.board && g_pAimChangeVar2 && g_pPhysicsStep && g_pAimChange) {

                        struct AimRanges {
                            uintptr_t t[6];      
                            uintptr_t aimChange; 
                        };
                        auto getRanges = M(AimRanges, g_pAimChangeVar2, ptr);
                        AimRanges ranges = getRanges(g_cgs.board);   

                        g_realAimChangeFn = (AimCbFull)g_pAimChange;
                        ranges.aimChange = (uintptr_t)&aimChangeThunk;
                        auto tripleOK = [](const uintptr_t* t) -> bool {
                            if (!t[0] && !t[1] && !t[2]) return true;       
                            return t[0] && t[1] && t[2] && t[0] <= t[1] && t[1] <= t[2];
                        };
                        if (tripleOK(&ranges.t[0]) && tripleOK(&ranges.t[3])) {
                            uint8_t buf[128] = {};
                            M(void, g_pPhysicsStep, ptr, ptr, ptr)((ptr)buf, g_cgs.board, (ptr)&ranges);
                            bool ok = F(uintptr_t, (uintptr_t)buf) != 0;   

                            for (int k = 0; k < 16; k++)
                                g_cgs.dbgPhys[k] = ((uint64_t*)buf)[k];
                            g_cgs.dbgPhysOk = ok ? 1 : 0;
                            
                            g_cgs.dbgAimCbCount = g_aimCbCount;
                            for (int k = 0; k < 8; k++) {
                                g_cgs.dbgAimCbArgs[k] = g_aimCbArgs[k];
                                g_cgs.dbgAimCbD[k] = g_aimCbD[k];
                            }

                            static uint64_t s_lastPhysLog = 0;
                            static uint64_t s_lastPhysHandle = 0xFFFF;
                            uint64_t nowMs = now_ms();
                            uint64_t handle = ((uint64_t*)buf)[0];
                            if (handle != s_lastPhysHandle ||
                                nowMs - s_lastPhysLog > 1000) {
                                s_lastPhysHandle = handle;
                                s_lastPhysLog = nowMs;
                                bug_log_write("PRED_DUMP ok=%d h=%llx",
                                              (int)ok, (unsigned long long)handle);
                                bug_log_write("PRED_HEX %016llx %016llx %016llx %016llx",
                                    (unsigned long long)((uint64_t*)buf)[0],
                                    (unsigned long long)((uint64_t*)buf)[1],
                                    (unsigned long long)((uint64_t*)buf)[2],
                                    (unsigned long long)((uint64_t*)buf)[3]);
                                bug_log_write("PRED_HEX %016llx %016llx %016llx %016llx",
                                    (unsigned long long)((uint64_t*)buf)[4],
                                    (unsigned long long)((uint64_t*)buf)[5],
                                    (unsigned long long)((uint64_t*)buf)[6],
                                    (unsigned long long)((uint64_t*)buf)[7]);
                                bug_log_write("PRED_HEX %016llx %016llx %016llx %016llx",
                                    (unsigned long long)((uint64_t*)buf)[8],
                                    (unsigned long long)((uint64_t*)buf)[9],
                                    (unsigned long long)((uint64_t*)buf)[10],
                                    (unsigned long long)((uint64_t*)buf)[11]);
                                bug_log_write("PRED_HEX %016llx %016llx %016llx %016llx",
                                    (unsigned long long)((uint64_t*)buf)[12],
                                    (unsigned long long)((uint64_t*)buf)[13],
                                    (unsigned long long)((uint64_t*)buf)[14],
                                    (unsigned long long)((uint64_t*)buf)[15]);
                                bug_log_write("PRED_DBL %.6g %.6g %.6g %.6g %.6g %.6g %.6g %.6g",
                                    ((double*)buf)[0], ((double*)buf)[1],
                                    ((double*)buf)[2], ((double*)buf)[3],
                                    ((double*)buf)[4], ((double*)buf)[5],
                                    ((double*)buf)[6], ((double*)buf)[7]);
                                bug_log_write("PRED_DBL %.6g %.6g %.6g %.6g %.6g %.6g %.6g %.6g",
                                    ((double*)buf)[8], ((double*)buf)[9],
                                    ((double*)buf)[10], ((double*)buf)[11],
                                    ((double*)buf)[12], ((double*)buf)[13],
                                    ((double*)buf)[14], ((double*)buf)[15]);
                                bug_log_write("AIMCBD n=%d d=%.6g %.6g %.6g %.6g %.6g %.6g %.6g %.6g",
                                    (int)g_aimCbCount,
                                    g_aimCbD[0], g_aimCbD[1], g_aimCbD[2], g_aimCbD[3],
                                    g_aimCbD[4], g_aimCbD[5], g_aimCbD[6], g_aimCbD[7]);
                                bug_log_write("AIMCB n=%d a=%llx %llx %llx %llx %llx %llx %llx %llx",
                                    (int)g_aimCbCount,
                                    (unsigned long long)g_aimCbArgs[0],
                                    (unsigned long long)g_aimCbArgs[1],
                                    (unsigned long long)g_aimCbArgs[2],
                                    (unsigned long long)g_aimCbArgs[3],
                                    (unsigned long long)g_aimCbArgs[4],
                                    (unsigned long long)g_aimCbArgs[5],
                                    (unsigned long long)g_aimCbArgs[6],
                                    (unsigned long long)g_aimCbArgs[7]);
                            }
                        } else {
                            g_cgs.dbgPhysOk = 2;   
                            bug_log_write("PHYS_EXEC: ranges INVALID (.so state-4 path)");
                        }
                }
            } else if (matchFlag && bit0Clear) {

                g_cgs.playerTurn = false;
                if (g_cgs.stateId != 4) { g_cgs.lastStateId = g_cgs.stateId; g_cgs.stateId = 4; }
                g_cgs.physicsObj = 0;
                g_cgs.aimValid = false;
            } else {
                
                g_cgs.playerTurn = bit0Clear;
                int newState = bit0Clear ? 3 : 5;
                if (g_cgs.stateId != newState) { g_cgs.lastStateId = g_cgs.stateId; g_cgs.stateId = newState; }
                g_cgs.physicsObj = 0;
                g_cgs.aimValid = false;
            }
        }

        static ptr s_lastSelf = 0, s_lastBoard = 0;
        if (s_lastSelf != self || s_lastBoard != g_cgs.board) {
            s_lastSelf = self;
            s_lastBoard = g_cgs.board;
            
            if (gPrediction) gPrediction->guiData.piecesCount = 0;
        }

        initManagersFromSelf(self);
    }
    hookUnlock();

    if (self) DirectFire::ProcessTick((uintptr_t)self);
}

DEFINES(void, autoMatchHook, ptr self, int tierId, double param1, double param2) {
    hookLock();
    bool hooksActive = !g_hooksPaused;
    if (hooksActive) {
        
        g_cgs.lastTierId = tierId;
        g_cgs.lastMatchSelf = self;
        g_cgs.lastMatchParam1 = param1;
        g_cgs.lastMatchParam2 = param2;
        dispatchMatchEvent(true);
        if (g_matchState) {
            g_matchState->state   = tierId;
            g_matchState->float1  = param1;
            g_matchState->float2  = param2;
            g_matchState->counter++;
            g_matchState->flag    = 1;
        }

        if (g_autoQueueEnabled) {
            AutoPlay::bAutoPlaying = true;
            bug_log_write("AutoQueue: match started (tierId=%d) AutoPlay button ON", tierId);
            bug_log_flush();   
        }
        LOGI(LOG_MATCH_TYPE, tierId, "auto");

        static int laPs = -2; static double laPx = 0.0, laPy = 0.0;
        if (tierId != laPs || param1 != laPx || param2 != laPy) {
            laPs = tierId; laPx = param1; laPy = param2;
            AP_DBG("LA s=%d x=%.3f y=%.3f", tierId, param1, param2);
        }
    }
    if (_autoMatchHook) _autoMatchHook(self, tierId, param1, param2);
    hookUnlock();
}

DEFINES(void, ownershipHook, ptr self, int playerId, int puckIndex, int arg3, double param1, double param2) {
    hookLock();
    bool hooksActive = !g_hooksPaused;
    if (hooksActive) {
        dispatchMatchEvent(false);
        if (g_matchState) {
            g_matchState->state   = playerId;
            g_matchState->float1  = param1;
            g_matchState->float2  = param2;
            g_matchState->counter++;
            g_matchState->flag    = 0;
        }
        LOGI(LOG_OWNERSHIP_CAPTURED);
    }
    
    if (_ownershipHook) _ownershipHook(self, playerId, puckIndex & 1, arg3, param1, param2);

    if (hooksActive && gPrediction) {
        gPrediction->guiData.piecesCount = 0;
        gPrediction->guiData.shotState = false;
    }
    if (hooksActive) {
        g_cgs.physicsObj = 0;
        g_cgs.aimValid = false;   
    }
    hookUnlock();
}

DEFINES(OwnershipResult, overlayFrameHook, ptr self) {
    hookLock();
    OwnershipResult result = {};
    if (_overlayFrameHook) result = _overlayFrameHook(self);

    if (!g_hooksPaused && self) {

        ptr walkerObj = F(ptr, (uintptr_t)self + 0x48);
        if (walkerObj) {
            ptr typeObj = F(ptr, (uintptr_t)walkerObj + 0x298);
            if (typeObj && !F(uint8_t, (uintptr_t)typeObj + 0x1bc)) {
                g_cgs.targetPiece = F(int32_t, (uintptr_t)typeObj + 0x1c0);
            }
        }

        uintptr_t arrBegin = *(uintptr_t*)(result.data + 0x18);
        uintptr_t arrEnd   = *(uintptr_t*)(result.data + 0x20);
        uintptr_t arrCap   = *(uintptr_t*)(result.data + 0x28);

        if (arrBegin && arrBegin <= arrEnd && arrEnd <= arrCap) {
            uintptr_t byteSize = arrEnd - arrBegin;
            
            if (byteSize % 0x60 == 0 && (byteSize - 0x60) <= 0xbff) {
                int count = (int)(byteSize / 0x60);
                if (count > CachedGameState::MAX_FRAME_PIECES)
                    count = CachedGameState::MAX_FRAME_PIECES;

                int valid = 0;
                for (int i = 0; i < count; i++) {
                    uintptr_t entry = arrBegin + (uintptr_t)i * 0x60;
                    int32_t pieceId = *(int32_t*)entry;       
                    int32_t pieceSt = *(int32_t*)(entry + 0x20); 
                    
                    if (pieceId < 0) break;

                    double px = F(double, entry + 0x28);
                    double py = F(double, entry + 0x30);

                    bool posOk = (px == px && py == py)
                              && (px != 0x7FF0000000000000 && py != 0x7FF0000000000000)
                              && (px >= -36.0 && px <= 36.0)
                              && (py >= -36.0 && py <= 36.0);
                    g_cgs.framePieceObjs[valid] = entry;
                    g_cgs.framePieceIds[valid] = pieceId;
                    g_cgs.framePieceStates[valid] = pieceSt;
                    g_cgs.framePiecePos[valid][0] = posOk ? px : 0.0;
                    g_cgs.framePiecePos[valid][1] = posOk ? py : 0.0;
                    g_cgs.framePiecePosValid[valid] = posOk;

                    static bool s_entryDumped = false;
                    if (!s_entryDumped && i == 0 && count > 1) {
                        s_entryDumped = true;
                        bug_log_write("ESP_FRAME_DUMP: n=%d id0=%d st0=%d", count, pieceId, pieceSt);
                        for (int k = 0; k < 11; k++) {
                            int off = 0x08 + k * 8;
                            double d = F(double, entry + off);
                            uint64_t bits = *(uint64_t*)(entry + off);
                            g_cgs.dbgEntry0[k] = bits;          
                            bug_log_write("  +0x%02x: %f (0x%llx)", off, d, (unsigned long long)bits);
                        }
                        g_cgs.dbgEntry0Count = 11;
                    }

                    if (i < 20) {
                        g_cgs.dbgPosIds[i] = pieceId;
                        g_cgs.dbgPosSts[i] = pieceSt;
                        g_cgs.dbgPosX[i] = F(double, entry + 0x28);
                        g_cgs.dbgPosY[i] = F(double, entry + 0x30);
                        g_cgs.dbgPosN = i + 1;      
                    }
                    valid++;
                }
                g_cgs.framePieceCount = valid;
                g_cgs.frameDataValid = (valid > 0);
                g_cgs.framePieceGen++;

                if (valid == 0) {

                    if (libmain) {
                        struct Dbl2x { double x, y; };
                        auto fnQuery  = (uintptr_t(*)(uintptr_t, int))(libmain + 0x1A9283C);
                        auto fnType   = (int(*)(uintptr_t))(libmain + 0x190DC74);
                        auto fnFlag   = (int(*)(uintptr_t))(libmain + 0x190DCEC);
                        auto fnId     = (int(*)(uintptr_t))(libmain + 0x18698B8);
                        auto fnPos    = (Dbl2x(*)(uintptr_t))(libmain + 0x18698C0);
                        auto fnRadius = (double(*)(uintptr_t))(libmain + 0x18698C8);
                        static sigjmp_buf pfJmp;
                        static struct sigaction pfOld;
                        struct sigaction pfSa; memset(&pfSa, 0, sizeof(pfSa));
                        pfSa.sa_handler = [](int){ siglongjmp(pfJmp, 1); };
                        sigemptyset(&pfSa.sa_mask);
                        sigaction(SIGSEGV, &pfSa, &pfOld);
                        if (sigsetjmp(pfJmp, 1) == 0) {
                            uintptr_t pobj = fnQuery((uintptr_t)self, 0);
                            if (pobj) {
                                uintptr_t node = F(ptr, pobj + 0x188);
                                int added = 0, hops = 0;
                                double rads[32]; int tvs[32];
                                double maxR = 0.0; int maxRi = -1;
                                int tc[6] = {0,0,0,0,0,0};
                                while (node && hops < 40) {
                                    uintptr_t po = node + 0x30;
                                    int t = fnType(po);
                                    bool keep = (t == 0 || t == 3 || t == 4 || t == 5)
                                             || (t == 1 && fnFlag(po) == 0);
                                    if (keep) {
                                        int pid = fnId(po);
                                        if (pid >= 0 && added < 32) {
                                            Dbl2x pp = fnPos(po);
                                            double pr = fnRadius(po);
                                            bool pOk = pp.x == pp.x && pp.y == pp.y
                                                    && pp.x > -36.0 && pp.x < 36.0
                                                    && pp.y > -36.0 && pp.y < 36.0
                                                    && pr > 0.1 && pr < 10.0;
                                            g_cgs.framePieceObjs[added] = po;
                                            g_cgs.framePieceIds[added] = pid;
                                            g_cgs.framePiecePos[added][0] = pOk ? pp.x : 0.0;
                                            g_cgs.framePiecePos[added][1] = pOk ? pp.y : 0.0;
                                            g_cgs.framePiecePosValid[added] = pOk;
                                            rads[added] = pOk ? pr : 0.0;
                                            tvs[added] = t;
                                            if (t >= 0 && t <= 5) tc[t]++;
                                            if (rads[added] > maxR) {
                                                maxR = rads[added]; maxRi = added;
                                            }
                                            added++;
                                        }
                                    }
                                    node = F(ptr, node);
                                    hops++;
                                }

                                for (int k = 0; k < added; ++k) {
                                    int st = (k == maxRi) ? 0
                                           : (tvs[k] == 4) ? 2 : 1;
                                    g_cgs.framePieceStates[k] = st;
                                }
                                valid = added;
                                g_cgs.framePieceCount = valid;
                                g_cgs.frameDataValid = (valid > 0);
                                g_cgs.framePieceGen++;
                                if (valid == 0) {
                                    AP_DBG("WALK+SO n=0 (list khali)");
                                } else {
                                    char tb[80]; int off = 0;
                                    for (int k = 0; k <= 5; ++k)
                                        if (tc[k]) off += snprintf(tb+off,
                                            sizeof(tb)-off, " t%d=%d", k, tc[k]);
                                    char b2[160];
                                    snprintf(b2, sizeof(b2), "SO n=%d mr=%.2f%s",
                                             valid, maxR, tb);
                                    AP_DBG("%s", b2);
                                }
                            } else {
                                AP_DBG("WALK 0 + SO-QUERY null");
                            }
                        } else {
                            AP_DBG("WALK 0 + SO-QUERY SIGSEGV");
                        }
                        sigaction(SIGSEGV, &pfOld, nullptr);
                    } else {
                        AP_DBG("WALK EMPTY n=0 (libmain?)");
                    }
                }
                
                if (valid > 0) {
                    g_cgs.lastUpdateMs = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()).count();
                    g_cgs.inGame = true;
                }
            }
        }

        if (!g_cgs.board) {

            g_cgs.inGame = g_cgs.frameDataValid;
        }
    }

    hookUnlock();
    return result;
}

DEFINES(int, tableUpdateHook, ptr self) {
    hookLock();
    int result = 0;
    if (_tableUpdateHook) result = _tableUpdateHook(self);
    if (!g_hooksPaused && self) {
        if (!sharedCarromGameManager.instance) {
            sharedCarromGameManager.instance = self;
            initManagersFromSelf(self);
        }
    }
    hookUnlock();
    return result;
}

DEFINES(ptr, shotCaptureHook, ptr self, ptr shotData) {
    hookLock();
    ptr result = 0;
    if (_shotCaptureHook) result = _shotCaptureHook(self, shotData);
    if (!g_hooksPaused && self) {
        if (!sharedCarromGameManager.instance) {
            sharedCarromGameManager.instance = self;
        }
    }
    hookUnlock();
    return result;
}

DEFINES(EGLBoolean, eglDestroyContextHook, EGLDisplay dpy, EGLContext ctx) {
    if (bImguiSetup) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
        bImguiSetup = false;
    }
    if (_eglDestroyContextHook) return _eglDestroyContextHook(dpy, ctx);
    return EGL_FALSE;
}

DEFINES(void, noAdHook1, ptr self, ptr result) {

    if (result) {
        F(uint64_t, result + 0x30) = 0;  
    }
    
}

DEFINES(void, noAdHook2, ptr self, ptr result) {
    
    if (result) {
        F(uint64_t, result + 0x18) = 1;  
    }
    
}

DEFINES(void, lobbyTierCaptureHook, ptr self) {
    if (_lobbyTierCaptureHook) _lobbyTierCaptureHook(self);  
    if (!self) return;
    if (!g_bridgeActive1) return;           
    if (g_bridgeActive2) {                   
        g_lobbySelf = self;                  
        g_lobbySelfTime = (ptr)1;             
    }
}

DEFINES(void, lobbyStateClearHook2, ptr self) {
    if (g_lobbySelf == self) {              
        g_lobbySelf = 0;                     
        g_lobbySelfTime = 0;                 
    }
    if (_lobbyStateClearHook2) _lobbyStateClearHook2(self);  
}

DEFINES(void, lobbyStateClearHook3, ptr self) {
    if (g_lobbySelf == self) {
        g_lobbySelf = 0;
        g_lobbySelfTime = 0;
    }
    if (_lobbyStateClearHook3) _lobbyStateClearHook3(self);  
}

DEFINES(int, lobbyBlockHook, ptr self) {

    if (!g_bridgeActive1 || !g_bridgeActive2) {
        
        if (_lobbyBlockHook) return _lobbyBlockHook(self);
        return 0;
    }
    
    return 0;
}

DEFINES(void, lobbyConfirmHook, ptr self, ptr a1, ptr a2, ptr a3, ptr a4) {

    if (g_bridgeActive1 && g_bridgeActive2 && self) {
        F(uint8_t, (uintptr_t)self + 0x53c) = 1;   
    }
    if (_lobbyConfirmHook) _lobbyConfirmHook(self, a1, a2, a3, a4);   
}

DEFINES(void, lobbyTierProcess2Hook, ptr self, ptr a1, ptr a2, ptr a3, ptr a4) {
    
    if (_lobbyTierProcess2Hook) _lobbyTierProcess2Hook(self, a1, a2, a3, a4);   
}

DEFINES(ptr, lobbyTierProcessHook, ptr self, ptr arg1, ptr arg2) {
    
    if (g_autoQueueEnabled && arg1) {

    }
    if (_lobbyTierProcessHook) return _lobbyTierProcessHook(self, arg1, arg2);
    return (ptr)0;
}

#include "mod/kill.h"

#include <link.h>

static int dl_callback(struct dl_phdr_info *info, size_t size, void *data) {
    const char* name = info->dlpi_name;
    if (!name || !name[0]) return 0;
    if (strstr(name, "libgame-CARROM") || strstr(name, "libgame-carrom")) {
        uintptr_t* base_ptr = (uintptr_t*)data;
        *base_ptr = (uintptr_t)info->dlpi_addr;
        LOGI("dl_iterate_phdr: FOUND %s at base 0x%lx", name, (unsigned long)*base_ptr);
        return 1;
    }
    return 0;
}

uintptr_t findLibGameBase() {
    uintptr_t base = 0;
    dl_iterate_phdr(dl_callback, &base);
    if (base) return base;

    static const char* search_patterns[] = {
        "libgame-CARROM", "libgame-carrom", "libgame.so", "libgame-", nullptr
    };
    std::ifstream maps("/proc/self/maps");
    std::string map;
    while (std::getline(maps, map)) {
        if (map.find("r-xp") != std::string::npos) {
            for (int i = 0; search_patterns[i]; i++) {
                if (map.find(search_patterns[i]) != std::string::npos) {
                    std::istringstream line(map);
                    std::string start;
                    std::getline(line, start, '-');
                    base = htol(start);
                    if (base) return base;
                }
            }
        }
    }
    return 0;
}

void __HOOK_EGL__() {

    HOOKS("libEGL.so", "eglSwapBuffers", Draw);
    HOOKS("libEGL.so", "eglDestroyContext", eglDestroyContextHook);
}

void __1__() {
    LOGI("__1__: START");

    PACKAGE_NAME = string(getcmdline());
    if (PACKAGE_NAME.empty()) PACKAGE_NAME = "com.miniclip.carrom";

    __IMGUI__();
    __HOOK_EGL__();

    if (!libmain) {
        for (int i = 0; i < 240; i++) {
            libmain = findLibGameBase();
            if (libmain) break;
            sleepm(500);
        }
    }

    if (libmain) {
        set_libgame_base(libmain);
        LOGI("__1__: libgame=0x%lx, installing hooks", (unsigned long)libmain);
        __KILL_HOOKS__();
        LOGI("__1__: hooks installed");
    } else {
        LOGI("__1__: libgame NOT FOUND");
    }

    __INPUT__();
    LOGI("__1__: DONE");
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    bug_log_init();
    LOGF("MOD VERSION: V33-V13 — Bug.txt at: %s", bug_log_get_path());                
    crash_log_init();
    VM = vm;
    install_crash_handlers();      
    setup_global_segv_handler();   

    pthread(__1__);
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_miniclip_carrom_CarromActivity_nativeOverlayTouch(JNIEnv* env, jclass thiz, jint action, jfloat x, jfloat y) {
    if (bImguiSetup) { ImGui_FeedTouchToIO(action, x, y); return JNI_FALSE; }
    return JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_miniclip_carrom_CarromActivity_nativeDumperTouch(JNIEnv* env, jclass thiz, jint action, jfloat x, jfloat y) {
    if (bImguiSetup) { ImGui_FeedTouchToIO(action, x, y); return JNI_FALSE; }
    return JNI_FALSE;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved) {}
