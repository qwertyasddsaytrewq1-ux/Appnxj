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

struct MenuManager : Class {
    MenuManager(ptr instance = 0) : Class(instance) {}

    int getMenuStateId() {
        return M(int, libmain + 0x019EBFC4, ptr)(instance);
    }

    bool isInQueue() { return getMenuStateId() == 12; }

    operator bool() { return instance != 0; }
};

static MenuManager sharedMenuManager;
