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

#ifndef LOGI
#define LOG_TAG "NativeHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#endif

#include <Dobby/include/dobby.h>
#include <Substrate/SubstrateHook.h>
#include <dlfcn/dlfcn.hpp>
#include <xhook/xhook.h>

#if defined(__aarch64__)
    #define HOOK(a, r) DobbyHook((void*)(a), (void*)r, (void**)&_##r)
    #define HOOKI(a, r, o) DobbyHook((void*)(a), (void*)r, (void**)&o)
#else
    #define HOOK(a, r) MSHookFunction((void*)(a), (void*)r, (void**)&_##r)
    #define HOOKI(a, r, o) MSHookFunction((void*)(a), (void*)r, (void**)&o)
#endif

#define HOOKS(l, s, r) ([]() { \
    void* pl = fdlopen(l, 4); \
    if (!pl) { LOGI("!dlopen %s", l); return; } \
    void* ps = fdlsym(pl, s); \
    if (!ps) { \
        LOGI("!dlsym %s %s", l, s); \
        fdlclose(pl); \
        return; \
    } \
    HOOK(ps, r); \
    fdlclose(pl); \
})()

#define DEFINES(type, func, ...) static type (*_##func)(__VA_ARGS__) = nullptr; static type func(__VA_ARGS__)
#define DEFINE(type, func, ...) type (*_##func)(__VA_ARGS__) = nullptr; type func(__VA_ARGS__)
