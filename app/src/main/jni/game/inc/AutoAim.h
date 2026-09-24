/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include "Prediction.h"
#include <imgui/imgui.h>

using namespace ImGui;

#ifndef MAX_ANGLE_DEFINED
#define MAX_ANGLE_DEFINED
inline constexpr double maxAngle = 360.0 / (180.0 / M_PI);
#endif

#ifndef NORMALIZE_ANGLE_DEFINED
#define NORMALIZE_ANGLE_DEFINED
inline double normalizeAngle(double angle) {
    double newAngle = angle;
    if (newAngle >= maxAngle) newAngle = fmod(newAngle, maxAngle);
    else if (newAngle < 0) newAngle = maxAngle - fmod(-newAngle, maxAngle);
    return newAngle;
}
#endif

bool ix = true;
namespace AutoAim {
    double lastSetAngle = 0.f;
    bool didSetAngle = false;

    bool shouldAutoAIM() {
        if (!g_cgs.gameManager || !g_cgs.board || !g_cgs.playerTurn) return false;
        if (!g_cgs.aimValid) return false;
        if (!g_cgs.physicsObj) return false;

        return !didSetAngle || didSetAngle;
    }

    void setAimAngle(double angle) {
        lastSetAngle = angle;
        if (!g_cgs.gameManager || !g_cgs.board || !g_cgs.playerTurn) return;

    }

    void AIM(double angleStep = 0.1f) {
        if (!g_cgs.gameManager || !g_cgs.board || !g_cgs.playerTurn) return;
        if (!g_cgs.aimValid) return;
        if (!g_cgs.physicsObj) return;

        auto startingAngle = NumberUtils::normalizeDoublePrecision(
            g_cgs.aimValid ? g_cgs.aimAngle : 0.0);

        gPrediction->determineShotResult(true, startingAngle);
        std::vector<int> startingPottedPieces;
        for (int i = 0; i < gPrediction->guiData.piecesCount; i++) {
            Prediction::PieceState& piece = gPrediction->guiData.pieces[i];
            if (piece.originalOnTable && !piece.onTable) {
                startingPottedPieces.push_back(i);
            }
        }

        auto myclass = (g_cgs.targetPiece >= 0 && g_cgs.targetPiece <= 3)
            ? (CarromPieceEnums::Type)g_cgs.targetPiece
            : (ix ? CarromPieceEnums::Type::WHITE_PUCK : CarromPieceEnums::Type::BLACK_PUCK);

        for (double angle = NumberUtils::normalizeDoublePrecision(normalizeAngle(startingAngle + angleStep)); angle != startingAngle; angle = NumberUtils::normalizeDoublePrecision(normalizeAngle(angle + angleStep))) {
            gPrediction->determineShotResult(true, angle);

            bool onlyQueenLeft = true;
            for (int i = 1; i < gPrediction->guiData.piecesCount; i++) {
                Prediction::PieceState& piece = gPrediction->guiData.pieces[i];
                if (piece.classification == myclass) {
                    if (piece.originalOnTable) onlyQueenLeft = false;
                }
            }

            std::vector<int> currentPottedPieces;
            bool isAngleGood = false;
            for (int i = 1; i < gPrediction->guiData.piecesCount; i++) {
                Prediction::PieceState& piece = gPrediction->guiData.pieces[i];
                if (piece.classification == onlyQueenLeft ? CarromPiece::Type::QUEEN : myclass) {
                    if (piece.originalOnTable && !piece.onTable) {
                        currentPottedPieces.push_back(i);
                        isAngleGood = true;
                    }
                }
            }

            if (isAngleGood && gPrediction->guiData.collision.firstHitPiece && gPrediction->guiData.collision.firstHitPiece->classification != myclass) isAngleGood = false;

            auto& striker = gPrediction->guiData.pieces[0];

            if (isAngleGood && striker.originalOnTable && !striker.onTable) isAngleGood = false;
            if (isAngleGood && !onlyQueenLeft) {
                auto* queenPiece = findQueenPiece();
                if (queenPiece && queenPiece->originalOnTable && !queenPiece->onTable) isAngleGood = false;
            }

            if (!currentPottedPieces.empty() && startingPottedPieces != currentPottedPieces && isAngleGood) {
                setAimAngle(angle);

                break;
            }
        }
    }

    void Draw() {
        ImGuiIO& io = GetIO();
        float padding = 30.0f;
        SetNextWindowPos(ImVec2(io.DisplaySize.x - persistent_int["iAIM_WindowX"], persistent_int["iAIM_WindowY"]), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        SetNextWindowSize(ImVec2(210, 65), ImGuiCond_FirstUseEver);

        if (Begin("AutoAim", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings)) {
            ImVec2 windowSize = GetWindowSize();
            float availableWidth = windowSize.x - GetStyle().WindowPadding.x * 2 - GetStyle().ItemSpacing.x * 2;
            float buttonWidth = availableWidth / 3;
            float availableHeight = windowSize.y - GetStyle().WindowPadding.y * 2;
            float buttonSize = (buttonWidth < availableHeight) ? buttonWidth : availableHeight;

            if (Button(ix ? "W" : "B", ImVec2(buttonSize, buttonSize))) ix = !ix;
            SameLine(); if (Button("<", ImVec2(buttonSize, buttonSize))) AIM(persistent_float["fAIM_AngleStep"]);
            SameLine(); if (Button(">", ImVec2(buttonSize, buttonSize))) AIM(-persistent_float["fAIM_AngleStep"]);
        } End();
    }
};
