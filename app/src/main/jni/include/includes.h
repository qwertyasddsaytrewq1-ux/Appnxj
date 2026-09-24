/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include <jni.h>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <initializer_list>
#include <cstring>
#include <map>
#include <vector>
#include <tuple>
#include <list>
#include <thread>
#include <set>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <chrono>
#include <deque>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <link.h>
#include <regex>

inline JavaVM* VM = nullptr;
#include "java.h"
#include "Vector/Vectors.h"
#include "stacktrace.h"
#include "pointers.h"
#include "logger.h"

using namespace std;

inline std::string PACKAGE_NAME;
inline uintptr_t libmain = 0;
inline uintptr_t libanogs = 0;
inline uintptr_t libintl = 0;
inline bool bOnInputEvent = false;
#ifndef BIMGUI_SETUP_DEFINED
#define BIMGUI_SETUP_DEFINED
inline bool bImguiSetup = false;
#endif
inline int Width = 0, Height = 0, Orientation = 0;
inline Vector2 screenCenter;

#define ptr uintptr_t
#define DWORD uintptr_t
#define ulong uint64_t
#define dword uint32_t
#define qword uint64_t

#define I(...) { LOGI(__VA_ARGS__); }
#define E(e, ...) { LOGD(__VA_ARGS__); e; }
#define R(i, ...) { LOGD(__VA_ARGS__); return i; }

#define IC(xyz) if (xyz) { \
    LOGI("%s:%d %s", rpart(__FILE__, '/').c_str(), __LINE__, #xyz); \
    continue; \
}

#define IDC(xyz) if (xyz) { \
    if (dynamic_bool["LogContinue"]) LOGI("%s:%d %s", rpart(__FILE__, '/').c_str(), __LINE__, #xyz); \
    continue; \
}

#define IR(xyz, ...) if (xyz) { \
    LOGI("%s:%d %s", rpart(__FILE__, '/').c_str(), __LINE__, #xyz); \
    return __VA_ARGS__; \
}

#define IDR(xyz, ...) if (xyz) { \
    if (dynamic_bool["LogReturn"]) LOGI("%s:%d %s", rpart(__FILE__, '/').c_str(), __LINE__, #xyz); \
    return __VA_ARGS__; \
}

#define ARGB(a, r, g, b) ((r << 0) | (g << 8) | (b << 16) | (a << 24))

#define WHITE              ImColor(255, 255, 255)
#define RED                ImColor(255, 0, 0)
#define GREEN              ImColor(0, 255, 0)
#define LIME               ImColor(0, 255, 0)
#define BLUE               ImColor(0, 0, 255)
#define BLACK              ImColor(0, 0, 0)
#define PURPLE             ImColor(128, 0, 128)
#define GREY               ImColor(128, 128, 128)
#define YELLOW             ImColor(255, 255, 0)
#define ORANGE             ImColor(255, 165, 0)
#define DARKGREEN          ImColor(0, 100, 0)
#define PINK               ImColor(255, 192, 203)
#define BROWN              ImColor(165, 42, 42)
#define CYAN               ImColor(0, 255, 255)

#define floop(s, e) for (int fi=s; fi<e; fi++)

inline void* void_func_wrapper(void* arg) {
    auto func = reinterpret_cast<void(*)()>(arg);
    if (func) func();
    return nullptr;
}

inline void pthread_create_detached(void (*func)(void)) {
    pthread_t x;
    pthread_create(&x, nullptr, void_func_wrapper, reinterpret_cast<void*>(func));
    pthread_detach(x);
}

inline void sleepm(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

inline bool EditPerm(void* addr, size_t len, int PERMISSION) {
    uintptr_t pageSize = sysconf(_SC_PAGE_SIZE);
    uintptr_t start = reinterpret_cast<uintptr_t>(addr) & ~(pageSize - 1);
    uintptr_t end = (reinterpret_cast<uintptr_t>(addr) + len + pageSize - 1) & ~(pageSize - 1);
    return mprotect(reinterpret_cast<void*>(start), end - start, PERMISSION) == 0;
}

inline bool EditMemory(ptr addr, size_t len, const void *data) {
    if (!EditPerm((void*)addr, len, PROT_READ | PROT_WRITE | PROT_EXEC)) return false;
    memcpy((void*)addr, data, len);
    EditPerm((void*)addr, len, PROT_READ | PROT_EXEC);
    return true;
}

#define DEFINED(type, name, ...) inline int __##name = -1; inline type (*_##name)(); inline type name()

DEFINED(uint, LOGS) { static int _I_ = 0; LOGI("LOGS: %i", _I_++); log_stacktrace(); return 0; }
DEFINED(uint, SLEEP) { static int _I_ = 0; LOGI("SLEEP: %i", _I_++); sleep(1); return 0; }
DEFINED(uint, SVOID) { return 0; }
DEFINED(uint, VOID) { static int _I_ = 0; LOGI("VOID: %i", _I_++); return 0; }
DEFINED(bool, STRUE) { return true; }
DEFINED(bool, TRUE) { static int _I_ = 0; LOGI("TRUE: %i", _I_++); return true; }
DEFINED(bool, SFALSE) { return false; }
DEFINED(bool, FALSE) { static int _I_ = 0; LOGI("FALSE: %i", _I_++); return false; }

inline int IMAX() { return 100000; }
inline int I0() { return 0; }
inline int I10() { return 10; }
inline int I1000() { return 1000; }
inline float FMAX() { return 100000.0f; }
inline float F1000() { return 1000.0f; }
inline float F100() { return 100.0f; }
inline float F10() { return 10.0f; }
inline float F0() { return 0.0f; }
inline float SF0() { return 0.0f; }
inline float SF10() { return 10.0f; }
inline float SF100() { return 100.0f; }
inline double DMAX() { return 100000.0; }

#define BT(addr) HOOKN(addr, TRUE);
#define BF(addr) HOOKN(addr, FALSE);
#define V(addr) HOOKN(addr, VOID);
#define IM(addr) HOOKN(addr, IMAX);
#define SV(addr) HOOKN(addr, SVOID);
#define ST(addr) HOOKN(addr, STRUE);
#define SF(addr) HOOKN(addr, SFALSE);

inline u_long htol(std::string strx) {
    return std::strtoul(strx.c_str(), nullptr, 16);
}

inline ptr absoluteAddress(const char *libraryName, uintptr_t offset = 0) {
    while (true) {
        std::ifstream maps("/proc/self/maps");
        std::string map;
        while (std::getline(maps, map)) {
            if (map.find(libraryName) != std::string::npos) {
                std::istringstream line(map);
                std::string start;
                std::getline(line, start, '-');
                return htol(start) + offset;
            }
        }
        sleepm(500);
    }
    return 0;
}

inline std::string rpart(const std::string& str, char c) {
    size_t last_slash = str.rfind(c);
    if (last_slash == std::string::npos) return str;
    return str.substr(last_slash + 1);
}

inline bool startswith(const std::string& str, const std::string& prefix) {
    return str.rfind(prefix, 0) == 0;
}

inline bool endswith(const char* str, const char* suffix) {
    if (!str || !suffix) return false;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return false;
    return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

inline std::string whois(const void* address, bool include_offset = false) {
    Dl_info info;
    char buf[256];
    if (!dladdr(address, &info)) {
        if (include_offset) {
            snprintf(buf, sizeof(buf), "[!dladdr](0x%lx)", (uintptr_t)address);
            return std::string(buf);
        }
        return "[!dladdr]";
    }

    auto libname = rpart(info.dli_fname, '/');
    if (!include_offset) return libname;

    uintptr_t offset = (uintptr_t)address - (uintptr_t)info.dli_fbase;
    snprintf(buf, sizeof(buf), "%s(0x%lx)", libname.c_str(), (unsigned long)offset);
    return std::string(buf);
}

inline std::string whois(ptr address, bool include_offset = false) {
    return whois((void*)address, include_offset);
}

#define TRIGGER_SAFEGUARD { \
    LOGI("%s:%d %s", rpart(__FILE__, '/').c_str(), __LINE__, "SAFEGUARD TRIGGERED, EXITING!"); \
    _exit(0); \
}

template<typename... Args>
inline std::string conc_impl(Args&&... args) { std::string r; ((r += std::string(std::forward<Args>(args))), ...); return r; }
#define CONC(...) conc_impl(__VA_ARGS__)

#define pthread(func) pthread_create_detached(func)

inline const char* getcmdline() {
    static char buf[256] = {0};
    int fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd >= 0) { read(fd, buf, sizeof(buf) - 1); close(fd); }
    return buf;
}

inline uintptr_t getCarromPoolBase() {
    return absoluteAddress("libgame-CARROM-GooglePlay-Gold-Release-Module-1456.so");
}

inline void SetHideRecording(bool hide) {
    if (hide) system("settings put secure hide_screen_update 1");
    else system("settings put secure hide_screen_update 0");
}

#include <sys/stat.h>
inline void create_directory_recursive(const std::string& path) {
    std::string dir;
    std::stringstream ss(path);
    std::string segment;
    while (std::getline(ss, segment, '/')) {
        if (!dir.empty()) dir += '/';
        dir += segment;
        mkdir(dir.c_str(), 0777);
    }
}

inline std::string getAndroidID(JNIEnv* env) {
    jclass atClass = env->FindClass("android/app/ActivityThread");
    if (!atClass) return "";
    jmethodID currentAT = env->GetStaticMethodID(atClass, "currentActivityThread", "()Landroid/app/ActivityThread;");
    if (!currentAT) { env->DeleteLocalRef(atClass); return ""; }
    jobject at = env->CallStaticObjectMethod(atClass, currentAT);
    if (!at) { env->DeleteLocalRef(atClass); return ""; }
    jmethodID getApp = env->GetMethodID(atClass, "getApplication", "()Landroid/app/Application;");
    jobject appContext = env->CallObjectMethod(at, getApp);
    env->DeleteLocalRef(atClass);
    if (!appContext) return "";
    std::string result = GetSystemAndroidID(appContext);
    env->DeleteLocalRef(appContext);
    return result;
}

inline std::string getClipboard(JNIEnv* env) {
    jclass atClass = env->FindClass("android/app/ActivityThread");
    if (!atClass) return "";
    jmethodID currentAT = env->GetStaticMethodID(atClass, "currentActivityThread", "()Landroid/app/ActivityThread;");
    if (!currentAT) { env->DeleteLocalRef(atClass); return ""; }
    jobject at = env->CallStaticObjectMethod(atClass, currentAT);
    if (!at) { env->DeleteLocalRef(atClass); return ""; }
    jmethodID getApp = env->GetMethodID(atClass, "getApplication", "()Landroid/app/Application;");
    jobject appContext = env->CallObjectMethod(at, getApp);
    env->DeleteLocalRef(atClass);
    if (!appContext) return "";
    std::string result = GetClipboardText(appContext);
    env->DeleteLocalRef(appContext);
    return result;
}

inline void __INPUT__() {

}
