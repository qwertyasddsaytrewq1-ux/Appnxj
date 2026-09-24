/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include <cmath>
#include <cstdio>
#include <cstring>

#include <Vector/Vectors.h>

#include "GameConstants.h"

#define NAN std::isnan

static constexpr double CONST_ZERO           = 0.0;
static constexpr double CONST_PI              = 3.14159265359;
static constexpr double CONST_PI_HALF         = 1.57079632679;
static constexpr double CONST_PI_THREE_HALF   = 4.71238898038;

inline void calcVectorAngle(double *outAngle, const Vector2D *vec) {
    double *pdVar1;
    double dVar2;

    dVar2 = vec->y;
    if (vec->x == CONST_ZERO) {
        pdVar1 = (double *)&CONST_PI_THREE_HALF;
        if (dVar2 < CONST_ZERO == (NAN(dVar2) || NAN(CONST_ZERO))) {
            pdVar1 = (double *)&CONST_PI_HALF;
        }
        dVar2 = *pdVar1;
    }
    else {
        dVar2 = atan(dVar2 / vec->x);

        if (vec->x < CONST_ZERO) {
            dVar2 = dVar2 + CONST_PI;
        }
        if (vec->x < CONST_ZERO) {
            dVar2 = dVar2 + CONST_PI;
        }
    }
    *outAngle = dVar2;
    return;
}

namespace NumberUtils {

    inline double normalizeDoublePrecision(double value, double = 0.0, double = 0.0, size_t = 7) {
        return value;
    }

    inline double calcAngle(const Vec2d& delta) {
        double angle;
        calcVectorAngle(&angle, &delta);
        return angle;
    }

    inline double calcAngle(Vec2d source, Vec2d Destination) {
        return calcAngle(source - Destination);
    }
}

inline double ShotPowerToPower(double shotPower) {

    constexpr double MAX_STRIKER_DRAG = 1.0;
    double clampedPower = std::clamp(shotPower, 0.0, MAX_STRIKER_DRAG);

    double powerRatio = clampedPower * (2.0 - clampedPower);
    return powerRatio;
}
