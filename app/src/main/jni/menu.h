
#pragma once
#include "include/includes.h"
#include "include/crash_log.h"
#include "game.h"
#include "game/Ruleset.h"
#include <curl/curl.h>
#include "include/java.h"
#include "mod/keylogin.h"
#include "oxorany/oxorany.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <sys/system_properties.h>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <Vector/Vectors.h>
#include <thread>
#include <condition_variable>
#include <imgui/imgui.h>
#include "icons/icons.h"
#include "icons/play_logo.h"
#include "icons/ref/black_logo_png.h"
#include "icons/ref_key_png.h"
#include "icons/ref_paste_png.h"
#include "icons/ref_telegram_png.h"

using namespace ImGui;
using namespace std;

struct MenuState {
    bool isOpen = false;
    int currentTab = 0;
    float sidebarWidth = 750.0f;
    float animProgress = 0.0f;
    float menuAlpha = 0.0f;
    float menuScale = 0.9f;
    ImVec4 accentColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
};
static MenuState g_menu;

static int   g_refAutoPlayMode = 0;
static int   g_refEspLineStyle = 0;
static float g_refLineThickness = 6.9f;
static float g_refLineOpacity = 0.9f;
static float g_refLineAnim = 0.0f;

static const int64_t EXPIRY_TS = O(1874793600LL);

static bool DEBUG_BYPASS_LOGIN = false;

static float EaseOutBack(float x) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(x - 1.0f, 3.0f) + c1 * powf(x - 1.0f, 2.0f);
}

static float EaseOutQuart(float x) {
    return 1.0f - powf(1.0f - x, 4.0f);
}

static void DrawGradientRect(ImDrawList* dl, ImVec2 p1, ImVec2 p2, ImU32 col1, ImU32 col2, bool horizontal = true) {
    if (horizontal) {
        dl->AddRectFilledMultiColor(p1, p2, col1, col2, col2, col1);
    } else {
        dl->AddRectFilledMultiColor(p1, p2, col1, col1, col2, col2);
    }
}

static const ImU32 UI_GOLD       = IM_COL32(255, 215, 0, 255);
static const ImU32 UI_GOLD_DARK   = IM_COL32(180, 150, 0, 255);
static const ImU32 UI_GOLD_LIGHT  = IM_COL32(255, 235, 59, 255);
static const ImU32 UI_CREAM       = IM_COL32(42, 42, 42, 255);
static const ImU32 UI_PANEL       = IM_COL32(45, 45, 45, 255);
static const ImU32 UI_PANEL_DARK  = IM_COL32(26, 26, 26, 255);
static const ImU32 UI_TEXT        = IM_COL32(255, 255, 255, 255);
static const ImU32 UI_MUTED       = IM_COL32(112, 83, 45, 255);

static void DrawGoldPanel(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding = 18.0f) {
    dl->AddRectFilled(a, b, UI_PANEL, rounding);
    dl->AddRect(a, b, IM_COL32(132, 82, 20, 255), rounding, 0, 2.0f);
    dl->AddRect(ImVec2(a.x+3, a.y+3), ImVec2(b.x-3, b.y-3), IM_COL32(236, 196, 105, 220), rounding-3.0f, 0, 1.0f);
}

static void DrawAutoPlayTarget(ImDrawList* dl, ImVec2 center, float r) {
    const ImU32 gold = IM_COL32(151, 94, 18, 255);
    const ImU32 light = IM_COL32(196, 135, 43, 255);
    dl->AddCircle(center, r, gold, 40, 2.2f);
    dl->AddCircle(center, r * 0.56f, light, 40, 2.0f);
    dl->AddCircleFilled(center, r * 0.18f, gold);
    dl->AddLine(ImVec2(center.x-r*1.18f, center.y), ImVec2(center.x-r*0.72f, center.y), gold, 2.0f);
    dl->AddLine(ImVec2(center.x+r*0.72f, center.y), ImVec2(center.x+r*1.18f, center.y), gold, 2.0f);
    dl->AddLine(ImVec2(center.x, center.y-r*1.18f), ImVec2(center.x, center.y-r*0.72f), gold, 2.0f);
    dl->AddLine(ImVec2(center.x, center.y+r*0.72f), ImVec2(center.x, center.y+r*1.18f), gold, 2.0f);
}

static bool SidebarButton(const char* label, GLuint iconTex, bool selected, float width) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return false;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    float iconSize = 46.0f;
    float vPad = 8.0f;
    float btnH = vPad + iconSize + 4.0f + g.FontSize + vPad;
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size(width, btnH);
    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id)) return false;
    bool hovered=false, held=false;
    bool pressed=ButtonBehavior(bb,id,&hovered,&held);
    ImDrawList* dl=window->DrawList;
    if (selected) {
        dl->AddRectFilled(bb.Min, bb.Max, UI_GOLD, 12.0f);
        dl->AddRect(bb.Min, bb.Max, IM_COL32(255,236,173,180), 12.0f, 0, 1.5f);
    } else if (hovered) {
        dl->AddRectFilled(bb.Min, bb.Max, IM_COL32(190,145,70,75), 12.0f);
    }
    ImVec2 center(bb.Min.x + width*0.5f, bb.Min.y + vPad + iconSize*0.5f);
    if (iconTex) {
        ImVec2 mn(center.x-iconSize*0.5f,center.y-iconSize*0.5f);
        ImVec2 mx(center.x+iconSize*0.5f,center.y+iconSize*0.5f);
        dl->AddImage((void*)(intptr_t)iconTex,mn,mx,ImVec2(0,0),ImVec2(1,1));
    }
    ImVec2 labelSize=CalcTextSize(label);
    ImVec2 textPos(bb.Min.x+(width-labelSize.x)*0.5f,bb.Min.y+vPad+iconSize+4.0f);
    dl->AddText(textPos,selected?IM_COL32(255,248,225,255):UI_TEXT,label);
    return pressed;
}

static bool ToggleSwitch(const char* label, bool* v) {
    ImGuiWindow* window=GetCurrentWindow();
    if(window->SkipItems) return false;
    ImGuiContext& g=*GImGui;
    const ImGuiID id=window->GetID(label);
    float height=28.0f, width=54.0f, radius=height*0.5f;
    ImVec2 textSize=CalcTextSize(label);
    ImVec2 pos=window->DC.CursorPos;
    ImVec2 size(GetContentRegionAvail().x, ImMax(height,textSize.y)+14.0f);
    ImRect bb(pos,pos+size);
    ItemSize(size,g.Style.FramePadding.y);
    if(!ItemAdd(bb,id)) return false;
    bool hovered=false,held=false;
    bool pressed=ButtonBehavior(bb,id,&hovered,&held);
    if(pressed)*v=!*v;
    static std::map<ImGuiID,float> anim;
    float &t=anim[id];
    float target=*v?1.0f:0.0f;
    t+=(target-t)*g.IO.DeltaTime*14.0f;
    ImDrawList* dl=window->DrawList;
    if(hovered) dl->AddRectFilled(bb.Min,bb.Max,IM_COL32(126,87,34,32),10.0f);
    ImVec2 togglePos(bb.Max.x-width-12.0f,bb.Min.y+(size.y-height)*0.5f);
    ImVec2 toggleEnd(togglePos.x+width,togglePos.y+height);
    ImVec4 off=ImVec4(0.2f,0.2f,0.2f,1.0f);
    ImVec4 on =ImVec4(1.0f,0.84f,0.0f,1.0f);
    dl->AddRectFilled(togglePos,toggleEnd,ImColor(ImLerp(off,on,t)),radius);
    dl->AddRect(togglePos,toggleEnd,IM_COL32(60,60,60,180),radius,0,1.0f);
    float knobX=togglePos.x+radius+(width-height)*t;
    float knobY=togglePos.y+radius;
    dl->AddCircleFilled(ImVec2(knobX+1,knobY+1),radius-3,IM_COL32(0,0,0,55));
    dl->AddCircleFilled(ImVec2(knobX,knobY),radius-4,IM_COL32(255,250,232,255));
    dl->AddText(ImVec2(bb.Min.x+12.0f,bb.Min.y+(size.y-textSize.y)*0.5f),UI_TEXT,label);
    return pressed;
}

static bool g_aqCounting = false;
static std::chrono::steady_clock::time_point g_aqLastCall;
static std::chrono::steady_clock::time_point g_aqCountdownStart;

static bool IsExpired() {
    return (int64_t)time(nullptr) >= EXPIRY_TS;
}

INLINE void DrawExpired(ImGuiIO& io) {
    float winW = g_menu.sidebarWidth;

    SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    SetNextWindowSize(ImVec2(winW, 0), ImGuiCond_Always);
    PushStyleColor(ImGuiCol_WindowBg, IM_COL32(18, 18, 18, 255));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 20.0f);
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30.0f, 30.0f));
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (Begin(O("##ExpiredWin"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
              ImGuiWindowFlags_AlwaysAutoResize)) {

        SetWindowFontScale(1.6f);
        ImVec2 titleSz = CalcTextSize(O("GALVNIC ENGINE EXPIRED"));
        SetCursorPosX((winW - 60.0f - titleSz.x) * 0.5f);
        TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "%s", O("GALVNIC ENGINE EXPIRED"));
        SetWindowFontScale(1.0f);

        Dummy(ImVec2(0, 16));

        PushTextWrapPos(GetCursorPosX() + winW - 60.0f);
        TextColored(ImVec4(0.85f, 0.85f, 0.90f, 1.0f), "%s",
            O("Beta Version Expired. Update on our Telegram Your Id"));
        PopTextWrapPos();

        Dummy(ImVec2(0, 10));
    }
    End();
    PopStyleVar(3);
    PopStyleColor();
}

INLINE void DrawAutoQueue() {
    AutoQueueTick();
    if ((!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN) {
        auto now = std::chrono::steady_clock::now();

        static bool hasStartedOnce = false;
        static auto lastStart = std::chrono::steady_clock::now() - std::chrono::milliseconds(2500);
        const auto sinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStart).count();
        if (hasStartedOnce && sinceStart < 2500) return;

        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - g_aqLastCall).count() > 500)
            g_aqCounting = false;
        g_aqLastCall = now;

        if (!g_aqCounting) {
            g_aqCounting = true;
            g_aqCountdownStart = now;
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_aqCountdownStart).count();
        const int countdown_ms = 1200;
        int remaining_ms = countdown_ms - (int)elapsed;

        if (remaining_ms <= 0) {
            StartAutoQueue();
            lastStart = now;
            hasStartedOnce = true;
            g_aqCounting = false;
            g_aqLastCall = now;
            return;
        }

        std::string count_str = std::to_string((remaining_ms + 999) / 1000);

        SetNextWindowPos(ImVec2(Width * 0.5f, Height * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1.f));
        PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 20.0f));
        PushStyleVar(ImGuiStyleVar_WindowRounding, 24.0f);

        if (Begin(O("##AutoQueueCD"), nullptr,
                  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                  ImGuiWindowFlags_AlwaysAutoResize)) {
            ImDrawList* dl  = GetWindowDrawList();
            ImVec2      wp  = GetWindowPos();
            ImVec2      ws  = GetWindowSize();
            dl->AddRectFilled(wp, ImVec2(wp.x + ws.x, wp.y + ws.y), IM_COL32(20, 20, 28, 0), 24.0f);

            SetWindowFontScale(3.5f);
            TextColored(ImVec4(1.f, 0.f, 0.f, 1.0f), "%s", count_str.c_str());
            SetWindowFontScale(1.0f);
        }
        End();
        PopStyleVar(2);
        PopStyleColor();
    }
}

#include "mod/ButtonClicker.h"

static void DrawToggleButton(bool cancelMode);

extern volatile bool g_autoQueueEnabled;

extern char g_espLastReason[160];

static struct SimWorker {
    std::mutex mtx;
    std::condition_variable cv;
    double reqAngle = 0.0, reqPower = 0.0;
    uint64_t reqVer = 0, servedVer = 0;
    bool started = false;
    std::thread th;
    int snapCount = 0;
    Prediction::PieceState snapPieces[MAX_PIECES_COUNT];
    void start() {
        if (started) return;
        started = true;
        th = std::thread([this]{
            for (;;) {
                double a, p;
                { std::unique_lock<std::mutex> lk(mtx);
                  cv.wait(lk, [&]{ return reqVer > servedVer; });
                  a = reqAngle; p = reqPower; servedVer = reqVer; }
                if (gPrediction) {
                    gPrediction->determineShotResult(false, a, p);
                    std::lock_guard<std::mutex> lk(mtx);
                    snapCount = gPrediction->guiData.piecesCount;
                    if (snapCount > MAX_PIECES_COUNT) snapCount = MAX_PIECES_COUNT;
                    for (int i = 0; i < snapCount; i++)
                        snapPieces[i] = gPrediction->guiData.pieces[i];
                }
            }
        });
        th.detach();
    }
    void submit(double a, double p) {
        if (!started) start();
        { std::lock_guard<std::mutex> lk(mtx); reqAngle = a; reqPower = p; reqVer++; }
        cv.notify_one();
    }
} s_simWorker;

inline void esp_status_impl(const char* line) {
    static const char* lastReason = "";
    static int lastLogMs = 0;
    auto nowMs = (int)(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count() % 1000000000);
    bool changed = strcmp(lastReason, line) != 0;
    if (changed || nowMs - lastLogMs > 5000) {

        bug_log_write("ESP: %s", line);
        lastReason = line;
        lastLogMs = nowMs;

        snprintf(g_espLastReason, sizeof(g_espLastReason), "%s", line);
    }
}
inline void esp_status(const char* reason) { esp_status_impl(reason); }
inline void esp_status_id(const char* reason, int id) {
    char buf[96]; snprintf(buf, sizeof(buf), "%s %d", reason, id); esp_status_impl(buf);
}

extern struct CachedGameState g_cgs;
static uint64_t esp_now_ms() {
    return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

volatile int g_espStep = 0;
const char* g_espStepName = "frame start";
#define ESP_STEP(n, name) do { g_espStep = (n); g_espStepName = (name); } while(0)

INLINE void DrawESP(ImDrawList* draw) {
    if ((!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN) {
        ESP_STEP(1, "g_cgs.gameManager check (.so 0x5ce0ff8)");
        if (!g_cgs.gameManager) { esp_status("stop: gameManager null (gameUpdateHook not fired)"); return; }

        ESP_STEP(2, "UpdateScreenTable");
        UpdateScreenTable(Width, Height);

        ESP_STEP(3, "lobby check (.so: frame piece count == 0)");

        if (g_cgs.framePieceCount == 0 || !g_cgs.frameDataValid) {
            esp_status("lobby mode (overlayFrame returned no pieces) — AutoQueue branch");
            g_autoQueueEnabled = persistent_bool[O("bAutoQueue")];

            g_bridgeActive2 = g_autoQueueEnabled;
            if (persistent_bool[O("bAutoQueue")]) DrawAutoQueue();
            return;
        }

        ESP_STEP(4, "stateId from cache (no stateStack walk)");
        auto stateId = g_cgs.stateId;
        if (g_cgs.playerTurn && gPrediction) {

            if (g_CurrentCandidate.idx < 0 && g_cgs.aimValid && g_cgs.gameManager) {
                static double lastEspScan = 0.0;
                double now = ImGui::GetTime();
                if (now - lastEspScan > 1.5) {
                    lastEspScan = now;
                    AutoPlay::ScanFast();
                }
            }
            if (g_CurrentCandidate.idx >= 0) {

                static int lastCandIdx = -1;
                static double lastRenderT = 0.0;

                double nowR = ImGui::GetTime();

                g_GameTrScreenH = (double)ImGui::GetIO().DisplaySize.y;
                static double lastAimAng = -99.0;
                static double s_liveAng = -100.0, s_livePow = 0.5;
                bool dragging = g_AimTouchActive && g_GameTrValid && g_cgs.aimValid;
                if (dragging) {
                    double ox = g_GameTrOrigin[0], oy = g_GameTrOrigin[1];
                    double ax = g_GameTrVecX[0],  ay = g_GameTrVecX[1];
                    double bx = g_GameTrVecY[0],  by = g_GameTrVecY[1];
                    double det = ax*by - ay*bx;
                    if (det != 0.0) {

                        double dx = (double)g_AimTouchX - ox;
                        double dy = (g_GameTrScreenH - (double)g_AimTouchY) - oy;
                        double fw_x = (dx*by - dy*bx) / det;
                        double fw_y = (dy*ax - dx*ay) / det;
                        Vec2d strikerW(g_cgs.aimAngle, g_cgs.aimPower);
                        double ddx = strikerW.x - fw_x, ddy = strikerW.y - fw_y;
                        double ang = std::atan2(ddy, ddx);

        ImVec2 spx = WorldToScreen(strikerW);
        double pdx = (double)g_AimTouchX - spx.x;
        double pdy = (double)g_AimTouchY - spx.y;
        double dist = std::sqrt(pdx*pdx + pdy*pdy);
                        if (std::fabs(ang - lastAimAng) > 0.01 || nowR - lastRenderT > 0.25) {
                            lastAimAng = ang; lastRenderT = nowR;
                            float ph = ImGui::GetIO().DisplaySize.y;
                            if (ph <= 0.f) ph = 1080.f;
                            double ratio = dist / (0.30 * (double)ph);
                            if (ratio < 0.10) ratio = 0.10;
                            if (ratio > 1.0)  ratio = 1.0;
                            s_liveAng = ang; s_livePow = ratio;
                            s_simWorker.submit(ang, ratio);
                        }
                    }
                } else if (g_CurrentCandidate.idx != -1 &&
                           (g_CurrentCandidate.idx != lastCandIdx || nowR - lastRenderT > 0.5)) {
                    lastAimAng = -99.0;
                    lastCandIdx = g_CurrentCandidate.idx;
                    lastRenderT = nowR;
                    s_liveAng = g_CurrentCandidate.angle; s_livePow = g_CurrentCandidate.power;
                    s_simWorker.submit(g_CurrentCandidate.angle, g_CurrentCandidate.power);
                } else if (nowR - lastRenderT > 0.5) {

                    if (s_liveAng < -90.0) {
                        Vec2d sw2(g_cgs.aimAngle, g_cgs.aimPower);
                        s_liveAng = std::atan2(0.0 - sw2.y, 0.0 - sw2.x);
                        s_livePow = 0.5;
                    }
                    lastRenderT = nowR;
                    s_simWorker.submit(s_liveAng, s_livePow);
                }
            }
        }
        if (!g_cgs.playerTurn) {
            esp_status_id("wait: stateId= (rolling/opponent turn)", stateId);
            return;
        }
        if (gPrediction && gPrediction->guiData.piecesCount == 0 && !g_cgs.playerTurn) {
            esp_status_id("no prediction data: stateId=", stateId);
        }

        ESP_STEP(11, "mBoard from cache (g_cgs.board) — .so 0x5ce0ff8+8");
        Board table = g_cgs.board;
        if (!table) { esp_status("stop: board null (cached from gameUpdateHook)"); return; }

        ESP_STEP(14, "mBoardProperties read");
        auto tableProperties = table.mBoardProperties();

        if (!tableProperties) { esp_status("DRAWING (boardProps null — pocket circles off)"); }

        ESP_STEP(15, "mPockets read");
        auto& pockets = tableProperties.mPockets();

        ESP_STEP(19, "ESP draw loops");
        esp_status_id("DRAWING ESP stateId=", stateId);
        g_GameTrScreenH = (double)ImGui::GetIO().DisplaySize.y;
        esp_status_id("TRF valid=", g_GameTrValid);

        if (persistent_bool[O("bESP_DrawPredictionLine")]) {

            std::lock_guard<std::mutex> s_lk(s_simWorker.mtx);
            int s_n = s_simWorker.snapCount;
            for (int i = 0; i < s_n; i++) {
                auto& piece = s_simWorker.snapPieces[i];

                if (piece.initialPosition != piece.predictedPosition && piece.positions.size() > 1) {

    float lineThick = (float)persistent_int[O("iLineThickness")];
                    float lineAlpha = persistent_float[O("fLineOpacity")];
                    if (lineThick < 1.f) lineThick = 1.f;
                    if (lineAlpha <= 0.f) lineAlpha = 1.f;
                    if (lineAlpha > 1.f) lineAlpha = 1.f;

                    ImColor lineColor;
                    if (g_refEspLineStyle == 0) {

                        lineColor = (i == 0)
                            ? ImColor(1.0f, 0.84f, 0.0f, lineAlpha)
                            : ImColor(0.30f, 0.95f, 1.0f, lineAlpha);
                    } else {

                        lineColor = ImColor(0.18f, 0.72f, 1.0f, lineAlpha);
                    }

                    if (i == 0) lineThick += 1.0f;

                    auto clampBoard = [](ImVec2 p) {
                        float maxX = ImGui::GetIO().DisplaySize.x;
                        float maxY = ImGui::GetIO().DisplaySize.y;
                        if (p.x < 0) p.x = 0; if (p.x > maxX) p.x = maxX;
                        if (p.y < 0) p.y = 0; if (p.y > maxY) p.y = maxY;
                        return p;
                    };
                    ImVec2 firstPos = clampBoard(WorldToScreen(piece.positions[0]));
                    if (i == 0) {

                        auto drawDotted = [&](ImVec2 a, ImVec2 b) {
                            ImVec2 d(b.x - a.x, b.y - a.y);
                            float len = sqrtf(d.x * d.x + d.y * d.y);
                            if (len < 2.f) return;
                            d.x /= len; d.y /= len;
                            float t = 0.f; bool on = true;
                            while (t < len) {
                                float step = on ? 8.f : 7.f;
                                if (t + step > len) step = len - t;
                                if (on)
                                    draw->AddLine(ImVec2(a.x + d.x * t, a.y + d.y * t),
                                                  ImVec2(a.x + d.x * (t + step), a.y + d.y * (t + step)),
                                                  lineColor, lineThick * 0.8f);
                                t += step; on = !on;
                            }
                        };
                        ImVec2 lastPos = firstPos;
                        for (int j = 1; j < (int)piece.positions.size(); j++) {
                            auto point = clampBoard(WorldToScreen(piece.positions[j]));
                            if (j == 1) draw->AddLine(lastPos, point, lineColor, lineThick);
                            else        drawDotted(lastPos, point);
                            lastPos = point;
                        }
                        int pn = (int)piece.positions.size();
                        if (pn >= 2) {
                            Vec2d a = piece.positions[pn - 2], b = piece.positions[pn - 1];
                            Vec2d dirv(b.x - a.x, b.y - a.y);
                            double dl = std::sqrt(dirv.x * dirv.x + dirv.y * dirv.y);
                            if (dl > 1e-6) {
                                dirv.x /= dl; dirv.y /= dl;
                                Vec2d ext(b.x + dirv.x * 80.0, b.y + dirv.y * 80.0);
                                drawDotted(lastPos, clampBoard(WorldToScreen(ext)));
                            }
                        }
                        if (pn >= 2) {
                            ImVec2 p1 = clampBoard(WorldToScreen(piece.positions[1]));
                            ImVec2 d(p1.x - firstPos.x, p1.y - firstPos.y);
                            float dl2 = sqrtf(d.x * d.x + d.y * d.y);
                            if (dl2 > 4.f) {
                                d.x /= dl2; d.y /= dl2;
                                ImVec2 tip(firstPos.x + d.x * 44.f, firstPos.y + d.y * 44.f);
                                ImVec2 n(-d.y, d.x);
                                draw->AddTriangleFilled(tip,
                                    ImVec2(tip.x - d.x * 14.f + n.x * 6.f, tip.y - d.y * 14.f + n.y * 6.f),
                                    ImVec2(tip.x - d.x * 14.f - n.x * 6.f, tip.y - d.y * 14.f - n.y * 6.f),
                                    lineColor);
                            }
                        }
                    } else {

                        ImVec2 lastPos = firstPos;
                        for (int j = 1; j < (int)piece.positions.size(); j++) {
                            auto point = clampBoard(WorldToScreen(piece.positions[j]));
                            draw->AddLine(lastPos, point, lineColor, lineThick);
                            draw->AddCircle(point, lineThick + 2.0f, lineColor, 10, 1.6f);
                            lastPos = point;
                        }
                    }

                    if (i != 0) {
                        int pn = (int)piece.positions.size();
                        draw->AddCircle(clampBoard(WorldToScreen(piece.positions[0])),
                                        lineThick + 2.0f, lineColor, 12, 2.0f);
                        draw->AddCircle(clampBoard(WorldToScreen(piece.positions[pn-1])),
                                        lineThick + 2.0f, lineColor, 12, 2.0f);
                    }
                }
            }
        }

        if (persistent_bool[O("bESP_DrawPredictionLine")]) {
            for (int i = 0; i < gPrediction->guiData.piecesCount; i++) {
                auto& piece = gPrediction->guiData.pieces[i];

                if (piece.initialPosition != piece.predictedPosition) {
                    float circleR = g_refLineThickness + 1.f;
                    if (circleR < 2.f) circleR = 2.f;

                    ImColor yellow(1.0f, 0.84f, 0.0f, 1.0f);
                    ImColor white(1.0f, 1.0f, 1.0f, 0.7f);
                    draw->AddCircleFilled(WorldToScreen(piece.initialPosition), circleR, yellow);
                    draw->AddCircleFilled(WorldToScreen(piece.predictedPosition), 16, white);
                }
            }
        }
    } else {

        esp_status("stop: AUTH GATE CLOSED — keylogin not passed (g_Token/g_Auth empty or mismatch). Check KEYLOGIN line in Bug.txt");
    }
}

static void DrawSidebar(float sidebarW) {
    static GLuint draw_icon_tex = LoadTextureFromMemory(draw_icon_png, draw_icon_png_len);
    static GLuint play_icon_tex = LoadTextureFromMemory(play_icon_png, play_icon_png_len);
    static GLuint q_icon_tex    = LoadTextureFromMemory(q_icon_png,    q_icon_png_len);
    static GLuint user_icon_tex = LoadTextureFromMemory(user_icon_png, user_icon_png_len);

    ImGuiContext& g  = *GImGui;
    ImDrawList*   dl = GetWindowDrawList();
    ImVec2        wp = GetWindowPos();

    float closeSize = 35.0f;
    float closeBtnW = 70.0f;
    float tabsW     = sidebarW - closeBtnW;
    float btnW      = tabsW / 4.0f;
    float marginB   = 12.0f;

    dl->ChannelsSplit(2);
    dl->ChannelsSetCurrent(1);

    BeginGroup();
    SetCursorPos(ImVec2(0.0f, 0.0f));

    float btnW3 = tabsW / 3.0f;
    if (SidebarButton(O("FUNCTIONS"),  draw_icon_tex, g_menu.currentTab == 0, btnW3)) g_menu.currentTab = 0;
    SameLine(0, 0);
    if (SidebarButton(O("AUTO MATCH"), play_icon_tex, g_menu.currentTab == 1, btnW3)) g_menu.currentTab = 1;
    SameLine(0, 0);
    if (SidebarButton(O("DEBUG"),  q_icon_tex, g_menu.currentTab == 2, btnW3)) g_menu.currentTab = 2;
    EndGroup();

    float sidebarH = GetItemRectMax().y - wp.y;

    dl->ChannelsSetCurrent(0);
    dl->AddRectFilled(wp, ImVec2(wp.x + sidebarW, wp.y + sidebarH), IM_COL32(26, 26, 26, 245), 18.0f);
    dl->AddRect(wp, ImVec2(wp.x + sidebarW, wp.y + sidebarH), IM_COL32(255, 215, 0, 220), 18.0f, 0, 1.5f);
    dl->ChannelsMerge();

    float sepX       = wp.x + sidebarW - closeBtnW;
    float sepCenterY = wp.y + sidebarH * 0.5f;
    float sepHalfH   = sidebarH * 0.28f;
    dl->AddLine(
        ImVec2(sepX, sepCenterY - sepHalfH),
        ImVec2(sepX, sepCenterY + sepHalfH),
        IM_COL32(236, 196, 105, 180), 1.5f
    );

    float closePosX = (sidebarW - closeBtnW) + (closeBtnW - closeSize) * 0.5f;
    float closePosY = (sidebarH - closeSize) * 0.5f;
    SetCursorPos(ImVec2(closePosX, closePosY));
    {
        ImGuiWindow* win = GetCurrentWindow();
        ImGuiID closeId  = win->GetID(O("##CloseMenu"));
        ImVec2 closePos  = win->DC.CursorPos;
        ImRect closeBb(closePos, closePos + ImVec2(closeSize, closeSize));
        ItemSize(ImVec2(closeSize, closeSize), g.Style.FramePadding.y);
        ItemAdd(closeBb, closeId);
        bool closeHovered = false, closeHeld = false;
        bool closePressed = ButtonBehavior(closeBb, closeId, &closeHovered, &closeHeld);
        if (closePressed) g_menu.isOpen = false;

        float xCX = closeBb.Min.x + closeSize * 0.5f;
        float xCY = closeBb.Min.y + closeSize * 0.5f;
        float xH  = closeSize * 0.32f;
        ImU32 xCol = closeHovered ? IM_COL32(255, 255, 255, 240) : IM_COL32(160, 160, 170, 200);
        dl->AddLine(ImVec2(xCX - xH, xCY - xH), ImVec2(xCX + xH, xCY + xH), xCol, 2.2f);
        dl->AddLine(ImVec2(xCX + xH, xCY - xH), ImVec2(xCX - xH, xCY + xH), xCol, 2.2f);
    }

    SetCursorPos(ImVec2(0.0f, sidebarH));
    Dummy(ImVec2(sidebarW, marginB));
}

static std::string ReadNSString(ptr str) {
    if (!str) return "null";
    int32_t len = F(int32_t, str + 0x10);
    if (len <= 0 || len > 512) return "?";
    std::string result;
    result.reserve(len);
    for (int32_t i = 0; i < len; i++) {
        uint16_t ch = F(uint16_t, str + 0x14 + i * 2);
        result += (ch > 0 && ch < 128) ? (char)ch : '?';
    }
    return result;
}

static float g_menuX = 108.0f;
static float g_menuY = 435.0f;
static float g_menuW = 540.0f;
static float g_menuH = 560.0f;

static float g_playBtnX = 54.0f;
static float g_playBtnY = 252.0f;
static float g_playBtnSize = 90.0f;

static float g_logoBtnX = 48.0f;
static float g_logoBtnY = 128.0f;
static float g_logoBtnSize = 100.0f;

static float g_toggleRotAngle = 0.0f;

static bool  g_autoPlayCalculating = false;

static void svConfig_Save() {
    std::string path = O("/data/user/0/") + PACKAGE_NAME + O("/files/svConfig.txt");
    FILE* f = fopen(path.c_str(), O("w"));
    if (!f) return;
    fprintf(f, O("iLineThickness=%d\n"),  persistent_int[O("iLineThickness")]);
    fprintf(f, O("iMenuSizeOffset=%d\n"), persistent_int[O("iMenuSizeOffset")]);
    fclose(f);
}
static void svConfig_Load() {
    std::string path = O("/data/user/0/") + PACKAGE_NAME + O("/files/svConfig.txt");
    FILE* f = fopen(path.c_str(), O("r"));
    if (!f) return;
    char line[64];
    while (fgets(line, sizeof(line), f)) {
        int v = 0;
        if (sscanf(line, O("iLineThickness=%d"),  &v) == 1) { persistent_int[O("iLineThickness")]  = v; continue; }
        if (sscanf(line, O("iMenuSizeOffset=%d"), &v) == 1) { persistent_int[O("iMenuSizeOffset")] = v; }
    }
    fclose(f);
}

static void DrawCalculating(ImGuiIO& io) {

    SetNextWindowPos(ImVec2(Width * 0.5f, Height * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    PushStyleColor(ImGuiCol_WindowBg, IM_COL32(18, 18, 18, 255));
    PushStyleColor(ImGuiCol_Border, IM_COL32(220, 30, 30, 255));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 18.0f);
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

    if (Begin(O("##CalcOverlay"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
              ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs)) {

        SetWindowFontScale(1.4f);
        TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), O("CALCULATING..."));
        SetWindowFontScale(1.0f);
    }
    End();
    PopStyleVar(2);
    PopStyleColor(2);
}

static void DrawContentArea(float winW, float winH) {
    bool need_save = false;
    ImDrawList* dl = GetWindowDrawList();
    ImVec2 wp = GetWindowPos();
    const float s = winW / 540.0f;

    const float headerH = 48.0f * s;
    const float footerH = 54.0f * s;
    const float padX = 14.0f * s;
    const float contentTop = 64.0f * s;
    const float footerY = winH - footerH;
    const float contentW = winW - padX * 2.0f;
    const ImFont* font = GetFont();
    const float textPx = 14.0f * s;
    const float titlePx = 13.0f * s;
    const float tabPx = 14.0f * s;
    const float mutedPx = 13.0f * s;

    auto TextAt = [&](const char* text, ImVec2 pos, ImU32 color, float px) {
        dl->AddText(font, px, pos, color, text);
    };
    auto CenterText = [&](const char* text, ImVec2 center, ImU32 color, float px) {
        ImVec2 ts = font->CalcTextSizeA(px, FLT_MAX, 0.0f, text);
        dl->AddText(font, px, ImVec2(center.x - ts.x * 0.5f, center.y - ts.y * 0.5f), color, text);
    };

    dl->AddLine(ImVec2(wp.x + padX, wp.y + headerH),
                ImVec2(wp.x + winW - padX, wp.y + headerH),
                IM_COL32(90, 72, 18, 180), 1.0f * s);
    TextAt(O("@GVM_TRUST - GALVNIC ENGINE"), ImVec2(wp.x + padX, wp.y + 14.0f*s),
           IM_COL32(255, 215, 0, 255), titlePx);

    float cy = contentTop;
    const float rowH = 40.0f * s;
    const float rowGap = 7.0f * s;
    const float rowR = 7.0f * s;

    auto DrawRefToggle = [&](const char* idText, const char* label, bool* value) {
        ImGuiWindow* w = GetCurrentWindow();
        ImGuiID id = w->GetID(idText);
        ImVec2 a(wp.x + padX, wp.y + cy);
        ImVec2 b(a.x + contentW, a.y + rowH);
        ImRect bb(a, b);
        ItemSize(ImVec2(contentW, rowH));
        if (ItemAdd(bb, id)) {
            bool hovered=false, held=false;
            if (ButtonBehavior(bb, id, &hovered, &held)) {
                *value = !*value;
                need_save = true;
            }
            dl->AddRectFilled(a, b,
                hovered ? IM_COL32(31,31,37,255) : IM_COL32(24,24,29,255), rowR);
            dl->AddRect(a, b, IM_COL32(70,65,27,220), rowR, 0, 1.0f*s);

            ImVec2 ts = font->CalcTextSizeA(textPx, FLT_MAX, 0.0f, label);
            TextAt(label, ImVec2(a.x + 10.0f*s, a.y + (rowH-ts.y)*0.5f),
                   IM_COL32(225,225,230,255), textPx);

            const float tw = 46.0f*s;
            const float th = 24.0f*s;
            const float r = th*0.5f;
            ImVec2 ta(b.x-tw-9.0f*s, a.y+(rowH-th)*0.5f);
            ImVec2 tb(ta.x+tw, ta.y+th);
            dl->AddRectFilled(ta, tb, *value ? UI_GOLD : IM_COL32(61,63,73,255), r);
            dl->AddRect(ta, tb, IM_COL32(48,48,54,180), r, 0, 1.0f*s);
            dl->AddCircleFilled(ImVec2(*value ? tb.x-r : ta.x+r, ta.y+r),
                                8.0f*s, IM_COL32(250,250,250,255));
        }
        cy += rowH + rowGap;
    };

    auto DrawRefSlider = [&](const char* label, const char* id, float* value,
                             float minV, float maxV, bool integerValue) {

        TextAt(label, ImVec2(wp.x + padX, wp.y + cy + 1.0f*s),
               IM_COL32(220,220,225,255), textPx);
        cy += 25.0f*s;

        ImGuiWindow* w = GetCurrentWindow();
        ImGuiID iid = w->GetID(id);
        ImVec2 a(wp.x + padX, wp.y + cy);
        ImVec2 b(a.x + contentW, a.y + 32.0f*s);
        ImRect bb(a,b);
        ItemSize(ImVec2(contentW,32.0f*s));
        if(ItemAdd(bb,iid)) {
            bool hovered=false,held=false;
            ButtonBehavior(bb,iid,&hovered,&held);
            float t = integerValue ?
                ((*((int*)value) - minV) / (maxV-minV)) :
                ((*value-minV)/(maxV-minV));
            t = ImClamp(t,0.0f,1.0f);
            if(held) {
                float nt = ImClamp((GetIO().MousePos.x-bb.Min.x)/bb.GetWidth(),0.0f,1.0f);
                if(integerValue) *((int*)value) = (int)lroundf(minV + nt*(maxV-minV));
                else *value = minV + nt*(maxV-minV);
                need_save = true;
                t = nt;
            }
            dl->AddRectFilled(a,b,IM_COL32(23,23,29,255),8.0f*s);
            dl->AddRect(a,b,IM_COL32(55,53,62,220),8.0f*s,0,1.0f*s);
            const float thumbX = a.x + t * bb.GetWidth();
            dl->AddRectFilled(ImVec2(thumbX-5.0f*s,a.y+4.0f*s),
                              ImVec2(thumbX+5.0f*s,b.y-4.0f*s), UI_GOLD, 5.0f*s);
            char valueText[32];
            if(integerValue) snprintf(valueText,sizeof(valueText),"%d",*((int*)value));
            else snprintf(valueText,sizeof(valueText),"%.1f",*value);
            CenterText(valueText, ImVec2((a.x+b.x)*0.5f,(a.y+b.y)*0.5f),
                       IM_COL32(205,205,210,255), mutedPx);
        }
        cy += 44.0f*s;
    };

    if (g_menu.currentTab == 0) {
        DrawRefToggle("##enablePrediction", "Enable Prediction", &persistent_bool[O("bESP_DrawPredictionLine")]);
        DrawRefToggle("##strikerPath", "Show Striker Path", &persistent_bool[O("bESP_DrawPocketsShotState")]);
        DrawRefToggle("##puckPaths", "Show Puck Paths", &persistent_bool[O("bESP_DrawPockets")]);
        DrawRefToggle("##predictionAfterShot", "Prediction After Shot", &persistent_bool[O("bESP_DrawPredictionAfterShot")]);
        DrawRefToggle("##autoPlay", "Auto Play", &persistent_bool[O("bAutoPlay")]);

        if (persistent_bool[O("bAutoPlay")]) {
            TextAt(O("Auto Play Mode"), ImVec2(wp.x + padX, wp.y + cy + 1.0f*s),
                   IM_COL32(255, 215, 0, 255), textPx);
            cy += 27.0f*s;

            const float modeGap = 8.0f*s;
            const float modeW = (contentW - modeGap) * 0.5f;
            const float modeH = 48.0f*s;
            for (int i = 0; i < 2; ++i) {
                const float x = wp.x + padX + i*(modeW + modeGap);
                ImVec2 a(x, wp.y + cy), b(x + modeW, wp.y + cy + modeH);
                ImGuiWindow* w = GetCurrentWindow();
                const char* id = i == 0 ? "##refFastMode" : "##refHumanMode";
                const char* label = i == 0 ? "FAST" : "HUMAN";
                ImGuiID iid = w->GetID(id);
                ImRect bb(a,b);
                ItemSize(ImVec2(modeW, modeH));
                if (ItemAdd(bb, iid)) {
                    bool hovered=false, held=false;
                    if (ButtonBehavior(bb, iid, &hovered, &held)) {
                        g_refAutoPlayMode = i;

                        AutoPlay::scan = (i == 0) ? AutoPlay::FAST : AutoPlay::SLOW;
                        need_save = true;
                    }
                    const bool selected = g_refAutoPlayMode == i;
                    dl->AddRectFilled(a, b, selected ? UI_GOLD : IM_COL32(22,22,27,255), 8.0f*s);
                    dl->AddRect(a, b, selected ? IM_COL32(255,220,35,255) : IM_COL32(55,53,62,230), 8.0f*s, 0, 1.0f*s);
                    CenterText(label, ImVec2((a.x+b.x)*0.5f,(a.y+b.y)*0.5f),
                               selected ? IM_COL32(20,20,20,255) : IM_COL32(215,215,220,255), textPx);
                }
            }
            cy += modeH + 9.0f*s;
        }

        DrawRefToggle("##cleanTable", "Clean Table", &persistent_bool[O("bCleanTable")]);

    } else if (g_menu.currentTab == 1) {
        DrawRefToggle("##autoMatchQueue", "Auto Match Queue", &persistent_bool[O("bAutoQueue")]);
        cy += 18.0f*s;

        TextAt(O("Select Game Mode"), ImVec2(wp.x + padX, wp.y + cy + 1.0f*s),
               IM_COL32(255, 215, 0, 255), textPx);
        cy += 30.0f*s;

        struct Mode { const char* label; int value; };
        static const Mode modes[] = { {"Carrom",0}, {"Disc Pool",1}, {"Freestyle",2} };
        int& selectedMode = persistent_int[O("iAutoQueue_FixTable")];
        if(selectedMode < 0 || selectedMode > 2) selectedMode = 0;

        const float gapX=10.0f*s;
        const float bw=(contentW-gapX*2.0f)/3.0f;
        const float bh=52.0f*s;
        for(int i=0;i<3;i++) {
            ImVec2 a(wp.x + padX + i*(bw+gapX), wp.y + cy);
            ImVec2 b(a.x+bw, a.y+bh);
            ImGuiWindow* w=GetCurrentWindow();
            ImGuiID id=w->GetID(modes[i].label);
            ImRect bb(a,b);
            ItemSize(ImVec2(bw,bh));
            if(ItemAdd(bb,id)) {
                bool hovered=false,held=false;
                if(ButtonBehavior(bb,id,&hovered,&held)) {
                    selectedMode=i;
                    int& coinIndex = persistent_int[O("iAutoQueue_CoinIndex")];
                    const int coinCount = (i == 2) ? 9 : 12;
                    if(coinIndex < 0 || coinIndex >= coinCount) coinIndex = 0;
                    need_save=true;
                }
                dl->AddRectFilled(a,b,selectedMode==i ? UI_GOLD : IM_COL32(22,22,27,255),9.0f*s);
                dl->AddRect(a,b,IM_COL32(55,53,62,230),9.0f*s,0,1.0f*s);
                CenterText(modes[i].label, ImVec2((a.x+b.x)*0.5f,(a.y+b.y)*0.5f),
                           selectedMode==i ? IM_COL32(20,20,20,255) : IM_COL32(210,210,215,255), textPx);
            }
        }
        cy += bh + 18.0f*s;

        TextAt(O("Select Coin"), ImVec2(wp.x + padX, wp.y + cy + 1.0f*s),
               IM_COL32(255, 215, 0, 255), textPx);
        cy += 27.0f*s;

        static const char* coin12[] = {
            "200", "500", "2K", "4K", "5K", "11K", "27K", "55K",
            "140K", "170K", "280K", "580K"
        };
        static const char* coin9[] = {
            "200", "500", "2K", "5K", "11K", "55K", "140K", "280K", "580K"
        };
        const char* const* coins = (selectedMode == 2) ? coin9 : coin12;
        const int coinCount = (selectedMode == 2) ? 9 : 12;
        int& selectedCoin = persistent_int[O("iAutoQueue_CoinIndex")];
        if(selectedCoin < 0 || selectedCoin >= coinCount) selectedCoin = 0;

        const float coinGapX = 10.0f*s;
        const float coinGapY = 5.0f*s;
        const float coinW = (contentW - coinGapX*3.0f) / 4.0f;
        const float coinH = 41.0f*s;
        for(int i=0;i<coinCount;i++) {
            const int col = i % 4;
            const int row = i / 4;
            ImVec2 a(wp.x + padX + col*(coinW+coinGapX), wp.y + cy + row*(coinH+coinGapY));
            ImVec2 b(a.x+coinW, a.y+coinH);
            ImGuiWindow* w=GetCurrentWindow();
            ImGuiID id=w->GetID(coins[i]);
            ImRect bb(a,b);
            ItemSize(ImVec2(coinW,coinH));
            if(ItemAdd(bb,id)) {
                bool hovered=false,held=false;
                if(ButtonBehavior(bb,id,&hovered,&held)) {
                    selectedCoin=i;
                    need_save=true;
                }
                const bool selected = selectedCoin == i;
                dl->AddRectFilled(a,b, selected ? UI_GOLD : IM_COL32(22,22,27,255), 7.0f*s);
                dl->AddRect(a,b, selected ? IM_COL32(255,220,20,255) : IM_COL32(55,53,62,230), 7.0f*s,0,1.0f*s);
                CenterText(coins[i], ImVec2((a.x+b.x)*0.5f,(a.y+b.y)*0.5f),
                           selected ? IM_COL32(20,20,20,255) : IM_COL32(210,210,215,255), textPx);
            }
        }
        const int coinRows = (coinCount + 3) / 4;
        cy += coinRows*(coinH+coinGapY) - coinGapY + 21.0f*s;

        {
            ImGuiWindow* w=GetCurrentWindow();
            ImGuiID id=w->GetID(O("##startMatch"));
            const float startH = 52.0f*s;
            ImVec2 a(wp.x + padX, wp.y + cy);
            ImVec2 b(a.x + contentW, a.y + startH);
            ImRect bb(a,b);
            ItemSize(ImVec2(contentW,startH));
            if(ItemAdd(bb,id)) {
                bool hovered=false,held=false;
                const bool pressed=ButtonBehavior(bb,id,&hovered,&held);
                dl->AddRectFilled(a,b,UI_GOLD,9.0f*s);
                dl->AddRect(a,b,IM_COL32(255,228,80,255),9.0f*s,0,1.0f*s);
                CenterText(O("START MATCH"), ImVec2((a.x+b.x)*0.5f,(a.y+b.y)*0.5f),
                           IM_COL32(20,20,20,255), textPx);
                if(pressed) {
                    persistent_bool[O("bAutoQueue")] = true;
                    need_save = true;
                }
            }
            cy += startH;
        }

    } else {

        TextAt(O("Select ESP Style"), ImVec2(wp.x + padX, wp.y + cy + 1.0f*s),
               IM_COL32(255, 215, 0, 255), textPx);
        cy += 29.0f*s;

        const float styleGap = 10.0f*s;
        const float styleW = (contentW - styleGap) * 0.5f;
        const float styleH = 88.0f*s;
        const float previewPad = 10.0f*s;
        const char* styleNames[2] = { "Classic Mode", "Sea Blue Glow" };

        for (int i=0; i<2; ++i) {
            const float x = wp.x + padX + i*(styleW + styleGap);
            ImVec2 a(x, wp.y + cy), b(x + styleW, wp.y + cy + styleH);
            ImGuiWindow* w = GetCurrentWindow();
            ImGuiID id = w->GetID(i == 0 ? "##classicLineStyle" : "##blueLineStyle");
            ImRect bb(a,b);
            ItemSize(ImVec2(styleW,styleH));
            if(ItemAdd(bb,id)) {
                bool hovered=false,held=false;
                if(ButtonBehavior(bb,id,&hovered,&held)) {
                    g_refEspLineStyle = i;
                    need_save = true;
                }
                const bool selected = g_refEspLineStyle == i;
                dl->AddRectFilled(a,b,IM_COL32(18,18,23,255),9.0f*s);
                dl->AddRect(a,b,selected ? UI_GOLD : IM_COL32(55,53,62,230),9.0f*s,0,selected ? 2.0f*s : 1.0f*s);

                const float left = a.x + previewPad;
                const float right = b.x - previewPad;
                const float mid = a.y + 31.0f*s;
                const float span = right-left;
                const float phase = fmodf(g_refLineAnim * 0.12f, 1.0f);
                ImVec2 prev(left, mid + 4.0f*s);
                ImVec2 ballPos = prev;
                for (int k=1; k<=28; ++k) {
                    const float t = (float)k/28.0f;
                    const float wave = sinf((t*3.2f + phase)*3.14159265f) * 5.0f*s;
                    ImVec2 cur(left + span*t, mid + wave);
                    ImU32 c;
                    if (i == 0) c = IM_COL32(255,215,0,(int)(255.0f*g_refLineOpacity));
                    else c = IM_COL32(40,185,255,(int)(255.0f*g_refLineOpacity));
                    dl->AddLine(prev, cur, c, ImMax(1.5f*s, g_refLineThickness*0.35f*s));
                    prev = cur;
                    ballPos = cur;
                }

                const ImU32 ballCol = (i == 0)
                    ? IM_COL32(255,235,70,(int)(255.0f*g_refLineOpacity))
                    : IM_COL32(70,205,255,(int)(255.0f*g_refLineOpacity));
                dl->AddCircleFilled(ballPos, 4.2f*s, ballCol, 20);
                dl->AddCircle(ballPos, 6.0f*s, IM_COL32(255,255,255,(int)(110.0f*g_refLineOpacity)), 20, 1.0f*s);
                CenterText(styleNames[i], ImVec2((a.x+b.x)*0.5f, b.y-17.0f*s),
                           selected ? IM_COL32(235,235,235,255) : IM_COL32(185,185,190,255), mutedPx);
            }
        }
        cy += styleH + 15.0f*s;

        TextAt(O("Line Thickness"), ImVec2(wp.x + padX, wp.y + cy + 1.0f*s),
               IM_COL32(220,220,225,255), textPx);
        cy += 24.0f*s;
        {
            ImGuiWindow* w = GetCurrentWindow();
            ImGuiID id = w->GetID("##refLineThickness");
            ImVec2 a(wp.x + padX, wp.y + cy), b(a.x + contentW, a.y + 30.0f*s);
            ImRect bb(a,b);
            ItemSize(ImVec2(contentW,30.0f*s));
            if(ItemAdd(bb,id)) {
                bool hovered=false,held=false;
                ButtonBehavior(bb,id,&hovered,&held);
                if(held) {
                    g_refLineThickness = ImClamp(1.0f + 9.0f*((GetIO().MousePos.x-a.x)/bb.GetWidth()),1.0f,10.0f);
                    need_save = true;
                }
                float t=(g_refLineThickness-1.0f)/9.0f;
                dl->AddRectFilled(a,b,IM_COL32(23,23,29,255),7.0f*s);
                dl->AddRect(a,b,IM_COL32(55,53,62,220),7.0f*s,0,1.0f*s);
                const float x=a.x+t*bb.GetWidth();
                dl->AddRectFilled(ImVec2(x-4.0f*s,a.y+4.0f*s),ImVec2(x+4.0f*s,b.y-4.0f*s),UI_GOLD,4.0f*s);
                char v[24]; snprintf(v,sizeof(v),"%.1f",g_refLineThickness);
                CenterText(v,ImVec2((a.x+b.x)*0.5f,(a.y+b.y)*0.5f),IM_COL32(205,205,210,255),mutedPx);
            }
        }
        cy += 40.0f*s;

        TextAt(O("Line Opacity"), ImVec2(wp.x + padX, wp.y + cy + 1.0f*s),
               IM_COL32(220,220,225,255), textPx);
        cy += 24.0f*s;
        {
            ImGuiWindow* w = GetCurrentWindow();
            ImGuiID id = w->GetID("##refLineOpacity");
            ImVec2 a(wp.x + padX, wp.y + cy), b(a.x + contentW, a.y + 30.0f*s);
            ImRect bb(a,b);
            ItemSize(ImVec2(contentW,30.0f*s));
            if(ItemAdd(bb,id)) {
                bool hovered=false,held=false;
                ButtonBehavior(bb,id,&hovered,&held);
                if(held) {
                    g_refLineOpacity = ImClamp(0.1f + 0.9f*((GetIO().MousePos.x-a.x)/bb.GetWidth()),0.1f,1.0f);
                    need_save = true;
                }
                float t=(g_refLineOpacity-0.1f)/0.9f;
                dl->AddRectFilled(a,b,IM_COL32(23,23,29,255),7.0f*s);
                dl->AddRect(a,b,IM_COL32(55,53,62,220),7.0f*s,0,1.0f*s);
                const float x=a.x+t*bb.GetWidth();
                dl->AddRectFilled(ImVec2(x-4.0f*s,a.y+4.0f*s),ImVec2(x+4.0f*s,b.y-4.0f*s),UI_GOLD,4.0f*s);
                char v[24]; snprintf(v,sizeof(v),"%.1f",g_refLineOpacity);
                CenterText(v,ImVec2((a.x+b.x)*0.5f,(a.y+b.y)*0.5f),IM_COL32(205,205,210,255),mutedPx);
            }
        }
        g_refLineAnim += GetIO().DeltaTime * 2.0f;
    }

    const float tabW = winW/3.0f;
    auto DrawTab = [&](const char* id, const char* label, float x, bool selected) {
        ImGuiWindow* w=GetCurrentWindow();
        ImGuiID iid=w->GetID(id);
        ImVec2 a(wp.x+x,wp.y+footerY), b(wp.x+x+tabW,wp.y+winH);
        ImRect bb(a,b);
        ItemSize(ImVec2(tabW,footerH));
        if(!ItemAdd(bb,iid)) return;
        bool hovered=false,held=false;
        bool pressed=ButtonBehavior(bb,iid,&hovered,&held);
        if(selected) dl->AddRectFilled(a,b,UI_GOLD,8.0f*s);
        else if(hovered) dl->AddRectFilled(a,b,IM_COL32(255,215,0,25),8.0f*s);
        CenterText(label, ImVec2((a.x+b.x)*0.5f,(a.y+b.y)*0.5f),
                   selected?IM_COL32(15,15,15,255):IM_COL32(220,220,225,255), tabPx);
        if(pressed) {
            if(strcmp(id,"##functionsTab")==0) g_menu.currentTab=0;
            else if(strcmp(id,"##debugTab")==0) g_menu.currentTab=2;
            else g_menu.currentTab=1;
        }
    };
    DrawTab("##functionsTab", O("FUNCTIONS"), 0, g_menu.currentTab==0);
    DrawTab("##autoMatchTab", O("AUTO MATCH"), tabW, g_menu.currentTab==1);
    DrawTab("##debugTab", O("ESP LINE"), tabW*2.0f, g_menu.currentTab==2);
    if(need_save) save_persistence();
}

INLINE void DrawMenu(ImGuiIO& io) {
    g_MenuConsumesTouch = g_menu.isOpen;
    if ((!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN) {

        jump_buffer_active = 1;
        g_espStep = 0; g_espStepName = "frame start";
        jump_buffer_active = 1;
        if (!sigsetjmp(jump_buffer, 1)) DrawESP(GetBackgroundDrawList());
        else {
            jump_buffer_active = 0;
            bug_log_write("ESP: SIGSEGV at step %d (%s) — recovering", g_espStep, g_espStepName);
            bug_log_flush();
        }
        jump_buffer_active = 0;
        jump_buffer_active = 0;

        if (g_cgs.gameManager) {
            jump_buffer_active = 1;
            if (sigsetjmp(jump_buffer, 1) == 0) {
                if (persistent_bool[O("bAutoPlay")]) AutoPlay::Update();
            } else {
                jump_buffer_active = 0;
                bug_log_write("AutoPlay: SIGSEGV caught — recovering");
            }
            jump_buffer_active = 0;

            jump_buffer_active = 1;
            if (sigsetjmp(jump_buffer, 1) == 0) {
                AutoAim::Draw();
            } else {
                jump_buffer_active = 0;
                bug_log_write("AutoAim: SIGSEGV caught — recovering");
            }
            jump_buffer_active = 0;
        }

        if (g_menu.isOpen) g_menu.menuAlpha += (1.0f-g_menu.menuAlpha)*io.DeltaTime*12.0f;
        else g_menu.menuAlpha=0.0f;

        if (g_menu.menuAlpha>0.01f) {
            const float s = ImMax(0.55f, Width/694.0f);
            const float winW=g_menuW*s;
            const float winH=g_menuH*s;

            float px=g_menuX*s;
            float py=g_menuY*s;

            SetNextWindowSize(ImVec2(winW,winH),ImGuiCond_Always);
            SetNextWindowPos(ImVec2(px,py),ImGuiCond_Always);
            PushStyleColor(ImGuiCol_WindowBg,ImVec4(0,0,0,0));
            PushStyleVar(ImGuiStyleVar_WindowRounding,18*s);
            PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
            PushStyleVar(ImGuiStyleVar_WindowBorderSize,0);
            PushStyleVar(ImGuiStyleVar_Alpha,g_menu.menuAlpha);

            ImGuiWindowFlags flags=ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar|
                ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove;
            if(Begin(O("##MainMenu"),&g_menu.isOpen,flags)) {
                ImDrawList* dl=GetWindowDrawList();
                ImVec2 a=GetWindowPos(), b(a.x+winW,a.y+winH);
                dl->AddRectFilled(a,b,IM_COL32(5,5,7,248),18*s);
                dl->AddRect(a,b,IM_COL32(112,82,15,255),18*s,0,2*s);
                dl->AddRect(ImVec2(a.x+3*s,a.y+3*s),ImVec2(b.x-3*s,b.y-3*s),IM_COL32(235,195,70,210),15*s,0,1*s);

                SetCursorPos(ImVec2(0,0));
                InvisibleButton(O("##MenuDrag"),ImVec2(winW,48*s));
                if(IsItemActive() && IsMouseDragging(ImGuiMouseButton_Left,2.0f)) {

                    g_menuX += io.MouseDelta.x/s;
                    g_menuY += io.MouseDelta.y/s;
                    if(g_menuX<0) g_menuX=0; if(g_menuY<0) g_menuY=0;
                    if(g_menuX>694-g_menuW) g_menuX=694-g_menuW;
                    if(g_menuY>1536-g_menuH) g_menuY=1536-g_menuH;
                }
                SetCursorPos(ImVec2(0,0));
                DrawContentArea(winW,winH);
            }
            End();
            PopStyleVar(4); PopStyleColor();
        }
    }
}

static void DrawToggleButton(bool cancelMode) {
    ImGuiIO& io=GetIO();
    static GLuint play_on_tex=LoadTextureFromMemory(play_on_png,play_on_png_len);
    static GLuint play_off_tex=LoadTextureFromMemory(play_off_png,play_off_png_len);

    const float s=ImMax(0.55f,io.DisplaySize.x/694.0f);
    const float buttonSize=g_playBtnSize*s;
    g_playBtnX=ImClamp(g_playBtnX,0.0f,694.0f-g_playBtnSize);
    g_playBtnY=ImClamp(g_playBtnY,0.0f,1536.0f-g_playBtnSize);
    const float px=g_playBtnX*s, py=g_playBtnY*s;

    const bool showModes = (!cancelMode && AutoPlay::bAutoPlaying);
    const float modeW=66.0f*s;
    const float modeH=30.0f*s;
    const float modeGap=0.0f;
    const float verticalGap=10.0f*s;
    const float modesW=modeW*2.0f+modeGap;
    const float windowW=ImMax(buttonSize,modesW);
    const float windowH=buttonSize+8.0f*s+(showModes?verticalGap+modeH:0.0f);

    SetNextWindowSize(ImVec2(windowW,windowH),ImGuiCond_Always);
    SetNextWindowPos(ImVec2(px-(windowW-buttonSize)*0.5f,py),ImGuiCond_Always);
    PushStyleColor(ImGuiCol_WindowBg,IM_COL32(0,0,0,0));
    PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
    if(Begin("##ToggleBtn",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBackground)) {
        ImDrawList* dl=GetWindowDrawList();
        const float playOffsetX=(windowW-buttonSize)*0.5f;
        ImVec2 pos(GetWindowPos().x+playOffsetX,GetWindowPos().y);

        SetCursorScreenPos(pos);
        InvisibleButton("##TglBtnHit",ImVec2(buttonSize,buttonSize));
        if(IsItemActive() && IsMouseDragging(ImGuiMouseButton_Left,2.0f)) {
            g_playBtnX+=io.MouseDelta.x/s;
            g_playBtnY+=io.MouseDelta.y/s;
        } else if(IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Left) &&
                  (GetMouseDragDelta(ImGuiMouseButton_Left).x*GetMouseDragDelta(ImGuiMouseButton_Left).x+
                   GetMouseDragDelta(ImGuiMouseButton_Left).y*GetMouseDragDelta(ImGuiMouseButton_Left).y)<9.0f) {
            if(cancelMode) {
                persistent_bool[O("bAutoQueue")]=false;
                g_aqCounting=false;
                g_aqLastCall=std::chrono::steady_clock::now();
            } else {
                AutoPlay::bAutoPlaying=!AutoPlay::bAutoPlaying;
                if(AutoPlay::bAutoPlaying) AutoPlay::ClearState();
            }
        }

        GLuint tex=(!cancelMode && AutoPlay::bAutoPlaying)?play_on_tex:play_off_tex;
        dl->AddImageRounded((void*)(intptr_t)tex,pos,ImVec2(pos.x+buttonSize,pos.y+buttonSize),ImVec2(0,0),ImVec2(1,1),IM_COL32_WHITE,buttonSize*0.5f);

        if(showModes) {
            const float bx=GetWindowPos().x+(windowW-modesW)*0.5f;
            const float by=GetWindowPos().y+buttonSize+verticalGap;
            for(int i=0;i<2;++i) {
                const float x=bx+i*(modeW+modeGap);
                ImVec2 a(x,by),b(x+modeW,by+modeH);
                const char* label=(i==0)?"FAST":"HUMAN";
                ImGuiID id=GetCurrentWindow()->GetID(i==0?"##attachedFast":"##attachedHuman");
                ImRect bb(a,b);
                ItemSize(ImVec2(modeW,modeH));
                if(ItemAdd(bb,id)) {
                    bool hovered=false,held=false;
                    if(ButtonBehavior(bb,id,&hovered,&held)) g_refAutoPlayMode=i;
                    const bool selected=(g_refAutoPlayMode==i);
                    const ImU32 fill=selected?IM_COL32(190,158,35,255):IM_COL32(27,27,34,245);
                    const ImU32 border=selected?IM_COL32(218,188,65,255):IM_COL32(70,68,78,235);
                    dl->AddRectFilled(a,b,fill,6.0f*s);
                    dl->AddRect(a,b,border,6.0f*s,0,1.0f*s);
                    const ImVec2 ts=GetFont()->CalcTextSizeA(10.0f*s,FLT_MAX,0.0f,label);
                    const ImVec2 tp((a.x+b.x-ts.x)*0.5f,(a.y+b.y-ts.y)*0.5f);
                    dl->AddText(GetFont(),10.0f*s,tp,selected?IM_COL32(20,20,20,255):IM_COL32(225,225,230,255),label);
                }
            }
        }
    }
    End();
    PopStyleVar();
    PopStyleColor();
}

static void DrawFloatingButton(ImGuiIO& io) {
    static GLuint logo_tex=LoadTextureFromMemory(black_logo_png,black_logo_png_len);
    const float s=ImMax(0.55f,io.DisplaySize.x/694.0f);
    const float logoSize=g_logoBtnSize*s;
    g_logoBtnX=ImClamp(g_logoBtnX,0.0f,694.0f-g_logoBtnSize);
    g_logoBtnY=ImClamp(g_logoBtnY,0.0f,1536.0f-g_logoBtnSize);
    const float px=g_logoBtnX*s, py=g_logoBtnY*s;

    SetNextWindowPos(ImVec2(px,py),ImGuiCond_Always);
    SetNextWindowSize(ImVec2(logoSize+8*s,logoSize+8*s),ImGuiCond_Always);
    PushStyleColor(ImGuiCol_WindowBg,IM_COL32(0,0,0,0));
    PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
    if(Begin("##FloatBtn",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings)) {
        ImDrawList* dl=GetWindowDrawList(); ImVec2 pos=GetCursorScreenPos();
        InvisibleButton("##FloatBtnHit",ImVec2(logoSize,logoSize));
        if(IsItemActive() && IsMouseDragging(ImGuiMouseButton_Left,2.0f)) {
            g_logoBtnX+=io.MouseDelta.x/s; g_logoBtnY+=io.MouseDelta.y/s;
        } else if(IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Left) &&
                  (GetMouseDragDelta(ImGuiMouseButton_Left).x*GetMouseDragDelta(ImGuiMouseButton_Left).x+
                   GetMouseDragDelta(ImGuiMouseButton_Left).y*GetMouseDragDelta(ImGuiMouseButton_Left).y)<9.0f) {

            g_menu.isOpen=!g_menu.isOpen;
        }
        dl->AddImageRounded((void*)(intptr_t)logo_tex,pos,ImVec2(pos.x+logoSize,pos.y+logoSize),ImVec2(0,0),ImVec2(1,1),IM_COL32_WHITE,logoSize*0.5f);
    }
    End(); PopStyleVar(); PopStyleColor();
}

static bool first_time = true;

static bool RefLoginButton(const char* id, const char* text, const ImVec2& pos,
                           const ImVec2& size, ImU32 fill, ImU32 border,
                           GLuint iconTex = 0, bool darkText = false) {
    SetCursorPos(pos);
    InvisibleButton(id, size);
    const bool pressed = IsItemClicked(ImGuiMouseButton_Left);
    const bool hovered = IsItemHovered();
    ImDrawList* dl = GetWindowDrawList();
    ImVec2 a = GetItemRectMin();
    ImVec2 b = GetItemRectMax();
    const ImU32 drawFill = hovered ? ColorConvertFloat4ToU32(ImVec4(
        ImMin(1.0f, ((fill >> IM_COL32_R_SHIFT) & 0xff) / 255.0f + 0.035f),
        ImMin(1.0f, ((fill >> IM_COL32_G_SHIFT) & 0xff) / 255.0f + 0.035f),
        ImMin(1.0f, ((fill >> IM_COL32_B_SHIFT) & 0xff) / 255.0f + 0.035f), 1.0f)) : fill;
    dl->AddRectFilled(a, b, drawFill, 18.0f);
    dl->AddRect(a, b, border, 18.0f, 0, 1.8f);

    const float iconW = iconTex ? size.y * 0.45f : 0.0f;
    const float gap = iconTex ? size.y * 0.12f : 0.0f;
    SetWindowFontScale(size.y / 76.0f);
    ImVec2 ts = CalcTextSize(text);
    const float groupW = iconW + gap + ts.x;
    float gx = a.x + (size.x - groupW) * 0.5f;
    const float gy = a.y + (size.y - ts.y) * 0.5f;
    if (iconTex) {
        float iy = a.y + (size.y - iconW) * 0.5f;
        dl->AddImage((void*)(intptr_t)iconTex,
                     ImVec2(gx, iy), ImVec2(gx + iconW, iy + iconW));
        gx += iconW + gap;
    }
    dl->AddText(ImVec2(gx, gy), darkText ? IM_COL32(8,8,8,255) : IM_COL32(255,255,255,255), text);
    SetWindowFontScale(1.0f);
    return pressed;
}

static void RefGoldHeading(ImDrawList* dl, const ImVec2& center, const char* text, float scale) {
    SetWindowFontScale(scale);
    ImVec2 ts = CalcTextSize(text);
    const float lineW = scale * 58.0f;
    const float gap = scale * 22.0f;
    const float total = lineW + gap + ts.x + gap + lineW;
    float x = center.x - total * 0.5f;
    const float y = center.y - ts.y * 0.5f;
    const ImU32 gold = IM_COL32(255, 205, 0, 255);

    for (int i = 0; i < 3; ++i) {
        float yy = y + ts.y * (0.20f + i * 0.30f);
        float len = lineW * (1.0f - i * 0.20f);
        dl->AddLine(ImVec2(x, yy), ImVec2(x + len, yy), gold, 2.0f);
        dl->AddLine(ImVec2(x + total - len, yy), ImVec2(x + total, yy), gold, 2.0f);
    }
    x += lineW + gap;
    dl->AddText(ImVec2(x, y), gold, text);
    SetWindowFontScale(1.0f);
}

INLINE void DrawLogin(ImGuiIO& io) {
    if (logged_in) return DrawMenu(io);

    SetNextWindowPos(ImVec2(0, 0));
    SetNextWindowSize(io.DisplaySize);
    PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.42f));
    Begin(O("##Overlay"), nullptr,
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
          ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus |
          ImGuiWindowFlags_NoSavedSettings);
    PopStyleColor();
    End();

    const float refAspect = 1216.0f / 1536.0f;
    const float maxW = ImMin(io.DisplaySize.x * 0.92f, 980.0f);
    const float cardW = maxW;
    const float cardH = ImMin(cardW / refAspect, io.DisplaySize.y * 0.88f);
    const float scale = cardW / 900.0f;

    SetNextWindowSize(ImVec2(cardW, cardH), ImGuiCond_Always);
    SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                     ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.035f, 0.038f, 0.042f, 0.965f));
    PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.82f, 0.0f, 1.0f));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 26.0f * scale);
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f * scale);

    if (Begin(O("##LoginCard"), nullptr,
              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings)) {
        ImDrawList* dl = GetWindowDrawList();
        const ImVec2 wp = GetWindowPos();
        const float s = scale;

        dl->AddRect(ImVec2(wp.x + 1.0f*s, wp.y + 1.0f*s),
                    ImVec2(wp.x + cardW - 1.0f*s, wp.y + cardH - 1.0f*s),
                    IM_COL32(255, 210, 0, 255), 26.0f*s, 0, 1.7f*s);

        static GLuint blackLogoTex = LoadTextureFromMemory(black_logo_png, black_logo_png_len);
        static GLuint keyTex = LoadTextureFromMemory(ref_key_png, ref_key_png_len);
        static GLuint pasteTex = LoadTextureFromMemory(ref_paste_png, ref_paste_png_len);
        static GLuint telegramTex = LoadTextureFromMemory(ref_telegram_png, ref_telegram_png_len);

        const float logoSize = cardW * 0.405f;
        const float logoTop = cardH * 0.047f;
        dl->AddImage((void*)(intptr_t)blackLogoTex,
                     ImVec2(wp.x + (cardW-logoSize)*0.5f, wp.y + logoTop),
                     ImVec2(wp.x + (cardW+logoSize)*0.5f, wp.y + logoTop + logoSize));

        RefGoldHeading(dl, ImVec2(wp.x + cardW*0.5f, wp.y + cardH*0.405f),
                       O("ENTER LICENSE KEY"), 1.55f*s);

        const float left = cardW * 0.062f;
        const float fieldW = cardW - left*2.0f;
        const float fieldH = cardH * 0.102f;
        const float fieldY = cardH * 0.485f;
        const float buttonH = cardH * 0.103f;
        const float loginY = cardH * 0.595f;
        const float pasteY = cardH * 0.705f;
        const float telegramY = cardH * 0.815f;

        const ImVec2 fa(wp.x + left, wp.y + fieldY);
        const ImVec2 fb(fa.x + fieldW, fa.y + fieldH);
        dl->AddRectFilled(fa, fb, IM_COL32(9,10,12,245), 18.0f*s);
        dl->AddRect(fa, fb, IM_COL32(92,92,92,255), 18.0f*s, 0, 2.0f*s);

        const float iconSize = fieldH * 0.42f;
        dl->AddImage((void*)(intptr_t)keyTex,
                     ImVec2(fa.x + fieldH*0.29f, fa.y + (fieldH-iconSize)*0.5f),
                     ImVec2(fa.x + fieldH*0.29f + iconSize, fa.y + (fieldH+iconSize)*0.5f));

        static char keyBuf[256] = {0};
        static bool keyLoaded = false;
        if (!keyLoaded) {
            auto it = persistent_string.find(O("key"));
            if (it != persistent_string.end() && !it->second.empty()) {
                strncpy(keyBuf, it->second.c_str(), sizeof(keyBuf)-1);
                keyBuf[sizeof(keyBuf)-1] = '\0';
            }
            keyLoaded = true;
        }

        SetCursorPos(ImVec2(left + fieldH*0.90f, fieldY + fieldH*0.07f));
        PushItemWidth(fieldW - fieldH*1.12f);
        PushStyleColor(ImGuiCol_FrameBg, ImVec4(0,0,0,0));
        PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0,0,0,0));
        PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0,0,0,0));
        PushStyleColor(ImGuiCol_Text, ImVec4(0.68f,0.68f,0.70f,1.0f));
        PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.48f,0.48f,0.50f,1.0f));
        PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, fieldH*0.18f));
        PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        SetWindowFontScale(1.30f*s);
        InputTextWithHint(O("##license_input"), O("Enter your license key here..."),
                          keyBuf, sizeof(keyBuf));
        SetWindowFontScale(1.0f);
        PopStyleVar(2);
        PopStyleColor(5);
        PopItemWidth();

        const ImU32 yellow = IM_COL32(255, 198, 0, 255);
        const ImU32 dark = IM_COL32(7, 7, 8, 255);
        const ImU32 grey = IM_COL32(25, 26, 29, 245);
        const ImU32 greyBorder = IM_COL32(86, 86, 88, 255);
        const ImU32 blue = IM_COL32(19, 139, 224, 255);
        const ImU32 blueBorder = IM_COL32(38, 177, 246, 255);

        if (RefLoginButton(O("##ref_login"), O("LOGIN"), ImVec2(left, loginY),
                           ImVec2(fieldW, buttonH), yellow, IM_COL32(255,221,75,255), 0, true)) {
            std::string keyToUse = keyBuf;
            if (keyToUse.empty()) {
                JNIEnv* env2 = nullptr;
                bool attachedClip = false;
                if (VM) {
                    jint r = VM->GetEnv((void**)&env2, JNI_VERSION_1_6);
                    if (r == JNI_EDETACHED) {
                        if (VM->AttachCurrentThread(&env2, nullptr) == 0)
                            attachedClip = true;
                    }
                }
                if (env2) {
                    keyToUse = getClipboard(env2);
                    if (attachedClip) VM->DetachCurrentThread();
                }
            }
            if (DEBUG_BYPASS_LOGIN) {
                logged_in = true;
                g_menu.isOpen = true;
            } else {
                JNIEnv* env = nullptr;
                jint getEnvResult = VM->GetEnv((void**)&env, JNI_VERSION_1_6);
                if (getEnvResult == JNI_EDETACHED) {
                    if (VM->AttachCurrentThread(&env, nullptr) != 0)
                        ERROR_MESSAGE = O("Failed to attach thread to JVM");
                } else if (getEnvResult != JNI_OK) {
                    ERROR_MESSAGE = O("Failed to get JNIEnv");
                } else {
                    persistent_string[O("key")] = keyToUse;
                    std::thread([](std::string androidId, std::string key) {
                        Login(androidId, key);
                    }, getAndroidID(env), keyToUse).detach();
                }
                first_time = false;
            }
        }

        if (RefLoginButton(O("##ref_paste"), O("PASTE LICENSE KEY"), ImVec2(left, pasteY),
                           ImVec2(fieldW, buttonH), grey, greyBorder, pasteTex, false)) {
            JNIEnv* env2 = nullptr;
            bool attached2 = false;
            if (VM) {
                jint r = VM->GetEnv((void**)&env2, JNI_VERSION_1_6);
                if (r == JNI_EDETACHED) {
                    if (VM->AttachCurrentThread(&env2, nullptr) == 0)
                        attached2 = true;
                }
            }
            if (env2) {
                std::string clip = getClipboard(env2);
                if (!clip.empty()) {
                    strncpy(keyBuf, clip.c_str(), sizeof(keyBuf)-1);
                    keyBuf[sizeof(keyBuf)-1] = '\0';
                    persistent_string[O("key")] = clip;
                }
                if (attached2) VM->DetachCurrentThread();
            }
        }

        if (RefLoginButton(O("##ref_telegram"), O("TELEGRAM"), ImVec2(left, telegramY),
                           ImVec2(fieldW, buttonH), blue, blueBorder, telegramTex, false)) {
            JNIEnv* env3 = nullptr;
            bool attached3 = false;
            if (VM) {
                jint r = VM->GetEnv(reinterpret_cast<void**>(&env3), JNI_VERSION_1_6);
                if (r == JNI_EDETACHED) {
                    if (VM->AttachCurrentThread(&env3, nullptr) == 0)
                        attached3 = true;
                }
            }
            if (env3) {
                jclass uriClass = env3->FindClass("android/net/Uri");
                jmethodID parse = env3->GetStaticMethodID(uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
                jstring url = env3->NewStringUTF("https://t.me/+3ng9H1jBE9JhMzI1");//chanel link
                jobject uri = env3->CallStaticObjectMethod(uriClass, parse, url);
                jclass intentClass = env3->FindClass("android/content/Intent");
                jmethodID ctor = env3->GetMethodID(intentClass, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V");
                jstring action = env3->NewStringUTF("android.intent.action.VIEW");
                jobject intent = env3->NewObject(intentClass, ctor, action, uri);
                jmethodID addFlags = env3->GetMethodID(intentClass, "addFlags", "(I)Landroid/content/Intent;");
                if (addFlags) env3->CallObjectMethod(intent, addFlags, 0x10000000);
                jclass activityThread = env3->FindClass("android/app/ActivityThread");
                jmethodID current = env3->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
                jobject at = env3->CallStaticObjectMethod(activityThread, current);
                jmethodID getApp = env3->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
                jobject app = env3->CallObjectMethod(at, getApp);
                jclass contextClass = env3->FindClass("android/content/Context");
                jmethodID startActivity = env3->GetMethodID(contextClass, "startActivity", "(Landroid/content/Intent;)V");
                if (startActivity && app && intent) env3->CallVoidMethod(app, startActivity, intent);
                if (attached3) VM->DetachCurrentThread();
            }
        }
    }
    End();
    PopStyleVar(3);
    PopStyleColor(2);
}

INLINE void SetupImgui() {
    if (bImguiSetup) return;

    if (PACKAGE_NAME.empty()) {
        PACKAGE_NAME = "com.miniclip.carrom";
    }

    LOGF("SetupImgui: creating context");
    ImGui::CreateContext();

    auto& style = ImGui::GetStyle();
    auto& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;

    LOGF("SetupImgui: loading theme");
    switch_theme(current_theme);

    LOGF("SetupImgui: loading persistence");
    load_persistence();
    persistent_bool[O("bESP_DrawPockets")] = true;
    persistent_bool[O("bESP_DrawPredictionAfterShot")] = true;
    persistent_bool[O("bAutoPlay")] = false;
    persistent_bool[O("bCleanTable")] = false;
    AutoPlay::bAutoPlaying = false;

    LOGF("SetupImgui: loading svConfig");
    svConfig_Load();

    LOGF("SetupImgui: loading imgui style");
    load_imgui_style();

    static string INI_PATH = O("/data/user_de/0/") + PACKAGE_NAME + O("/no_backup/.ini");
    io.IniFilename = persistent_bool["bImguiAutoSave"] ? INI_PATH.c_str() : nullptr;
    io.ConfigWindowsMoveFromTitleBarOnly = persistent_bool["bMoveOnlyWithTitleBar"];

    ImFontConfig font_cfg;
    float fontSize = persistent_float["fFontScale"];
    if (fontSize < 1.0f) fontSize = 40.0f;
    font_cfg.SizePixels = fontSize;
    io.Fonts->AddFontDefault(&font_cfg);

    LOGF("SetupImgui: initializing OpenGL3");
    ImGui_ImplOpenGL3_Init(O("#version 300 es"));

    bImguiSetup = true;
    LOGF("SetupImgui: DONE");
}

static int draw_call_count = 0;
DEFINES(EGLBoolean, Draw, EGLDisplay dpy, EGLSurface surface) {
    draw_call_count++;
    if (draw_call_count <= 3) LOGF("Draw: called #%d", draw_call_count);

    eglQuerySurface(dpy, surface, EGL_WIDTH, &Width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &Height);
    if (draw_call_count <= 3) LOGF("Draw: W=%d H=%d", Width, Height);

    if (Width <= 0 || Height <= 0) return _Draw(dpy, surface);

    screenCenter = Vector2(Width / 2, Height / 2);

    if (!bImguiSetup) {
        LOGF("Draw: calling SetupImgui");
        SetupImgui();
        LOGF("Draw: SetupImgui returned, bImguiSetup=%d", bImguiSetup);
    }

    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)Width, (float)Height);
    static double lastTime = 0.0;
    double currentTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count() / 1000.0;
    io.DeltaTime = lastTime > 0.0 ? (float)(currentTime - lastTime) : 1.0f / 60.0f;
    lastTime = currentTime;

    ImGui_ProcessPendingTouch();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (IsExpired()) {
        DrawExpired(io);
    } else if ((!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN) {
        DrawMenu(io);

        DrawFloatingButton(io);
        DrawToggleButton(persistent_bool[O("bAutoQueue")]);

{
    SetNextWindowPos(ImVec2(Width * 0.5f, Height - 60.0f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));

    Begin(O("##PoweredBy"), nullptr,
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
          ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize |
          ImGuiWindowFlags_NoInputs);

    TextColored(ImColor(255, 215, 0, 255), O("GALVNIC ENGINE"));

    End();
}

        if (g_autoPlayCalculating) DrawCalculating(io);
    } else {
        DrawLogin(io);
    }
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui_ClearHoverEffect();

    return _Draw(dpy, surface);
}

void __IMGUI__() {
    create_directory_recursive(CONC(O("/data/user_de/0/"), PACKAGE_NAME.c_str(), O("/no_backup")));
}
