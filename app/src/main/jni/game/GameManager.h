/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include "StateManager.h"
#include "VisualCue.h"
#include "Table.h"
#include "VisualEnglishControl.h"

#include "inc/NumberUtils.h"
#include "CarromPiece.h"

struct CarromGameManager : Class {
    Field<0x3e0, uintptr_t> _rules;
    Field<0x3b8, Board> mBoard;
    Field<0x4b8, VisualStriker> mVisualStriker;
    Field<0x4c8, VisualEnglishControl> mVisualEnglishControl;
    Field<0x508, GameStateManager> mStateManager;
    Field<0x5c0, int> mGameMode;

    CarromGameManager(ptr instance = 0)
        : Class(instance),
          _rules(instance),
          mBoard(instance),
          mVisualStriker(instance),
          mVisualEnglishControl(instance),
          mStateManager(instance),
          mGameMode(instance) {}

    Vec2d getShotSpin() {
        VisualEnglishControl visualEnglishControl = this->mVisualEnglishControl;
        if (!visualEnglishControl) return Vec2d(0, 0);

        Vec2d english = visualEnglishControl.mEnglish;

        return english ? english * STRIKER_PROPERTIES_ENGLISH_MULTIPLIER : english;
    }

    CarromPieceEnums::Type getPlayerTargetPiece() {
        auto rules = _rules();
        if (!rules) return CarromPieceEnums::Type::ERR_TYPE;

        auto classification_vector = F(uintptr_t, rules + 0xC8);
        if (!classification_vector) return CarromPieceEnums::Type::ERR_TYPE;
        auto sm = mStateManager();
        if (!sm) return CarromPieceEnums::Type::ERR_TYPE;
        return F(CarromPieceEnums::Type, classification_vector + (sm.isPlayerTurn() ? 0 : 4));
    }

    bool isQueenCoverRequired() {
        auto rules = _rules();
        if (!rules) return false;
        return F(bool, rules + 0x68);
    }

    uint getPottedPocketId() {
        auto rules = _rules();
        if (!rules) return 0;
        return F(uint, rules + 0x118);
    }

    operator bool() { return instance != 0; }
};

static CarromGameManager sharedCarromGameManager;
