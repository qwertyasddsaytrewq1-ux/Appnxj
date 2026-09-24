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
#include "CCNode.h"
#include "CarromPiece.h"
#include <cmath>
#include "inc/NumberUtils.h"

struct VisualGuide : Instance {
    Field<0x28, double> mAimAngle;
    Field<0xa0, ptr> mClassification;

    VisualGuide(ptr instance = 0) : Instance(instance), mAimAngle(instance), mClassification(instance) {}
};

struct VisualStriker : CCNode {
    Field<0x3a8, VisualGuide> mVisualGuide;
    Field<0x3b0, double> mPower;

    VisualStriker(ptr instance = 0) : CCNode(instance), mVisualGuide(instance), mPower(instance) {}

    double getShotAngle() {
        auto angle = mVisualGuide().mAimAngle();

        return angle;
    }

    double getShotPower(bool strict = false) {
        auto power = mPower();
        if (strict && power <= 0.0) return 0.0;

        if (power <= 0.0 || power > 1.0) power = 1.f;
        else power = NumberUtils::normalizeDoublePrecision(power);

        auto maxPower = STRIKER_PROPERTIES_MAX_POWER;

        return (1.0 - sqrt(1.0 - power)) * maxPower;
    }

    operator bool() { return instance && this->isInstanceOf("VisualStriker"); }
};
