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

struct FrictionProperties {

    double _coefficientOfSlidingFriction = 0.07000000000000000;
    double _coefficientOfRollingFriction = 0.00800000000000000;
    double _coefficientOfSpinningFriction = 0.02200000000000000;

    double _timeOfequilibriumFactor = 0.0008789064581287676;

    double _restitutionFactor = 0.92;
    double _rollingDragCoefficient = 0.24;
    double _lowSpeedFriction = 0.02;
    double _velocityMultiplier = 1.005;
    double _velocityThreshold = 0.0005;

    double _velocityReductionSlidingFactor = 160.00000000000000;
    double _velocityReductionRollingFactor = 8.5000000000000000;
    double _deltaSpinFactor = 9.8000000000000007;
};
