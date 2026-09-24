/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include <csignal>
#include <csetjmp>
#include <cstdint>
#include <android/log.h>

#define LOG_TAG "NativeCrashHandler"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace SignalManager {

    static sigjmp_buf g_jumpBuffer;
    static volatile sig_atomic_t g_jumpActive = 0;

    inline void StandardCrashHandler(int signal, siginfo_t* info, void* context) {
        LOGE("Captured Signal: %d at address: %p", signal, info->si_addr);

        if (g_jumpActive) {
            g_jumpActive = 0;
            siglongjmp(g_jumpBuffer, 1);
        }

        _exit(signal);
    }

    inline void RegisterSignalHandlers() {
        struct sigaction sa{};
        sa.sa_sigaction = StandardCrashHandler;
        sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
        sigemptyset(&sa.sa_mask);

        sigaction(SIGSEGV, &sa, nullptr);
        sigaction(SIGBUS,  &sa, nullptr);
        sigaction(SIGABRT, &sa, nullptr);
        sigaction(SIGILL,  &sa, nullptr);
    }

    template <typename T>
    inline bool SafeRead(uintptr_t address, T& outValue) {
        if (!address) return false;

        g_jumpActive = 1;
        if (sigsetjmp(g_jumpBuffer, 1) == 0) {
            outValue = *reinterpret_cast<const T*>(address);
            g_jumpActive = 0;
            return true;
        }

        g_jumpActive = 0;
        return false;
    }
}
