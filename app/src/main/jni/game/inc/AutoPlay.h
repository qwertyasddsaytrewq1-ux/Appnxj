/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include "Prediction.fast.h"
#include <imgui/imgui.h>
#include <algorithm>
#include "ScreenTable.h"
#include "mod/ButtonClicker.h"
#include "include/input.h"

#include "DirectFire.h"

#include <cstdio>
inline char g_AutoPlayDbg[192] = "AP: boot";
#define AP_DBG(...) snprintf(g_AutoPlayDbg, sizeof(g_AutoPlayDbg), __VA_ARGS__)
#include "include/java.h"

using namespace ImGui;

struct ShotDragger {
    bool Active = false;
    float T = 0.f;
    int TouchIndex = 9;
    ImVec2 StartPos, ReleasePos, CurPos;

    enum State { PRESS, PULL, HOLD } state = PRESS;

    void Fire(ImVec2 start, ImVec2 release) {
        if (Active) return;
        StartPos = start;
        ReleasePos = release;
        CurPos = start;
        T = 0.f;
        Active = true;
        state = PRESS;
    }

    void Update() {
        if (!Active) return;
        float dt = ImGui::GetIO().DeltaTime;
        T += dt;

        JNIThreadScope jni;
        if (!jni.env) return;

        if (state == PRESS) {

            if (T >= 0.12f) {
                NativeTouchesBegin(jni.env, TouchIndex, StartPos.x, StartPos.y);
                state = PULL;
                T = 0.f;
            }
        } else if (state == PULL) {
            float t = T / 0.45f;
            if (t > 1.f) t = 1.f;
            CurPos = ImVec2(StartPos.x + (ReleasePos.x - StartPos.x) * t,
                            StartPos.y + (ReleasePos.y - StartPos.y) * t);
            NativeTouchesMove(jni.env, TouchIndex, CurPos.x, CurPos.y);
            if (t >= 1.f) { state = HOLD; T = 0.f; AP_DBG("AP: drag PULL->HOLD done"); }
        } else if (state == HOLD) {

            if (T >= 0.20f) {
                NativeTouchesEnd(jni.env, TouchIndex, ReleasePos.x, ReleasePos.y);
                Active = false;
                state = PRESS;
            }
        }

        if (dynamic_bool["DebugTouch"] && Active) {
            ImDrawList* fg = ImGui::GetForegroundDrawList();
            fg->AddCircleFilled(CurPos, 15.0f, IM_COL32(255, 180, 0, 150));
            fg->AddCircle(StartPos, 20.0f, IM_COL32(0, 255, 0, 220), 0, 2.0f);
            fg->AddLine(StartPos, ReleasePos, IM_COL32(255, 0, 0, 200), 2.0f);
        }
    }
};
inline ShotDragger shotDragger;

#ifndef MAX_ANGLE_DEFINED
#define MAX_ANGLE_DEFINED
inline constexpr double maxAngle = 360.0 / (180.0 / M_PI);
#endif

#ifndef NORMALIZE_ANGLE_DEFINED
#define NORMALIZE_ANGLE_DEFINED
inline double normalizeAngle(double angle) {
    double newAngle = angle;
    if (newAngle >= maxAngle) newAngle = fmod(newAngle, maxAngle);
    else if (newAngle < 0) newAngle = maxAngle - fmod(-newAngle, maxAngle);
    return newAngle;
}
#endif

Candidate g_CurrentCandidate = { -1 };

inline int PredictionIdxToGamePieceId(int predIdx) {
    int n = g_cgs.framePieceCount;
    if (!g_cgs.frameDataValid || predIdx < 0 || n <= 0 || n > 32) return -1;
    int strikerSrc = -1;
    for (int i = 0; i < n; ++i)
        if (g_cgs.framePieceStates[i] == 0) { strikerSrc = i; break; }
    if (strikerSrc < 0)
        for (int i = 0; i < n; ++i)
            if (g_cgs.framePieceIds[i] == 0) { strikerSrc = i; break; }
    if (strikerSrc < 0) return -1;
    if (predIdx == 0) return g_cgs.framePieceIds[strikerSrc];
    int out = 1;
    for (int i = 0; i < n; ++i) {
        if (i == strikerSrc) continue;
        if (!g_cgs.framePiecePosValid[i]) continue;
        if (out == predIdx) return g_cgs.framePieceIds[i];
        ++out;
    }
    return -1;
}

ImVec2 GetPocketScreenPos(int pocketIdx) {
    Board board = g_cgs.board;
    if (!board) return {};

    auto boardProperties = board.mBoardProperties();
    if (!boardProperties) return {};

    auto& pockets = boardProperties.mPockets();
    return WorldToScreen(pockets[pocketIdx]);
}

bool IsShotValid() {
    auto& cand = g_CurrentCandidate;
    if (cand.idx == -1) return false;

    CarromPieceEnums::Type myclass = (CarromPieceEnums::Type)g_cgs.targetPiece;
    uint pottedPocket = g_cgs.pottedPocketId;
    if (pottedPocket < 4 && cand.pocketIndex != pottedPocket) return false;

    if (!gPrediction->guiData.pieces[0].onTable) return false;
    if (!gPrediction->guiData.pieces[cand.idx].originalOnTable) return false;
    if (gPrediction->guiData.pieces[cand.idx].onTable) return false;
    if (gPrediction->guiData.pieces[cand.idx].pocketIndex != cand.pocketIndex) return false;

    auto* queenPiecePtr = findQueenPiece();
    if (myclass == CarromPieceEnums::Type::ANY && queenPiecePtr && queenPiecePtr->originalOnTable && !queenPiecePtr->onTable) return false;

    auto& firstHit = gPrediction->guiData.collision.firstHitPiece;
    if (firstHit) {
        if (myclass == CarromPieceEnums::Type::ANY) {
            if (firstHit->classification == CarromPieceEnums::Type::QUEEN) return false;
        } else if (firstHit->classification != myclass) return false;
    }

    return true;
}

Point2D lastFailedStrikerPos = { -1000.0, -1000.0 };

namespace AutoPlay {
    double lastSetAngle = 0.f;
    bool didSetAngle = false;
    bool bAutoPlaying = false;

    enum State {
        IDLE,
        SCANNING,
        NOMINATING,
        EXECUTING,
    } state = IDLE;

    double pendingShotPower = 0.f;
    double pendingShotAngle = 0.f;
    int nominationFrameCounter = 0;

    enum ScanMode {
        FAST,
        SLOW,
    } scan = FAST;

    bool shouldAutoPlay() {

        if (!g_cgs.gameManager || !g_cgs.board) return false;
        if (!g_cgs.playerTurn) return false;
        if (!g_cgs.physicsObj) return false;
        return !didSetAngle || didSetAngle;
    }

    void setAimAngle(double angle) {

        lastSetAngle = angle;
    }

    void takeShot(double angle, double power) {
        setAimAngle(angle);
        gPrediction->determineShotResult(false, angle, power);
        if (!g_cgs.aimValid) return;

        g_cgs.pendingShotAngle = angle;
        g_cgs.pendingShotPower = power;
        g_cgs.pendingShot = true;

    }

        void ClearState() {
        g_CurrentCandidate.idx = -1;
        lastFailedStrikerPos = { -1000.0, -1000.0 };
    }

    bool dfWaiting = false;
    double dfWaitAngle = 0.0;
    double dfWaitPower = 0.0;
    int dfWaitFrames = 0;

    int dfCooldown = 0;

    void DragFire(double angle, double power) {
        if (shotDragger.Active) return;
        Vec2d strikerW(g_cgs.aimAngle, g_cgs.aimPower);
        g_GameTrScreenH = (double)ImGui::GetIO().DisplaySize.y;
        ImVec2 sp = WorldToScreen(strikerW);

        ImVec2 dir((float)cos(angle), -(float)sin(angle));
        float pullPx = (float)power * 0.30f * ImGui::GetIO().DisplaySize.y;
        ImVec2 rel(sp.x - dir.x * pullPx, sp.y - dir.y * pullPx);
        shotDragger.Fire(sp, rel);
        LOGI("V74 DRAG-FALLBACK ang=%.3f pow=%.3f str=(%.0f,%.0f) rel=(%.0f,%.0f) len=%.0f",
             angle, power, sp.x, sp.y, rel.x, rel.y);
    }

    void FireRealShot(double angle, double power) {
        AP_DBG("AP: FIRE-REAL a=%.2f p=%.2f", angle, power);
        int pieceId = PredictionIdxToGamePieceId(g_CurrentCandidate.idx);
        if (DirectFire::Submit(angle, power, pieceId)) {
            dfWaiting = true;
            dfWaitAngle = angle;
            dfWaitPower = power;
            dfWaitFrames = 0;
            AP_DBG("AP: DFIRE submit id=%d a=%.2f p=%.2f", pieceId, angle, power);
            return;
        }
        DragFire(angle, power);
    }

    void Shoot(double angle, double power = 0.f) {
        AP_DBG("AP: SHOOT a=%.2f p=%.2f", angle, power);
        setAimAngle(angle);
        gPrediction->determineShotResult(false, angle, power);

        bool nominating = false;
        bool requireQueenCover = g_cgs.queenCoverRequired;
        auto myclass = (CarromPieceEnums::Type)g_cgs.targetPiece;
        if (requireQueenCover && myclass != CarromPieceEnums::Type::ANY) {
            if (g_CurrentCandidate.idx != -1 && g_cgs.pottedPocketId != g_CurrentCandidate.pocketIndex) {
                nominating = true;
            }
        }

        if (nominating) {
            pendingShotPower = power;
            pendingShotAngle = angle;
            state = NOMINATING;
            nominationFrameCounter = 0;
        } else {
            takeShot(angle, power);
            FireRealShot(angle, power);
            ClearState();
            state = IDLE;
        }
    }

    void ScanSlow(double angleStep = 0.01f) {
        static double currentScanAngle = 0.0;
        static bool isScanning = false;
        static Point2D lastScanStrikerPos = { -1000.0, -1000.0 };

        if (g_CurrentCandidate.idx != -1) return;

        if (!isScanning || gPrediction->guiData.pieces[0].initialPosition != lastScanStrikerPos) {
            currentScanAngle = 0.0;
            isScanning = true;
            lastScanStrikerPos = gPrediction->guiData.pieces[0].initialPosition;
        }

        CarromPieceEnums::Type myclass = (CarromPieceEnums::Type)g_cgs.targetPiece;
        uint pottedPocket = g_cgs.pottedPocketId;

        int steps = 0;
        bool foundShot = false;

        while (steps < 10 && currentScanAngle < maxAngle) {
            double angle = currentScanAngle;
            currentScanAngle += angleStep;
            steps++;

            std::vector<double> powers = {0.52, 0.76, 1.0};
            for (double power : powers) {
                gPrediction->determineShotResult(true, angle, power, Vec2d(g_cgs.shotSpinX, g_cgs.shotSpinY));

                bool isPotentiallyValid = false;
                int targetIdx = -1;

                for (int i = 1; i < gPrediction->guiData.piecesCount; i++) {
                    auto& piece = gPrediction->guiData.pieces[i];
                    if (piece.originalOnTable && !piece.onTable) {
                        bool isValidTarget = false;
                        if (myclass == CarromPieceEnums::Type::ANY) {
                            if (piece.classification != CarromPieceEnums::Type::STRIKER && piece.classification != CarromPieceEnums::Type::QUEEN) isValidTarget = true;
                        } else {
                            if (piece.classification == myclass) isValidTarget = true;
                        }
                        if (pottedPocket < 4 && piece.pocketIndex != pottedPocket) isValidTarget = false;
                        if (isValidTarget) { targetIdx = i; break; }
                    }
                }

                if (targetIdx != -1) {
                    if (!gPrediction->guiData.pieces[0].onTable) continue;
                    if (!queenOnTableNow() && myclass != CarromPieceEnums::Type::QUEEN) continue;
                    auto firstHit = gPrediction->guiData.collision.firstHitPiece;
                    if (!firstHit) continue;
                    if (myclass == CarromPieceEnums::Type::ANY) {
                        if (firstHit->classification == CarromPieceEnums::Type::QUEEN) continue;
                    } else if (firstHit->classification != myclass) continue;

                    isPotentiallyValid = true;
                    g_CurrentCandidate.idx = targetIdx;
                    g_CurrentCandidate.angle = angle;
                    g_CurrentCandidate.power = power;
                    g_CurrentCandidate.pocketIndex = gPrediction->guiData.pieces[targetIdx].pocketIndex;
                }

                if (isPotentiallyValid) {
                    foundShot = true;
                    Shoot(angle, power);
                    break;
                }
            }
            if (foundShot) break;
        }

        if (!foundShot && currentScanAngle >= maxAngle) {
            isScanning = false;
            currentScanAngle = 0.0;
            state = IDLE;
        }
    }

    void ScanFast(double angleStep = 0.1f) {
        if (g_CurrentCandidate.idx != -1) return;
        if (gPrediction->guiData.pieces[0].initialPosition == lastFailedStrikerPos) return;

        if (!g_cgs.aimValid || !g_cgs.gameManager) return; double startingAngle = g_cgs.aimAngle;
        CarromPieceEnums::Type myclass = (CarromPieceEnums::Type)g_cgs.targetPiece;
        uint pottedPocket = g_cgs.pottedPocketId;
        std::vector<Candidate> candidates;
        auto pockets = getPockets();
        auto& striker = gPrediction->guiData.pieces[0];

        for (int i = 1; i < gPrediction->guiData.piecesCount; i++) {
            auto& piece = gPrediction->guiData.pieces[i];
            if (!piece.originalOnTable) continue;

            bool isACandidate = (myclass == CarromPieceEnums::Type::ANY) ? piece.classification != CarromPieceEnums::Type::QUEEN : piece.classification == myclass;
            if (!isACandidate) continue;

            for (int pocketIdx = 0; pocketIdx < pockets.size(); pocketIdx++) {
                if (pottedPocket < 4 && pocketIdx != pottedPocket) continue;
                Point2D pocket = pockets[pocketIdx];
                Point2D toPocket = pocket - piece.initialPosition;
                double distTargetToPocket = sqrt(toPocket.square());
                if (distTargetToPocket < 0.1) continue;

                Point2D direction = toPocket * (1.0 / distTargetToPocket);
                Point2D ghostPiecePos = piece.initialPosition - direction * (PIECE_RADIUS + STRIKER_RADIUS);

                Point2D shotLine = ghostPiecePos - striker.initialPosition;
                double distStrikerToTarget = sqrt(shotLine.square());
                double angle = atan2(shotLine.y, shotLine.x);
                if (angle < 0) angle += 2 * M_PI;

                double score = distStrikerToTarget + distTargetToPocket;

                double power = 1.0;
                candidates.push_back({i, angle, score, pocketIdx, power});
            }
        }

        std::sort(candidates.begin(), candidates.end());
        bool foundShot = false;
        int tried = 0;
        for (const auto& cand : candidates) {
            if (++tried > 24) break;
            double angle = NumberUtils::normalizeDoublePrecision(normalizeAngle(cand.angle));

            auto shotIsValid = [&]() -> bool {
                if (!gPrediction->firstHitIsTarget) return false;
                if (!gPrediction->guiData.pieces[0].onTable) return false;
                if (gPrediction->guiData.pieces[cand.idx].onTable) return false;
                if (gPrediction->guiData.pieces[cand.idx].pocketIndex != cand.pocketIndex) return false;

                bool isAngleGood = false;
                for (int i = 1; i < gPrediction->guiData.piecesCount; i++) {
                    Prediction::PieceState& piece = gPrediction->guiData.pieces[i];
                    bool match = (myclass == CarromPieceEnums::Type::ANY) ? (piece.classification != CarromPieceEnums::Type::STRIKER && piece.classification != CarromPieceEnums::Type::QUEEN) : (piece.classification == myclass);
                    if (match && piece.originalOnTable && !piece.onTable) isAngleGood = true;
                }
                if (isAngleGood && gPrediction->guiData.collision.firstHitPiece) {
                     auto firstHit = gPrediction->guiData.collision.firstHitPiece;
                     if (myclass != CarromPieceEnums::Type::ANY && firstHit->classification != myclass) isAngleGood = false;
                     else if (myclass == CarromPieceEnums::Type::ANY && firstHit->classification == CarromPieceEnums::Type::QUEEN) isAngleGood = false;
                }
                if (isAngleGood && !gPrediction->guiData.pieces[0].onTable) isAngleGood = false;
                auto* queenPieceRef = findQueenPiece();
                if (isAngleGood && queenPieceRef && queenPieceRef->originalOnTable && !queenPieceRef->onTable && myclass != CarromPieceEnums::Type::QUEEN) isAngleGood = false;
                return isAngleGood;
            };
            auto evalShot = [&](double a, double p) -> bool {
                gPrediction->determineShotResult(true, a, p, Vec2d(g_cgs.shotSpinX, g_cgs.shotSpinY), cand);
                return shotIsValid();
            };

            const double soTiers[3] = {0.52, 0.76, 1.0};
            for (double tierPower : soTiers) {
                if (!evalShot(angle, tierPower)) continue;

                double bestAngle = angle, bestPower = tierPower;
                const double soAngleDeltas[4] = {-0.008, 0.008, -0.022, 0.022};
                const double soPowerDeltas[2] = {-0.07, 0.07};
                for (double da : soAngleDeltas) {
                    double a2 = normalizeAngle(angle + da);
                    if (evalShot(a2, bestPower)) bestAngle = a2;
                }
                for (double dp : soPowerDeltas) {
                    double p2 = tierPower + dp;

                    if (p2 < 0.24) p2 = 0.24;
                    if (p2 > 1.0) p2 = 1.0;
                    if (evalShot(bestAngle, p2)) bestPower = p2;
                }

                g_CurrentCandidate = cand;
                g_CurrentCandidate.angle = bestAngle;
                g_CurrentCandidate.power = bestPower;
                foundShot = true;
                Shoot(bestAngle, bestPower);
                break;
            }
            if (foundShot) break;
        }

        if (!foundShot) {
            lastFailedStrikerPos = striker.initialPosition;
            scan = SLOW;
        }
    }

    bool isAnimationActive() {

        return false;
    }

    void Update() {

        if (dfWaiting) {
            int st = DirectFire::PollResult();
            if (st == 1) {
                dfWaiting = false;
                dfCooldown = 45;
                AP_DBG("AP: DFIRE OK");
            } else if (st == 2 || ++dfWaitFrames > 60) {
                dfWaiting = false;
                AP_DBG("AP: DFIRE fail (st=%d) — DRAG fallback a=%.2f", st, dfWaitAngle);
                DragFire(dfWaitAngle, dfWaitPower);
            }
        }
        buttonClicker.Update();
        shotDragger.Update();
        if (dfWaiting) return;
        if (dfCooldown > 0) { dfCooldown--; return; }
        if (shotDragger.Active) return;
        if (isAnimationActive()) return;

        if (!bAutoPlaying || !g_cgs.playerTurn) {
            state = IDLE;
            return;
        }

        if (state == IDLE) {
            state = SCANNING;
            scan = FAST;
        } else if (state == SCANNING) {

            if (g_CurrentCandidate.idx != -1) {
                AP_DBG("AP: cand FOUND idx=%d ang=%.2f pw=%.2f pk=%d", g_CurrentCandidate.idx, g_CurrentCandidate.angle, g_CurrentCandidate.power, g_CurrentCandidate.pocketIndex);
                Shoot(g_CurrentCandidate.angle, g_CurrentCandidate.power);
            } else if (scan == FAST) {

                static double s_scanT = -1e9;
                double nowT = ImGui::GetTime();
                if (nowT - s_scanT > 3.0) {
                    s_scanT = nowT;
                    ScanFast();
                    AP_DBG("AP: scan DONE cand=%d ang=%.2f pw=%.2f", g_CurrentCandidate.idx, g_CurrentCandidate.angle, g_CurrentCandidate.power);
                }
            }
            else if (scan == SLOW) ScanSlow(0.003f);
        } else if (state == NOMINATING) {
            nominationFrameCounter++;
            if (nominationFrameCounter == 10) {
                buttonClicker.Click(GetPocketScreenPos(g_CurrentCandidate.pocketIndex));
            }
            if (nominationFrameCounter > 20 && !buttonClicker.Active) {
                AP_DBG("AP: NOMINATE firing a=%.2f p=%.2f", pendingShotAngle, pendingShotPower);
                takeShot(pendingShotAngle, pendingShotPower);
                FireRealShot(pendingShotAngle, pendingShotPower);
                ClearState();
                state = IDLE;
            }
        }
    }
};
