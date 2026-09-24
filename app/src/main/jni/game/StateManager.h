/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include "Types.h"
#include "Foundation.h"
#include "State.h"

struct StateManager : Class {
    Field<0x8, PNSArray<State>*> mStateStack;

    StateManager(ptr instance = 0) : Class(instance), mStateStack(instance) {}

    int32_t getCurrentStateId() {
        if (!instance) return -1;

        ptr arrPtr = F(ptr, instance + 0x8);
        if (!arrPtr) return -1;

        uintptr_t count = F(uintptr_t, arrPtr + 0x8);
        uintptr_t dataPtr = F(uintptr_t, arrPtr + 0x18);
        if (count == 0 || count > 256 || !dataPtr) return -1;

        uintptr_t lastStatePtr = F(uintptr_t, dataPtr + (count - 1) * 8);
        if (!lastStatePtr) return -1;

        return F(int32_t, lastStatePtr + 0x18);
    }

    operator bool() { return instance && isInstanceOf("StateManager"); }
};

struct MainStateManager : StateManager {
    MainStateManager(ptr instance = 0) : StateManager(instance) {}

    bool isInMenu() { return getCurrentStateId() == 3; }
    bool isInGame() { return getCurrentStateId() == 4; }
};

struct GameStateManager : StateManager {
    GameStateManager(ptr instance = 0) : StateManager(instance) {}

    bool isPlayerTurn() { return getCurrentStateId() == 4; }
};
