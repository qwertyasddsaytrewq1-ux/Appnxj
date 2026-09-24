/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once
#include <stdio.h>
#include <string.h>
#include <android/log.h>
#include <chrono>
#include <ctime>
#define LOGCUT_TAG "LogCut"
static const char* LOGCUT_PATHS[] = {
    "/storage/emulated/0/Download/logcut.txt",
    "/sdcard/Download/logcut.txt",
    "/storage/emulated/0/logcut.txt",
    "/sdcard/logcut.txt",
    "/data/local/tmp/logcut.txt",
    "/data/data/com.miniclip.carrom/files/logcut.txt",
    nullptr
};
static const char* g_logcut_path = nullptr;
static inline const char* logcut_get_path() {
    if (g_logcut_path) return g_logcut_path;
    for (int i = 0; LOGCUT_PATHS[i]; i++) {
        FILE* f = fopen(LOGCUT_PATHS[i], "w");
        if (f) { fclose(f); g_logcut_path = LOGCUT_PATHS[i]; return g_logcut_path; }
    }
    g_logcut_path = "/storage/emulated/0/Download/logcut.txt";
    return g_logcut_path;
}
static inline void logcut_init() {
    const char* path = logcut_get_path();
    FILE* f = fopen(path, "w");
    if (f) {
        fprintf(f, "==============================================================\n");
        fprintf(f, "  NativeOverlay V17 Black — LogCut\n");
        fprintf(f, "  Log file: %s\n", path);
        fprintf(f, "==============================================================\n\n");
        fclose(f);
    }
    __android_log_print(ANDROID_LOG_INFO, LOGCUT_TAG, "LogCut initialized at %s", path);
}
static inline void logcut_write(const char* level, const char* tag, const char* fmt, ...) {
    char buf[4096];
    int pos = 0;
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    char ts[32];
    strftime(ts, sizeof(ts), "%H:%M:%S", localtime(&t));
    pos += snprintf(buf + pos, sizeof(buf) - pos, "[%s.%03d] [%s] [%s] ", ts, (int)ms.count(), level, tag);
    va_list args;
    va_start(args, fmt);
    pos += vsnprintf(buf + pos, sizeof(buf) - pos, fmt, args);
    va_end(args);
    __android_log_print(ANDROID_LOG_INFO, LOGCUT_TAG, "%s", buf);
    const char* path = logcut_get_path();
    FILE* f = fopen(path, "a");
    if (f) { fprintf(f, "%s\n", buf); fclose(f); }
}
#define LOGCUT_INFO(tag, ...) logcut_write("INFO", tag, __VA_ARGS__)
#define LOGCUT_WARN(tag, ...) logcut_write("WARN", tag, __VA_ARGS__)
#define LOGCUT_ERROR(tag, ...) logcut_write("ERROR", tag, __VA_ARGS__)
#define LOGCUT_HOOK(tag, ...) logcut_write("HOOK", tag, __VA_ARGS__)
#define LOGCUT_DARK_OWNER(tag, ...) logcut_write("DARK_OWNER", tag, __VA_ARGS__)
static inline void logcut_dump_libgame(uintptr_t base) {
    if (base) LOGCUT_INFO("LibGame", "base address: 0x%lx", (unsigned long)base);
}
