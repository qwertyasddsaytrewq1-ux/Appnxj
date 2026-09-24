/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <deque>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctime>
#include <android/log.h>

#define BUG_LOG_TAG "AppNativeLogger"
#define BUG_LOG_VERSION "V33-V13"
#define BUG_QUEUE_CAP 8192
#define BUG_ROTATE_BYTES (4 * 1024 * 1024)

struct BugLogState {
    pthread_mutex_t mtx;
    std::deque<std::string> queue;
    const char* path;
    bool thread_started;
    BugLogState() : path(nullptr), thread_started(false) {
        pthread_mutex_init(&mtx, nullptr);
    }
};

inline BugLogState& bls() { static BugLogState s; return s; }

inline const char* bug_log_resolve_path() {
    char pkg[256] = {0};
    FILE* cmd = fopen("/proc/self/cmdline", "r");
    if (cmd) { size_t n = fread(pkg, 1, sizeof(pkg) - 1, cmd); fclose(cmd); if (n) pkg[n] = 0; }
    if (!pkg[0]) strcpy(pkg, "com.miniclip.carrom");

    const char* candidates[6];
    char ext0[300], ext1[300], ext2[300];
    snprintf(ext0, sizeof(ext0), "/storage/emulated/0/Android/data/%s/files/Bug.txt", pkg);
    snprintf(ext1, sizeof(ext1), "/sdcard/Android/data/%s/files/Bug.txt", pkg);
    snprintf(ext2, sizeof(ext2), "/data/data/%s/files/Bug.txt", pkg);
    candidates[0] = "/storage/emulated/0/Download/Bug.txt";
    candidates[1] = "/sdcard/Download/Bug.txt";
    candidates[2] = ext0;
    candidates[3] = ext1;
    candidates[4] = "/data/local/tmp/Bug.txt";
    candidates[5] = ext2;

    for (int i = 0; i < 6; i++) {
        FILE* f = fopen(candidates[i], "a");
        if (f) { fclose(f); return strdup(candidates[i]); }
    }
    return "/data/local/tmp/Bug.txt";
}

inline void bug_log_drain_locked_to_file() {
    if (!bls().path) bls().path = bug_log_resolve_path();
    std::deque<std::string> local;
    pthread_mutex_lock(&bls().mtx);
    local.swap(bls().queue);
    pthread_mutex_unlock(&bls().mtx);
    if (local.empty()) return;

    struct stat st;
    if (stat(bls().path, &st) == 0 && st.st_size > BUG_ROTATE_BYTES) {
        char oldp[320];
        snprintf(oldp, sizeof(oldp), "%s.old", bls().path);
        remove(oldp);
        rename(bls().path, oldp);
    }

    FILE* f = fopen(bls().path, "a");
    if (!f) return;
    for (const auto& line : local) { fputs(line.c_str(), f); fputc('\n', f); }
    fclose(f);
}

inline void* bug_log_thread(void*) {

    if (!bls().path) bls().path = bug_log_resolve_path();
    {
        time_t t = time(nullptr);
        char header[512];
        snprintf(header, sizeof(header),
                 "==================================================\n"
                 "  NativeOverlay Bug.txt  (start %s"
                 "  file: %s\n"
                 "==================================================",
                 ctime(&t), bls().path);
        struct stat st;
        bool fresh = (stat(bls().path, &st) != 0 || st.st_size == 0);
        FILE* f = fopen(bls().path, fresh ? "w" : "a");
        if (f) { fputs(header, f); fclose(f); }
        __android_log_print(ANDROID_LOG_INFO, BUG_LOG_TAG, "BugLog: writing to %s", bls().path);
    }
    while (true) {
        usleep(500 * 1000);
        bug_log_drain_locked_to_file();
    }
    return nullptr;
}

inline void bug_log_ensure_thread() {
    if (bls().thread_started) return;
    bls().thread_started = true;
    pthread_t th;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&th, &attr, bug_log_thread, nullptr);
    pthread_attr_destroy(&attr);
}

inline void bug_log_write(const char* fmt, ...) {
    char buf[2048];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (n <= 0) return;
    if (n >= (int)sizeof(buf)) n = sizeof(buf) - 1;
    buf[n] = 0;

    char line[2148];
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tmv;
    localtime_r(&ts.tv_sec, &tmv);
    snprintf(line, sizeof(line), "[%02d:%02d:%02d.%03ld] %s",
             tmv.tm_hour, tmv.tm_min, tmv.tm_sec, ts.tv_nsec / 1000000, buf);

    __android_log_print(ANDROID_LOG_INFO, BUG_LOG_TAG, "%s", buf);

    bug_log_ensure_thread();
    pthread_mutex_lock(&bls().mtx);
    if ((int)bls().queue.size() >= BUG_QUEUE_CAP) bls().queue.pop_front();
    bls().queue.push_back(line);
    pthread_mutex_unlock(&bls().mtx);
}

inline void bug_log_flush() {
    if (!bls().thread_started) return;
    bug_log_drain_locked_to_file();
}

inline const char* bug_log_get_path() {
    if (!bls().path) bls().path = bug_log_resolve_path();
    return bls().path;
}

inline void bug_log_init() {
    bug_log_ensure_thread();
    bug_log_write("MOD LOADED: NativeOverlay %s — Bug.txt system ACTIVE, file: %s",
                  BUG_LOG_VERSION, bug_log_get_path());
    bug_log_flush();
}

#define BUGLOG(...) bug_log_write(__VA_ARGS__)
