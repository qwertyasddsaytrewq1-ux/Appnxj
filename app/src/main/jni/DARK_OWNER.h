
#pragma once
#include <android/log.h>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <cmath>
struct BlackFeatures {
    bool predictionEnabled = true;
    bool trajectoryLines = true;
    bool autoAimEnabled = false;
    bool autoShotEnabled = false;
    bool autoQueueEnabled = false;
    bool autoMatchEnabled = false;
    bool matchLifecycleEnabled = true;
    bool noAdsEnabled = true;
    bool espLineEnabled = false;
    bool skinChangerEnabled = false;
    int  trajectoryLineCount = 5;
    float autoShotPower = 0.8f;
};
static BlackFeatures g_dark_owner;
namespace dark_owner_solver {
    struct TrajectoryPoint { float x, y; };
    static std::vector<TrajectoryPoint> g_trajectory;
    static std::mutex mutex;
    static inline void update(float sx, float sy, float angle, float power) {
        if (!g_dark_owner.predictionEnabled) return;
        std::lock_guard<std::mutex> lock(mutex);
        g_trajectory.clear();
        for (int i = 0; i < g_dark_owner.trajectoryLineCount; i++) {
            float t = (float)i / g_dark_owner.trajectoryLineCount;
            g_trajectory.push_back({sx + cosf(angle) * power * t * 100.0f, sy + sinf(angle) * power * t * 100.0f});
        }
    }
}
namespace dark_owner_bridge {
    struct AimData { float angle, power; bool has_aim; };
    static AimData g_aim = {0, 0, false};
    static std::mutex mutex;
    static inline void on_aim_change(float angle, float power) {
        if (!g_dark_owner.predictionEnabled) return;
        std::lock_guard<std::mutex> lock(mutex);
        g_aim = {angle, power, true};
    }
    static inline void on_aim_release(float angle, float power, float spin) {
        std::lock_guard<std::mutex> lock(mutex);
        g_aim.has_aim = false;
    }
    static inline void on_ownership_change(int playerId, int puckIndex) {}
    static inline void on_table_update(void* tableData) {}
}
namespace dark_owner_clone {
    static bool g_clone_active = false;
    static std::mutex mutex;
    static inline void on_clone(void* d) { if (!g_dark_owner.predictionEnabled) return; std::lock_guard<std::mutex> l(mutex); g_clone_active = true; }
    static inline void shutdown() { std::lock_guard<std::mutex> l(mutex); g_clone_active = false; }
}
namespace dark_owner_striker {
    struct StrikerState { float x, y, angle, power; bool active; };
    static StrikerState g_striker = {0,0,0,0,false};
    static std::mutex mutex;
    static inline void update(float x, float y, float a, float p) {
        if (!g_dark_owner.predictionEnabled) return;
        std::lock_guard<std::mutex> l(mutex);
        g_striker = {x,y,a,p,true};
    }
}
namespace dark_owner_recorder {
    struct ShotRecord { float angle, power, spin; };
    static std::vector<ShotRecord> g_shots;
    static std::mutex mutex;
    static inline void record(float a, float p, float s) {
        std::lock_guard<std::mutex> l(mutex);
        g_shots.push_back({a,p,s});
        if (g_shots.size() > 50) g_shots.erase(g_shots.begin());
    }
}
namespace dark_owner_assistant {
    struct ShotCandidate { float angle, power, spin, score; bool has_candidate; };
    static ShotCandidate g_candidate = {0,0,0,0,false};
    static std::mutex mutex;
    static inline void on_shot_data(void* d) { if (!g_dark_owner.autoAimEnabled) return; std::lock_guard<std::mutex> l(mutex); g_candidate.has_candidate = true; }
    static inline void on_shot_sim(void* d, int c) { if (!g_dark_owner.autoAimEnabled) return; std::lock_guard<std::mutex> l(mutex); }
    static inline void on_shot_release(float a, float p, float s) { if (!g_dark_owner.autoShotEnabled || !g_candidate.has_candidate) return; std::lock_guard<std::mutex> l(mutex); }
}
namespace dark_owner_game_bridge {
    static bool g_prereq = false;
    static std::mutex mutex;
    static inline void on_board_geometry(void* d) { std::lock_guard<std::mutex> l(mutex); g_prereq = true; }
    static inline void on_table_update(void* d) { dark_owner_bridge::on_table_update(d); }
}
namespace dark_owner_simulation {
    static bool g_sim = false;
    static std::mutex mutex;
    static inline void on_sim_table(void* d) { std::lock_guard<std::mutex> l(mutex); g_sim = true; }
    static inline void on_sim_cleanup() { std::lock_guard<std::mutex> l(mutex); g_sim = false; }
}
namespace dark_owner_auto_match {
    struct State { bool enabled, auto_retry, match_in_progress; int ap_mode, countdown; };
    static State g_am = {false, false, false, 1, 5};
    static std::mutex mutex;
    static inline void enable() { std::lock_guard<std::mutex> l(mutex); g_am.enabled = true; }
    static inline void cancel() { std::lock_guard<std::mutex> l(mutex); g_am.enabled = false; g_am.match_in_progress = false; }
    static inline void on_match_start() { std::lock_guard<std::mutex> l(mutex); g_am.match_in_progress = true; }
    static inline void on_match_end() { std::lock_guard<std::mutex> l(mutex); g_am.match_in_progress = false; }
}
namespace dark_owner_match_lifecycle {
    struct State { bool lobby_entered, match_started, ad_playing; int lobby_tier; };
    static State g_ml = {false, false, false, 0};
    static std::mutex mutex;
    static inline void on_lobby_entered(int t) { std::lock_guard<std::mutex> l(mutex); g_ml.lobby_entered = true; g_ml.lobby_tier = t; }
    static inline void on_match_started() { std::lock_guard<std::mutex> l(mutex); g_ml.match_started = true; }
    static inline void on_match_ended() { std::lock_guard<std::mutex> l(mutex); g_ml.match_started = false; }
    static inline bool is_in_match() { std::lock_guard<std::mutex> l(mutex); return g_ml.match_started; }
}
namespace dark_owner_ads {
    static std::atomic<bool> g_suppress{true};
    static inline bool should_suppress() { return g_dark_owner.noAdsEnabled && g_suppress.load(std::memory_order_relaxed); }
    static inline void set_enabled(bool e) { g_suppress.store(e, std::memory_order_relaxed); }
    static inline void on_ad_request() { if (should_suppress()) __android_log_print(ANDROID_LOG_INFO, "Ads", "suppressing ad"); }
}
namespace dark_owner_esp {
    struct State { bool esp_line_enabled; };
    static State g_esp = {false};
    static std::mutex mutex;
    static inline void set_esp_line(bool e) { std::lock_guard<std::mutex> l(mutex); g_esp.esp_line_enabled = e; }
    static inline bool is_esp_line_enabled() { std::lock_guard<std::mutex> l(mutex); return g_esp.esp_line_enabled; }
}
static inline void dark_owner_init_all() {
    __android_log_print(ANDROID_LOG_INFO, "TRUST", "Initializing all subsystems");
    dark_owner_auto_match::enable();
    dark_owner_ads::set_enabled(g_dark_owner.noAdsEnabled);
    __android_log_print(ANDROID_LOG_INFO, "TRUST", "All subsystems initialized");
}
static inline void SyncBlackFeatures() {
    dark_owner_ads::set_enabled(g_dark_owner.noAdsEnabled);
    dark_owner_esp::set_esp_line(g_dark_owner.espLineEnabled);
    if (g_dark_owner.autoMatchEnabled) dark_owner_auto_match::enable();
    else dark_owner_auto_match::cancel();
}
