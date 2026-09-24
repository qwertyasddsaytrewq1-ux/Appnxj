/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#include <cmath>
#include <cstdio>
#include <cstring>
#include <array>
#include <string>

#include <Vector/Vectors.h>
#include "include/includes.h"

bool isInstanceOf(ptr obj, std::string className) { return F(char*, *(ptr*)obj + 0x10) == className; }
char* getClassName(ptr obj) { return F(char*, *(ptr*)obj + 0x10); }

double normalizeDoublePrecision(double value, double negativeThreshold = 0.0, double negativeExtraLen = 0.0, size_t maxLen = 7) {
    if (std::abs(value) >= 10000.0) return std::floor(value);

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "%lf", value);
    size_t strLen = std::strlen(buffer);

    size_t allowedLen = maxLen;
    if (value < negativeThreshold) allowedLen = maxLen + negativeExtraLen;
    if (strLen > allowedLen) buffer[allowedLen] = '\0';

    double result = 0.0;
    std::sscanf(buffer, "%lf", &result);
    return result;
}

auto colors = std::to_array<ImColor>({
    ImColor(0.2f, 0.8f, 1.0f),
    ImColor(1.0f, 0.15f, 0.15f),

    ImColor(0.95f, 0.95f, 0.9f),
    ImColor(0.95f, 0.95f, 0.9f),
    ImColor(0.95f, 0.95f, 0.9f),
    ImColor(0.95f, 0.95f, 0.9f),
    ImColor(0.95f, 0.95f, 0.9f),
    ImColor(0.95f, 0.95f, 0.9f),
    ImColor(0.95f, 0.95f, 0.9f),
    ImColor(0.95f, 0.95f, 0.9f),
    ImColor(0.95f, 0.95f, 0.9f),

    ImColor(0.15f, 0.15f, 0.15f),
    ImColor(0.15f, 0.15f, 0.15f),
    ImColor(0.15f, 0.15f, 0.15f),
    ImColor(0.15f, 0.15f, 0.15f),
    ImColor(0.15f, 0.15f, 0.15f),
    ImColor(0.15f, 0.15f, 0.15f),
    ImColor(0.15f, 0.15f, 0.15f),
    ImColor(0.15f, 0.15f, 0.15f),
    ImColor(0.15f, 0.15f, 0.15f)
});

static_assert(colors.size() == 20);
