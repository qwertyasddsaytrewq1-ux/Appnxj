LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libcurl
LOCAL_SRC_FILES := include/external/curl/curl-android-$(TARGET_ARCH_ABI)/lib/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libssl
LOCAL_SRC_FILES := include/external/curl/openssl-android-$(TARGET_ARCH_ABI)/lib/libssl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto
LOCAL_SRC_FILES := include/external/curl/openssl-android-$(TARGET_ARCH_ABI)/lib/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

# --- Dobby ---
include $(CLEAR_VARS)
LOCAL_MODULE := dobby
DOBBY_DIR := $(LOCAL_PATH)/include/external/Dobby
LOCAL_SRC_FILES := \
  $(DOBBY_DIR)/source/dobby.cpp \
  $(DOBBY_DIR)/source/Backend/UserMode/ExecMemory/clear-cache-tool-all.c \
  $(DOBBY_DIR)/source/Backend/UserMode/ExecMemory/code-patch-tool-posix.cc \
  $(DOBBY_DIR)/source/Backend/UserMode/MultiThreadSupport/ThreadSupport.cpp \
  $(DOBBY_DIR)/source/Backend/UserMode/PlatformUtil/Linux/ProcessRuntime.cc \
  $(DOBBY_DIR)/source/Backend/UserMode/Thread/PlatformThread.cc \
  $(DOBBY_DIR)/source/Backend/UserMode/Thread/platform-thread-posix.cc \
  $(DOBBY_DIR)/source/Backend/UserMode/UnifiedInterface/platform-posix.cc \
  $(DOBBY_DIR)/source/Backend/UserMode/UnifiedInterface/semaphore.cc \
  $(DOBBY_DIR)/source/InstructionRelocation/arm64/InstructionRelocationARM64.cc \
  $(DOBBY_DIR)/source/InterceptRouting/InstrumentRouting/instrument_routing_handler.cpp \
  $(DOBBY_DIR)/source/InterceptRouting/NearBranchTrampoline/near_trampoline_arm64.cc \
     $(DOBBY_DIR)/source/TrampolineBridge/ClosureTrampolineBridge/arm64/ClosureTrampolineARM64.cc \
  $(DOBBY_DIR)/source/TrampolineBridge/ClosureTrampolineBridge/arm64/closure_bridge_arm64.cc \
  $(DOBBY_DIR)/source/TrampolineBridge/ClosureTrampolineBridge/arm64/helper_arm64.cc \
  $(DOBBY_DIR)/source/TrampolineBridge/Trampoline/trampoline_arm64.cc \
  $(DOBBY_DIR)/source/core/codegen/codegen-arm.cc \
  $(DOBBY_DIR)/source/core/emulator/dummy.cc \
  $(DOBBY_DIR)/builtin-plugin/SymbolResolver/elf/dobby_symbol_resolver.cc \
  $(DOBBY_DIR)/external/logging/logging.cc
LOCAL_EXPORT_C_INCLUDES := $(DOBBY_DIR)/include
LOCAL_C_INCLUDES := $(DOBBY_DIR)/include $(DOBBY_DIR)/source $(DOBBY_DIR)/source/dobby $(DOBBY_DIR)/external/logging $(DOBBY_DIR)/external/osbase 
LOCAL_C_INCLUDES += $(DOBBY_DIR)/source/Backend/UserMode
LOCAL_C_INCLUDES += $(DOBBY_DIR)/builtin-plugin
LOCAL_C_INCLUDES += $(DOBBY_DIR)
LOCAL_C_INCLUDES += $(DOBBY_DIR)/external
LOCAL_C_INCLUDES += $(DOBBY_DIR)/source/InterceptRouting
LOCAL_C_INCLUDES += $(DOBBY_DIR)/source/MemoryAllocator
LOCAL_CFLAGS := -O2 -fvisibility=hidden -fno-rtti -fno-exceptions -DNDEBUG
LOCAL_CPPFLAGS := -O2 -fno-rtti -fno-exceptions -std=c++17
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := DARK_OWNER

LOCAL_CFLAGS := -Wno-error=format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w
LOCAL_CFLAGS += -funwind-tables
LOCAL_CPPFLAGS := -std=c++20 -Wno-error=format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w -s
LOCAL_CPPFLAGS += -Wno-error=c++11-narrowing -fms-extensions -fexceptions -fpermissive
LOCAL_CPPFLAGS += -funwind-tables -fno-omit-frame-pointer
LOCAL_LDFLAGS += -Wl,--strip-all -Wl,--exclude-libs,ALL -Wl,--gc-sections
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := main.cpp \
  include/external/dlfcn/dlfcn.cpp \
  include/external/imgui/imgui.cpp \
  include/external/Substrate/SubstrateHook.cpp \
  include/external/Substrate/SubstrateDebug.cpp \
  include/external/Substrate/SubstratePosixMemory.cpp \
  include/external/oxorany/oxorany.cpp

LOCAL_C_INCLUDES := $(NDK_ROOT)/sources/android
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/external
LOCAL_C_INCLUDES += $(LOCAL_PATH)/game
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/external/imgui
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/external/imgui/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/external/imgui/src/backends
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/external/imgui/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/external/Dobby/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/external/curl/curl-android-$(TARGET_ARCH_ABI)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/external/curl/openssl-android-$(TARGET_ARCH_ABI)/include

LOCAL_LDLIBS := -landroid -lGLESv3 -lEGL -ldl -llog -lz
LOCAL_STATIC_LIBRARIES := libcurl libssl libcrypto dobby
include $(BUILD_SHARED_LIBRARY)
