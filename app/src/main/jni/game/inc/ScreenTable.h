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

#ifndef IMGUI_VERSION
struct ImVec2 {
    float x, y;
    ImVec2(float _x, float _y) : x(_x), y(_y) {}
};
#else
#include <imgui.h>
#endif

inline constexpr double REF_WIDTH = 1280.0;
inline constexpr double REF_HEIGHT = 640.0;

inline constexpr double REF_TABLE_LEFT = 433.5;
inline constexpr double REF_TABLE_RIGHT = 846.5;
inline constexpr double REF_TABLE_TOP = 113.5;
inline constexpr double REF_TABLE_BOTTOM = 526.5;

inline constexpr double REF_TABLE_CANVAS_WIDTH = REF_TABLE_RIGHT - REF_TABLE_LEFT;

inline double TABLE_LEFT = 0.0;
inline double TABLE_TOP = 0.0;
inline double TABLE_RIGHT = 0.0;
inline double TABLE_BOTTOM = 0.0;
inline double TABLE_SCALE = 1.0;

inline double g_GameTrOrigin[2] = {0.0, 0.0};
inline double g_GameTrVecX[2]   = {0.0, 0.0};
inline double g_GameTrVecY[2]   = {0.0, 0.0};
inline double g_GameTrScreenH   = 0.0;
inline int    g_GameTrValid = 0;

inline ImVec2 WorldToScreen(Vec2d worldPos) {
    if (g_GameTrValid && g_GameTrScreenH > 0.0) {

        double px = g_GameTrOrigin[0] + g_GameTrVecX[0]*worldPos.x + g_GameTrVecY[0]*worldPos.y;
        double py = g_GameTrScreenH - (g_GameTrOrigin[1] + g_GameTrVecX[1]*worldPos.x + g_GameTrVecY[1]*worldPos.y);
        if (std::isfinite(px) && std::isfinite(py))
            return ImVec2((float)px, (float)py);
    }

    return ImVec2(-9999.f, -9999.f);
}

inline void UpdateScreenTable(double screenWidth, double screenHeight) {

    double heightScale = screenHeight / REF_HEIGHT;

    double scaledRefWidth = heightScale * REF_WIDTH;
    double offsetX = (screenWidth - scaledRefWidth) / 2.0;

    TABLE_LEFT = offsetX + (heightScale * REF_TABLE_LEFT);
    TABLE_RIGHT = offsetX + (heightScale * REF_TABLE_RIGHT);
    TABLE_TOP = heightScale * REF_TABLE_TOP;
    TABLE_BOTTOM = heightScale * REF_TABLE_BOTTOM;

    TABLE_SCALE = (TABLE_RIGHT - TABLE_LEFT) / TABLE_WIDTH;
}
