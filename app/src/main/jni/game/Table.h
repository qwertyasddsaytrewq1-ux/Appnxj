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
#include "CarromPiece.h"

extern ptr libmain;

struct BoardProperties : Class {
    FieldImpl<0x68, Vec2d*, false> mPockets;

    BoardProperties(ptr instance = 0) : Class(instance), mPockets(instance) {}

    double getPocketRadius() { return 6.0; }
    double getLength() { return 74.0; }
    double getWidth() { return 74.0; }

    operator bool() { return instance && this->isInstanceOf("CarromBoardProperties"); }
};

#include "FrictionProperties.h"

struct Board : Class {
    Field<0x3b0, BoardProperties> mBoardProperties;
    Field<0x3c0, FrictionProperties> _frictionProperties;
    Field<0x450, PNSArray<CarromPiece>*> mPieces;
    Field<0x588, Vec4d> mBoardCollisionBounds;

    Board(ptr instance = 0)
        : Class(instance),
          mBoardProperties(instance),
          _frictionProperties(instance),
          mPieces(instance),
          mBoardCollisionBounds(instance) {}

    operator bool() { return instance && this->isInstanceOf("Board"); }
};
