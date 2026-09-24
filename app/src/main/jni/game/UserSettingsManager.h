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

struct UserSettingsManager : Class {
    Field<0x1c, int> mPowerGaugeLocation;
    Field<0x20, int> mPowerGaugeOrientation;

    UserSettingsManager(ptr instance = 0) : Class(instance), mPowerGaugeLocation(instance), mPowerGaugeOrientation(instance) {}

    operator bool() { return instance && isInstanceOf("UserSettingsManager"); }
};

static UserSettingsManager sharedUserSettingsManager;
