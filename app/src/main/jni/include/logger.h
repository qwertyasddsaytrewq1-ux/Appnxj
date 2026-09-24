/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include <android/log.h>
#include "bug_log.h"

#ifndef LOG_TAG
#define LOG_TAG "AppNativeLogger"
#endif

#ifdef NDEBUG

    #define LOGI(...) bug_log_write(__VA_ARGS__)
    #define LOGD(...) ((void)0)
    #define LOGW(...) bug_log_write(__VA_ARGS__)
    #define LOGE(...) bug_log_write(__VA_ARGS__)
#else

    #define LOGI(...) bug_log_write(__VA_ARGS__)
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
    #define LOGW(...) bug_log_write(__VA_ARGS__)
    #define LOGE(...) bug_log_write(__VA_ARGS__)
#endif

#define IFL(cond) if ([&](){ bool _res = (cond); if (_res) LOGI("%s", #cond); return _res; }())
#define IFLN(cond) if ([&](){ bool _res = (cond); if (!_res) LOGI("!(%s)", #cond); return _res; }())
