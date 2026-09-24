/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include <map>
#include <string>
#include "imgui.h"
#include "external/json/json.hpp"
#include "includes.h"

using json = nlohmann::json;

#ifndef CRED
#define CRED ImColor(255, 0, 0, 255)
#endif
#ifndef CGREEN
#define CGREEN ImColor(0, 255, 0, 255)
#endif
#ifndef CWHITE
#define CWHITE ImColor(255, 255, 255, 255)
#endif
#ifndef CYELLOW
#define CYELLOW ImColor(255, 255, 0, 255)
#endif

inline std::map<std::string, bool> features_bool = {
    {"bAIM", true},
    {"bAIM_AutoAngle", true},
    {"bAIM_AutoPower", false},
    {"bAIM_IgnorePocketed", true},
    {"bAIM_DrawFov", true},
    {"bAIM_DirectShotsOnly", false},
    {"bAIM_CushionBankShots", true},
    {"bESP", true},
    {"bESP_Line", true},
    {"bESP_PredictionLine", true},
    {"bESP_TargetIndicator", true},
    {"bESP_GhostPiece", true},
    {"bESP_QueenFocus", false},
    {"bAutoPlay", false},
    {"bAutoPlay_HumanizedDelay", true},
    {"bTouchBlocker", false}
};

inline std::map<std::string, float> features_float = {
    {"fAIM_Fov", 120.0f},
    {"fAIM_MinPower", 20.0f},
    {"fAIM_MaxPower", 100.0f},
    {"fESP_GuidelineThickness", 2.5f},
    {"fESP_TrajectoryAlpha", 0.8f},
    {"fAutoPlay_DelayMs", 350.0f},
    {"fHeight", 0.0f},
    {"fWidth", 0.0f}
};

inline std::map<std::string, int> features_int = {
    {"iAIM_TargetPriority", 0},
    {"iAIM_MaxBounces", 2},
    {"iESP_GuidelineStyle", 0}
};

inline std::map<std::string, ImColor> features_color = {
    {"cGuidelineDirect", CGREEN},
    {"cGuidelineBank", CYELLOW},
    {"cTargetPiece", CRED},
    {"cPredictedPocket", CWHITE}
};

inline const char* targetPriorityModes[] = {"Closest Pocket", "Easiest Shot Angle", "Queen First"};
inline const char* guidelineStyles[] = {"Solid Line", "Dotted Path", "Gradient Line"};
inline const char* cushionBounceModes[] = {"Direct Only (0 Bounce)", "1 Cushion Bank", "2 Cushion Bank"};
