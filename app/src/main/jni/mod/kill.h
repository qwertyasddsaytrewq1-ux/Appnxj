
#pragma once

#include "include/obfuscation.h"
#include "include/crash_log.h"
#include "include/hook.h"
#include "include/random_defs.h"

typedef void* (*sel_registerName_t)(const char*);

void __KILL_HOOKS__() {
    LOGF("kill.h: __KILL_HOOKS__ start");

    if (!libmain) {
        LOGF("kill.h: libmain not found, skipping game hooks");
        return;
    }

    struct HookAddr { uintptr_t addr; void** tramp; const char* name; };
    HookAddr hookAddrs[6] = {
        { 0x01A999D0, (void**)&_gameUpdateHook,   "gameUpdate"   },
        { 0x01901AE4, (void**)&_autoMatchHook,    "autoMatch"    },
        { 0x01901CAC, (void**)&_ownershipHook,    "ownership"    },
        { 0x01A021E4, (void**)&_overlayFrameHook, "overlayFrame" },
        { 0x01A935CC, (void**)&_tableUpdateHook,  "tableUpdate"  },
        { 0x01A944F4, (void**)&_shotCaptureHook,  "shotCapture"  },
    };

    auto rollbackAll = [&]() {
        for (auto& h : hookAddrs) {
            if (*h.tramp) { DobbyDestroy((void*)(libmain + h.addr)); *h.tramp = nullptr; }
        }

        g_pShotSearch = 0; g_pAimChange = 0; g_pAimChangeVar2 = 0;
        g_pPhysicsEvent = 0; g_pPhysicsEvent2 = 0; g_pAimVar991 = 0; g_pResolver2 = 0;
        g_pSharedMainManager = 0;
        LOGF("kill.h: ROLLBACK (.so 0x136ab4) — hooks destroyed, table cleared");
    };

    if (DobbyHook((void*)(libmain + 0x01A999D0), (void*)gameUpdateHook, (void**)&_gameUpdateHook) == 0
        && _gameUpdateHook) {
        LOGF("kill.h: gameUpdate hook OK");
    } else {

        if (!_gameUpdateHook) DobbyDestroy((void*)(libmain + 0x01A999D0));
        LOGF("kill.h: gameUpdate hook FAILED — rollback + stop (.so master installer)");
        rollbackAll(); return;
    }

    if (DobbyHook((void*)(libmain + 0x01901AE4), (void*)autoMatchHook, (void**)&_autoMatchHook) == 0
        && _autoMatchHook) {
        LOGF("kill.h: autoMatch hook OK");
        g_cgs.startMatchFn = (void(*)(ptr, int, double, double))_autoMatchHook;
    } else {
        if (!_autoMatchHook) DobbyDestroy((void*)(libmain + 0x01901AE4));
        LOGF("kill.h: autoMatch hook FAILED — rollback + stop");
        rollbackAll(); return;
    }

    if (DobbyHook((void*)(libmain + 0x01901CAC), (void*)ownershipHook, (void**)&_ownershipHook) == 0
        && _ownershipHook) {
        LOGF("kill.h: ownership hook OK");
    } else {
        if (!_ownershipHook) DobbyDestroy((void*)(libmain + 0x01901CAC));
        LOGF("kill.h: ownership hook FAILED — rollback + stop");
        rollbackAll(); return;
    }

    if (DobbyHook((void*)(libmain + 0x01A021E4), (void*)overlayFrameHook, (void**)&_overlayFrameHook) == 0
        && _overlayFrameHook) {
        LOGF("kill.h: overlayFrame hook OK");
    } else {
        if (!_overlayFrameHook) DobbyDestroy((void*)(libmain + 0x01A021E4));
        LOGF("kill.h: overlayFrame hook FAILED — rollback + stop");
        rollbackAll(); return;
    }

    if (DobbyHook((void*)(libmain + 0x01A935CC), (void*)tableUpdateHook, (void**)&_tableUpdateHook) == 0
        && _tableUpdateHook) {
        LOGF("kill.h: tableUpdate hook OK");
    } else {
        if (!_tableUpdateHook) DobbyDestroy((void*)(libmain + 0x01A935CC));
        LOGF("kill.h: tableUpdate hook FAILED — rollback + stop");
        rollbackAll(); return;
    }

    if (DobbyHook((void*)(libmain + 0x01A944F4), (void*)shotCaptureHook, (void**)&_shotCaptureHook) == 0
        && _shotCaptureHook) {
        LOGF("kill.h: shotCapture hook OK");
    } else {
        if (!_shotCaptureHook) DobbyDestroy((void*)(libmain + 0x01A944F4));
        LOGF("kill.h: shotCapture hook FAILED — rollback + stop");
        rollbackAll(); return;
    }

    g_pShotSearch        = libmain + 0x01A9B658;
    g_pAimChange         = libmain + 0x01A968D0;
    g_pAimChangeVar2     = libmain + 0x01A96914;

    g_pPhysicsEvent      = libmain + 0x01A9283C;
    g_pPhysicsEvent2     = libmain + 0x01A92924;
    g_pPhysicsStep       = libmain + 0x018391FC;
    g_pMatchStart        = libmain + 0x0190DC3C;
    g_pMatchLifecycle1   = libmain + 0x0190DC74;
    g_pShotAssistant1    = libmain + 0x01A940E4;
    g_pShotAssistant2    = libmain + 0x01A95688;
    g_pShotAssistant3    = libmain + 0x01A9CAA8;

    g_pAimVar991         = libmain + 0x01A991A8;
    g_pResolver2         = libmain + 0x02C3C108;
    g_pSimTable          = libmain + 0x0183C9E8;
    g_pSimCleanup        = libmain + 0x01845484;
    g_pSharedDirector    = libmain + 0x018698B8;
    g_pSharedMainManager = libmain + 0x018698C0;
    g_pSharedMenuManager = libmain + 0x018698C8;
    LOGF("kill.h: call table stored (17 entries)");

    {
        sel_registerName_t pSelRegisterName = (sel_registerName_t)(libmain + 0x020EE10C);
        if (pSelRegisterName) {
            g_SEL_isRemote  = pSelRegisterName("isRemote");
            g_SEL_tableNode = pSelRegisterName("tableNode");
            LOGF("Game No Ads: sel_registerName stored (isRemote=%p tableNode=%p)",
                 g_SEL_isRemote, g_SEL_tableNode);
        } else {
            LOGF("Game No Ads: sel_registerName addr null, skipping");
        }
    }

    {
        struct BridgeTarget { uintptr_t addr; void* cb; void** tramp; const char* name; };

        BridgeTarget bt[] = {
            { libmain + 0x01d51dd8, (void*)lobbyTierCaptureHook,  (void**)&_lobbyTierCaptureHook,  "tierCapture" },
            { libmain + 0x01d53238, (void*)lobbyStateClearHook2,  (void**)&_lobbyStateClearHook2,  "stateClear2" },
            { libmain + 0x01d52920, (void*)lobbyStateClearHook3,  (void**)&_lobbyStateClearHook3,  "stateClear3" },
            { libmain + 0x01ef0050, (void*)lobbyBlockHook,        (void**)&_lobbyBlockHook,        "lobbyBlock" },
            { libmain + 0x0199409c, (void*)lobbyTierProcessHook, (void**)&_lobbyTierProcessHook, "tierProcess" },
            { libmain + 0x01d59f6c, (void*)lobbyConfirmHook,      (void**)&_lobbyConfirmHook,      "autoMatchConfirm" },
            { libmain + 0x01cd51b34,(void*)lobbyTierProcess2Hook, (void**)&_lobbyTierProcess2Hook,"tierProcess2" },
        };
        int okCount = 0;
        for (auto& t : bt) {
            if (t.addr > 0x1000) {
                int r = DobbyHook((void*)t.addr, t.cb, t.tramp);
                if (r == 0) { okCount++; LOGF("Auto Match bridge: %s OK", t.name); }
                else { LOGF("Auto Match bridge: %s FAILED (r=%d) — skipped", t.name, r); }
            }
        }

        if (okCount == 7) {
            g_bridgeActive1 = true;
            LOGF("Auto Match bridge: 7/7 OK — bridgeActive1 SET (.so 0xffcb4)");
        }
    }

    {

    }

    LOGF("kill.h: done — 6 game hooks + call table + sel_registerName (skin changer REMOVED, .so match)");
}

DEFINE_DECOYS_10 DEFINE_DECOYS_3 DEFINE_DECOYS_25

EXPORT NOINLINE void __KILL__() asm(GET_N(0));
void __KILL__() {
    LOGF("kill.h: __KILL__() early init");
}
