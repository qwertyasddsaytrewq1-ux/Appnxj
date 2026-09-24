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
#include <vector>
#include <string>
#include <mutex>
#include "imgui.h"

#ifndef BIMGUI_SETUP_DEFINED
#define BIMGUI_SETUP_DEFINED
inline bool bImguiSetup = false;
#endif
#ifndef BTOUCH_OUTSIDE_DEFINED
#define BTOUCH_OUTSIDE_DEFINED
inline bool bTouchFromOutsideImGui = false;
inline bool g_MenuConsumesTouch = false;
#endif

inline bool IsPointOverAnyWindow(ImVec2 point) {
    return ImGui::IsAnyItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}

typedef void (*TouchBeginFn)(JNIEnv*, jobject, jint, float, float, jboolean, jboolean);
typedef void (*TouchEndFn)(JNIEnv*, jobject, jint, float, float, jboolean, jboolean);
typedef void (*TouchMoveFn)(JNIEnv*, jobject, jintArray, jfloatArray, jfloatArray, jboolean, jboolean);

inline TouchBeginFn _nativeTouchesBegin = nullptr;
inline TouchEndFn _nativeTouchesEnd = nullptr;
inline TouchMoveFn _nativeTouchesMove = nullptr;

inline float g_AimTouchX = -1.f, g_AimTouchY = -1.f;
inline bool  g_AimTouchActive = false;

inline void nativeTouchesBegin(JNIEnv* env, jobject obj, jint i, float x, float y, jboolean isObscured, jboolean isPartiallyObscured) {
    if (!bImguiSetup) {
        if (_nativeTouchesBegin) _nativeTouchesBegin(env, obj, i, x, y, isObscured, isPartiallyObscured);
        return;
    }

    if (i == 0) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[i] = true;
        io.MousePos = ImVec2(x, y);

        bTouchFromOutsideImGui = !g_MenuConsumesTouch && !IsPointOverAnyWindow(ImVec2(x, y));
    }

    if (g_MenuConsumesTouch) return;

    if (bTouchFromOutsideImGui && _nativeTouchesBegin) {
        _nativeTouchesBegin(env, obj, i, x, y, isObscured, isPartiallyObscured);
        if (i == 0) { g_AimTouchActive = true; g_AimTouchX = x; g_AimTouchY = y; }
    }
}

inline void nativeTouchesEnd(JNIEnv* env, jobject obj, jint i, float x, float y, jboolean isObscured, jboolean isPartiallyObscured) {
    if (!bImguiSetup) {
        if (_nativeTouchesEnd) _nativeTouchesEnd(env, obj, i, x, y, isObscured, isPartiallyObscured);
        return;
    }

    if (i == 0) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[i] = false;

        if (g_MenuConsumesTouch) {
            bTouchFromOutsideImGui = false;
            return;
        }
        if (bTouchFromOutsideImGui) {
            bTouchFromOutsideImGui = false;
        } else {
            return;
        }
    }

    if (_nativeTouchesEnd) {
        _nativeTouchesEnd(env, obj, i, x, y, isObscured, isPartiallyObscured);
    }
    if (i == 0) g_AimTouchActive = false;
}

inline void nativeTouchesMove(JNIEnv* env, jobject obj, jintArray ids, jfloatArray xs, jfloatArray ys, jboolean isObscured, jboolean isPartiallyObscured) {
    if (!bImguiSetup) {
        if (_nativeTouchesMove) _nativeTouchesMove(env, obj, ids, xs, ys, isObscured, isPartiallyObscured);
        return;
    }

    jsize length = env->GetArrayLength(ids);
    if (length <= 0) {
        if (_nativeTouchesMove) _nativeTouchesMove(env, obj, ids, xs, ys, isObscured, isPartiallyObscured);
        return;
    }

    jint* id_elements = env->GetIntArrayElements(ids, NULL);
    jfloat* x_elements = env->GetFloatArrayElements(xs, NULL);
    jfloat* y_elements = env->GetFloatArrayElements(ys, NULL);

    int i = id_elements[0];
    float x = x_elements[0], y = y_elements[0];

    if (g_MenuConsumesTouch) {
        if (i == 0) {
            ImGui::GetIO().MousePos = ImVec2(x, y);
        }
        env->ReleaseIntArrayElements(ids, id_elements, JNI_ABORT);
        env->ReleaseFloatArrayElements(xs, x_elements, JNI_ABORT);
        env->ReleaseFloatArrayElements(ys, y_elements, JNI_ABORT);
        return;
    }

    env->ReleaseIntArrayElements(ids, id_elements, JNI_ABORT);
    env->ReleaseFloatArrayElements(xs, x_elements, JNI_ABORT);
    env->ReleaseFloatArrayElements(ys, y_elements, JNI_ABORT);

    if (i == 0) {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = ImVec2(x, y);
    }

    if (bTouchFromOutsideImGui && _nativeTouchesMove) {
        _nativeTouchesMove(env, obj, ids, xs, ys, isObscured, isPartiallyObscured);
        if (i == 0) { g_AimTouchX = x; g_AimTouchY = y; }
    }
}

struct PendingTouchEvent {
    int action;
    float x, y;
};
inline std::vector<PendingTouchEvent> g_PendingTouches;
inline std::mutex g_PendingTouchMutex;

inline void ImGui_FeedTouchToIO(int action, float x, float y) {
    std::lock_guard<std::mutex> lock(g_PendingTouchMutex);
    g_PendingTouches.push_back({action, x, y});
}

inline void ImGui_ProcessPendingTouch() {
    std::vector<PendingTouchEvent> events;
    {
        std::lock_guard<std::mutex> lock(g_PendingTouchMutex);
        events.swap(g_PendingTouches);
    }
    ImGuiIO& io = ImGui::GetIO();
    for (auto& e : events) {
        if (e.action == 0 || e.action == 2) {
            io.MousePos = ImVec2(e.x, e.y);
            io.MouseDown[0] = true;
        } else {
            io.MouseDown[0] = false;
        }
    }
}

inline void NativeTouchesBegin(JNIEnv* env, int idx, float x, float y) {

    if (_nativeTouchesBegin) _nativeTouchesBegin(env, nullptr, idx, x, y, false, false);
}

inline void NativeTouchesEnd(JNIEnv* env, int idx, float x, float y) {
    nativeTouchesEnd(env, nullptr, idx, x, y, false, false);
}

inline void NativeTouchesMove(JNIEnv* env, int idx, float x, float y) {

    jintArray ids = env->NewIntArray(1);
    env->SetIntArrayRegion(ids, 0, 1, &idx);
    jfloatArray xs = env->NewFloatArray(1);
    env->SetFloatArrayRegion(xs, 0, 1, &x);
    jfloatArray ys = env->NewFloatArray(1);
    env->SetFloatArrayRegion(ys, 0, 1, &y);

    nativeTouchesMove(env, nullptr, ids, xs, ys, false, false);

    env->DeleteLocalRef(ids);
    env->DeleteLocalRef(xs);
    env->DeleteLocalRef(ys);
}
