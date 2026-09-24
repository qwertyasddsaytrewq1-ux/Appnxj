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
#include <android/log.h>

#define LOG_TAG "NativeHelper"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct JNIThreadScope {
    JNIEnv* env = nullptr;
    bool needsDetach = false;

    JNIThreadScope() {
        if (!VM) return;
        jint res = VM->GetEnv((void**)&env, JNI_VERSION_1_6);
        if (res == JNI_EDETACHED) {
            if (VM->AttachCurrentThread(&env, nullptr) == 0) {
                needsDetach = true;
            }
        }
    }

    ~JNIThreadScope() {
        if (needsDetach && VM) {
            VM->DetachCurrentThread();
        }
    }
};

inline std::string GetSystemAndroidID(jobject appContext) {
    JNIThreadScope scope;
    if (!scope.env || !appContext) return "";

    JNIEnv* env = scope.env;

    jclass contextClass = env->FindClass("android/content/Context");
    jmethodID getContentResolver = env->GetMethodID(contextClass, "getContentResolver", "()Landroid/content/ContentResolver;");
    jobject contentResolver = env->CallObjectMethod(appContext, getContentResolver);

    jclass settingsSecureClass = env->FindClass("android/provider/Settings$Secure");
    jfieldID androidIdField = env->GetStaticFieldID(settingsSecureClass, "ANDROID_ID", "Ljava/lang/String;");
    jstring androidIdStr = (jstring)env->GetStaticObjectField(settingsSecureClass, androidIdField);

    jmethodID getStringMethod = env->GetStaticMethodID(settingsSecureClass, "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
    jstring idJStr = (jstring)env->CallStaticObjectMethod(settingsSecureClass, getStringMethod, contentResolver, androidIdStr);

    std::string result;
    if (idJStr) {
        const char* chars = env->GetStringUTFChars(idJStr, nullptr);
        result = chars;
        env->ReleaseStringUTFChars(idJStr, chars);
        env->DeleteLocalRef(idJStr);
    }

    env->DeleteLocalRef(contentResolver);
    env->DeleteLocalRef(contextClass);
    env->DeleteLocalRef(settingsSecureClass);
    env->DeleteLocalRef(androidIdStr);

    return result;
}

inline std::string GetClipboardText(jobject appContext) {
    JNIThreadScope scope;
    if (!scope.env || !appContext) return "";

    JNIEnv* env = scope.env;

    jclass contextClass = env->FindClass("android/content/Context");
    jmethodID getSystemService = env->GetMethodID(contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");

    jstring serviceName = env->NewStringUTF("clipboard");
    jobject clipboardManager = env->CallObjectMethod(appContext, getSystemService, serviceName);
    env->DeleteLocalRef(serviceName);

    std::string result = "";
    if (clipboardManager) {
        jclass clipboardClass = env->FindClass("android/content/ClipboardManager");
        jmethodID getTextMethod = env->GetMethodID(clipboardClass, "getText", "()Ljava/lang/CharSequence;");
        jobject charSequence = env->CallObjectMethod(clipboardManager, getTextMethod);

        if (charSequence) {
            jclass charSeqClass = env->FindClass("java/lang/CharSequence");
            jmethodID toStringMethod = env->GetMethodID(charSeqClass, "toString", "()Ljava/lang/String;");
            jstring textStr = (jstring)env->CallObjectMethod(charSequence, toStringMethod);

            const char* chars = env->GetStringUTFChars(textStr, nullptr);
            result = chars;
            env->ReleaseStringUTFChars(textStr, chars);

            env->DeleteLocalRef(textStr);
            env->DeleteLocalRef(charSeqClass);
            env->DeleteLocalRef(charSequence);
        }
        env->DeleteLocalRef(clipboardClass);
        env->DeleteLocalRef(clipboardManager);
    }

    env->DeleteLocalRef(contextClass);
    return result;
}
