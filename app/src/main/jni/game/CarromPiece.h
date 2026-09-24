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
#include <Vector/Vectors.h>

struct CarromPieceEnums {

    enum Type : int {
        ANY = -1,
        WHITE_PUCK = 0,
        BLACK_PUCK = 1,
        QUEEN = 2,
        STRIKER = 3,
        ERR_TYPE = -8
    };

    enum State : int {
        DEFAULT = 1,
        IN_POCKET = 2,
        UNKNOWN = 3,
        POTTED = 4,
        ERR_STATE = -8
    };
};

struct CarromPiece : CarromPieceEnums, Class {
    Field<0x50, Vec2d> position;
    Field<0x60, Vec2d> velocity;
    Field<0x98, double> radius;
    Field<0x90, double> mass;
    Field<0xa0, Type> pieceType;
    Field<0xa4, State> state;

    CarromPiece(ptr instance = 0) : Class(instance),
        position(instance), velocity(instance), radius(instance),
        mass(instance), pieceType(instance), state(instance) {}

    bool isOnTable() {
        auto s = this->state();
        return s == State::DEFAULT || s == State::IN_POCKET;
    }

    operator bool() { return instance && this->isInstanceOf("CarromPiece"); }
};
