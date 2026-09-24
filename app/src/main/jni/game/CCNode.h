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

struct CGPoint {
    double x;
    double y;
};

struct CGSize {
    double width;
    double height;
};

struct CGRect {
    CGPoint origin;
    CGSize size;
};

struct CCNode : Class {

    Field<0x2c8, CGSize> _contentSize;
    Field<0x2e0, ptr> _parent;

    CCNode(ptr instance = 0) : Class(instance), _contentSize(instance), _parent(instance) {}

    operator bool() { return instance && this->isInstanceOf("CCNode"); }
};
