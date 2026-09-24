/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include "NumberUtils.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

#include "GameConstants.h"
#include <Vector/Vectors.h>

#include "game/CarromPiece.h"
#include "game/GameManager.h"

class Prediction {
public:
    static bool pocketStatus[BOARD_POCKETS_COUNT];
    static float shotResult[MAX_SHOT_RESULT_SIZE];

    struct PieceState {
        int index = 0;
        CarromPieceEnums::Type classification = CarromPieceEnums::Type::ERR_TYPE;
        CarromPieceEnums::State state = CarromPieceEnums::State::ERR_STATE;
        bool originalOnTable = false;
        bool onTable = false;
        int pocketIndex = -1;

        Point2D velocity;
        Vec3d spin;
        Point2D initialPosition;
        Point2D predictedPosition;
        std::vector<Point2D> positions;
        double radius = 0.0;

        double getRadius() const {
            return radius > 0.01 && radius < 10.0 ? radius
                 : (index == 0 ? STRIKER_RADIUS : PIECE_RADIUS);
        }
        void move(double time) {
            predictedPosition.x += velocity.x * time;
            predictedPosition.y += velocity.y * time;
        }
        bool isMovingOrSpinning() const {
            return velocity.square() > 1e-8 ||
                   (spin.x * spin.x + spin.y * spin.y + spin.z * spin.z) > 1e-8;
        }

        void calcVelocity();
        void calcVelocityPostCollision(double angle);
        bool isPiecePieceCollision(double* smallestTime, PieceState& other) const;
        bool isPiecePointCollision(double* smallestTime, const Point2D& point) const;
        bool isPieceLineCollision(double* smallestTime, const Point2D& a, const Point2D& b) const;
        bool willCollideWithBoard(const double* smallestTime) const;
        void determinePieceBoardCollision(void* data, double* smallestTime);
        void findNextCollision(void* data, double* time);
    };

    struct Collision {
        enum class Type { PIECE, LINE, POINT };
        bool valid = false;
        Type type = Type::POINT;
        double angle = 0.0;
        Point2D point{};
        PieceState* pieceA = nullptr;
        PieceState* pieceB = nullptr;
        PieceState* firstHitPiece = nullptr;
    };

    struct SceneData {
        int piecesCount = 0;
        PieceState pieces[MAX_PIECES_COUNT];
        Collision collision{};
        bool shotState = false;
    } guiData;

    Candidate m_candidate = {-1};
    bool firstHitIsTarget = false;
    bool fastCalc = true;
    int shotResultSize = 0;

    Prediction() = default;

    static constexpr double USE_CACHED_AIM = -999.0;
    bool determineShotResult(
        bool isAuto,
        double shotAngle = USE_CACHED_AIM,
        double shotPower = USE_CACHED_AIM,
        Vec2d shotSpin = {},
        Candidate cand = {-1});

    void initPieces();
    void initStriker(double shotAngle, double shotPower, const Vec2d& shotSpin);
    void determinePiecesPositions();
    void handleCollision();
    void handlePiecePieceCollision() const;
    void determineShotState();
};

inline Prediction prediction;
inline Prediction* gPrediction = &prediction;

inline Prediction::PieceState* findQueenPiece() {
    if (!gPrediction) return nullptr;
    for (int i = 0; i < gPrediction->guiData.piecesCount; ++i) {
        if (gPrediction->guiData.pieces[i].classification == CarromPieceEnums::Type::QUEEN)
            return &gPrediction->guiData.pieces[i];
    }
    return nullptr;
}
inline bool queenOnTableNow() { return findQueenPiece() != nullptr; }
inline bool Prediction::pocketStatus[BOARD_POCKETS_COUNT] = {};
inline float Prediction::shotResult[MAX_SHOT_RESULT_SIZE] = {};

inline double predictionPrevAngle = std::numeric_limits<double>::quiet_NaN();
inline double predictionPrevPower = std::numeric_limits<double>::quiet_NaN();
inline Vec2d predictionPrevSpin = Vec2d(0, 0);
inline bool predictionPrevAuto = false;

inline const std::array<Point2D, BOARD_POCKETS_COUNT>& getPockets() {
    static const std::array<Point2D, BOARD_POCKETS_COUNT> pockets = {
        Point2D(-34.5, -34.5), Point2D(34.5, -34.5),
        Point2D(34.5, 34.5), Point2D(-34.5, 34.5)
    };
    return pockets;
}

inline const std::array<Point2D, BOARD_SHAPE_SIZE>& getTableShape() {
    static const std::array<Point2D, BOARD_SHAPE_SIZE> frame = {
        Point2D(-34.0, -32.0), Point2D(-34.0, 32.0),
        Point2D(-32.0, 34.0), Point2D(32.0, 34.0),
        Point2D(34.0, 32.0), Point2D(34.0, -32.0),
        Point2D(32.0, -34.0), Point2D(-32.0, -34.0),
        Point2D(-34.5, -34.0), Point2D(-34.0, -34.5),
        Point2D(34.0, -34.5), Point2D(34.5, -34.0),
        Point2D(34.5, 34.0), Point2D(34.0, 34.5),
        Point2D(-34.0, 34.5), Point2D(-34.5, 34.0),
        Point2D(-35.0, -33.5), Point2D(-33.5, -35.0),
        Point2D(33.5, -35.0), Point2D(35.0, -33.5),
        Point2D(35.0, 33.5), Point2D(33.5, 35.0),
        Point2D(-33.5, 35.0), Point2D(-35.0, 33.5),
        Point2D(-36.0, -36.0), Point2D(36.0, -36.0),
        Point2D(36.0, 36.0), Point2D(-36.0, 36.0),
        Point2D(-37.0, -37.0), Point2D(37.0, -37.0),
        Point2D(37.0, 37.0), Point2D(-37.0, 37.0)
    };
    return frame;
}

inline bool Prediction::determineShotResult(bool isAuto, double shotAngle, double shotPower, Vec2d shotSpin, Candidate cand) {

    if (shotAngle == USE_CACHED_AIM) shotAngle = g_cgs.aimValid ? g_cgs.aimAngle : 0.0;
    if (shotPower == USE_CACHED_AIM) shotPower = g_cgs.aimValid ? g_cgs.aimPower : 0.0;

    if (shotAngle == predictionPrevAngle && shotPower == predictionPrevPower &&
        shotSpin == predictionPrevSpin && isAuto == predictionPrevAuto &&
        cand.idx == m_candidate.idx) return false;

    predictionPrevAngle = shotAngle;
    predictionPrevPower = shotPower;
    predictionPrevSpin = shotSpin;
    predictionPrevAuto = isAuto;

    fastCalc = isAuto;
    m_candidate = cand;
    firstHitIsTarget = false;

    initPieces();
    if (guiData.piecesCount <= 0) return false;
    initStriker(shotAngle, shotPower, shotSpin);
    guiData.collision = Collision{};
    for (bool& p : pocketStatus) p = false;

    determinePiecesPositions();

    for (int i = 0; i < guiData.piecesCount; ++i) {
        auto& piece = guiData.pieces[i];
        if (piece.positions.empty() || piece.positions.back() != piece.predictedPosition)
            piece.positions.push_back(piece.predictedPosition);
    }
    determineShotState();
    return true;
}

inline void Prediction::initPieces() {

    guiData.piecesCount = 0;
    if (!g_cgs.frameDataValid || g_cgs.framePieceCount == 0) return;

    int n = g_cgs.framePieceCount;
    if (n > CachedGameState::MAX_FRAME_PIECES) n = CachedGameState::MAX_FRAME_PIECES;

    double maxAbs = 0.0;
    for (int i = 0; i < n; ++i) {
        if (!g_cgs.framePiecePosValid[i]) continue;
        maxAbs = std::max(maxAbs, std::max(
            std::fabs(g_cgs.framePiecePos[i][0]),
            std::fabs(g_cgs.framePiecePos[i][1])));
    }

    const bool normalized = false;
    auto toWorld = [](double v) { return v; };

    int strikerSrc = -1;
    for (int i = 0; i < n; ++i) {
        if (g_cgs.framePieceStates[i] == 0) { strikerSrc = i; break; }
    }
    if (strikerSrc < 0) {
        for (int i = 0; i < n; ++i) {
            if (g_cgs.framePieceIds[i] == 0) { strikerSrc = i; break; }
        }
    }

    static bool s_loggedLayout = false;
    if (!s_loggedLayout) {
        s_loggedLayout = true;
        bug_log_write("ESP_PIECES: n=%d strikerSrc=%d normalized=%d maxAbs=%f",
                      n, strikerSrc, (int)normalized, maxAbs);
    }

    g_cgs.dbgPiecesN = n;
    g_cgs.dbgStrikerSrc = strikerSrc;
    g_cgs.dbgNormalized = (int)normalized;
    g_cgs.dbgMaxAbs = maxAbs;

    int out = 0;
    auto addPiece = [&](int srcIdx, CarromPieceEnums::Type cls) -> bool {
        if (out >= MAX_PIECES_COUNT) return false;
        if (!g_cgs.framePiecePosValid[srcIdx]) return false;
        PieceState& piece = guiData.pieces[out];
        piece = PieceState{};
        piece.index = out;

        piece.state = CarromPieceEnums::State::DEFAULT;
        piece.classification = cls;
        piece.originalOnTable = true;
        piece.onTable = true;
        piece.initialPosition = Vec2d(toWorld(g_cgs.framePiecePos[srcIdx][0]),
                                      toWorld(g_cgs.framePiecePos[srcIdx][1]));
        piece.predictedPosition = piece.initialPosition;
        piece.positions.reserve(32);
        piece.positions.push_back(piece.initialPosition);
        out++;
        return true;
    };

    if (strikerSrc < 0) return;

    addPiece(strikerSrc, CarromPieceEnums::Type::STRIKER);

    {
        double ax = g_cgs.aimAngle, ay = g_cgs.aimPower;
        bool plausible = (ax == ax && ay == ay)
                      && (ax > -34.0 && ax < 34.0)
                      && (ay > -30.0 && ay < -18.0 || ay > 18.0 && ay < 30.0);
        if ((g_cgs.aimValid || plausible) && out >= 1) {
        Vec2d sp(g_cgs.aimAngle, g_cgs.aimPower);
        guiData.pieces[0].initialPosition = sp;
        guiData.pieces[0].predictedPosition = sp;
        if (!guiData.pieces[0].positions.empty())
            guiData.pieces[0].positions[0] = sp;
        }
    }
    for (int i = 0; i < n && out < MAX_PIECES_COUNT; ++i) {
        if (i == strikerSrc) continue;

        int st = g_cgs.framePieceStates[i];
        CarromPieceEnums::Type cls =
            (st == 0) ? CarromPieceEnums::Type::STRIKER :
            (st == 1) ? CarromPieceEnums::Type::WHITE_PUCK :
                        CarromPieceEnums::Type::BLACK_PUCK;
        addPiece(i, cls);
    }

    guiData.piecesCount = out;
    if (out < 2) { guiData.piecesCount = 0; }
    g_cgs.dbgPiecesBuilt = out;
}

inline void Prediction::initStriker(double shotAngle, double shotPower, const Vec2d& shotSpin) {
    if (guiData.piecesCount <= 0) return;

    if (shotPower < 0.24) shotPower = 0.24;
    if (shotPower > 1.0)  shotPower = 1.0;

    const double c = std::cos(shotAngle);
    const double s = std::sin(shotAngle);
    PieceState& striker = guiData.pieces[0];

    double maxPower = 120.0;
    if (libmain) {
        uintptr_t sp = F(uintptr_t, libmain + O(0x2DEB610));
        if (sp) {
            double mp = F(double, sp + 0x30);
            if (std::isfinite(mp) && mp > 1.0 && mp < 10000.0) maxPower = mp;
        }
    }
    const double speed = shotPower * maxPower;
    striker.velocity.x = speed * c;
    striker.velocity.y = speed * s;

    const double spinFactor = speed / STRIKER_RADIUS;
    const double v = -shotSpin.y * spinFactor;
    striker.spin.x = -(s * v);
    striker.spin.y = c * v;
    striker.spin.z = shotSpin.x * spinFactor;
}

inline void Prediction::determinePiecesPositions() {
    constexpr int MAX_PHYSICS_STEPS = 200000;
    int physicsSteps = 0;
    bool anyMoving = false;

    do {
        if (++physicsSteps > MAX_PHYSICS_STEPS) {
            for (int i = 0; i < guiData.piecesCount; ++i) {
                guiData.pieces[i].velocity.nullify();
                guiData.pieces[i].spin.nullify();
            }
            break;
        }

        double remaining = TIME_PER_TICK;
        do {
            double step = remaining;
            guiData.collision = Collision{};

            for (int i = 0; i < guiData.piecesCount; ++i) {
                if (guiData.pieces[i].onTable && guiData.pieces[i].isMovingOrSpinning())
                    guiData.pieces[i].findNextCollision(&guiData, &step);
            }

            if (step < 0.0 || !std::isfinite(step)) step = 0.0;
            if (step > remaining) step = remaining;

            for (int i = 0; i < guiData.piecesCount; ++i) {
                if (guiData.pieces[i].onTable && guiData.pieces[i].isMovingOrSpinning())
                    guiData.pieces[i].move(step);
            }

            if (guiData.collision.valid) {
                handleCollision();
                if (guiData.collision.firstHitPiece && m_candidate.idx >= 0 &&
                    m_candidate.idx < guiData.piecesCount) {
                    firstHitIsTarget = guiData.collision.firstHitPiece->index == m_candidate.idx;
                    if (!firstHitIsTarget) return;
                }
            }

            if (step <= MIN_TIME) {

                if (guiData.collision.valid && guiData.collision.pieceA)
                    guiData.collision.pieceA->move(MIN_TIME);
                step = MIN_TIME;
            }
            remaining -= step;
        } while (remaining > MIN_TIME && physicsSteps < MAX_PHYSICS_STEPS);

        anyMoving = false;
        for (int i = 0; i < guiData.piecesCount; ++i) {
            if (!guiData.pieces[i].onTable) continue;
            guiData.pieces[i].calcVelocity();
            if (guiData.pieces[i].isMovingOrSpinning()) anyMoving = true;
        }
    } while (anyMoving && physicsSteps < MAX_PHYSICS_STEPS);
}

inline void Prediction::handleCollision() {
    if (!guiData.collision.valid || !guiData.collision.pieceA) return;
    PieceState& a = *guiData.collision.pieceA;
    if (!fastCalc) a.positions.push_back(a.predictedPosition);

    if (guiData.collision.type == Collision::Type::PIECE) {
        if (!guiData.collision.pieceB) return;
        handlePiecePieceCollision();
        if (!fastCalc) guiData.collision.pieceB->positions.push_back(guiData.collision.pieceB->predictedPosition);
        if (!guiData.collision.firstHitPiece) guiData.collision.firstHitPiece = guiData.collision.pieceB;
    } else if (guiData.collision.type == Collision::Type::LINE) {
        a.calcVelocityPostCollision(guiData.collision.angle);
    } else {
        Point2D delta = {
            guiData.collision.point.y - a.predictedPosition.y,
            -(guiData.collision.point.x - a.predictedPosition.x)
        };
        guiData.collision.angle = -NumberUtils::calcAngle(delta);
        a.calcVelocityPostCollision(guiData.collision.angle);
    }
}

inline void Prediction::handlePiecePieceCollision() const {
    if (!guiData.collision.pieceA || !guiData.collision.pieceB) return;
    PieceState& a = *guiData.collision.pieceA;
    PieceState& b = *guiData.collision.pieceB;
    Point2D rel = a.predictedPosition - b.predictedPosition;
    const double distSq = rel.square();
    if (distSq < 1e-12) return;

    const double invDist = 1.0 / std::sqrt(distSq);
    Point2D normal = rel * invDist;
    const double vaN = a.velocity.x * normal.x + a.velocity.y * normal.y;
    const double vbN = b.velocity.x * normal.x + b.velocity.y * normal.y;
    Point2D va = normal * vaN;
    Point2D vb = normal * vbN;

    a.velocity.x = vb.x - (va.x - a.velocity.x);
    a.velocity.y = vb.y - (va.y - a.velocity.y);
    b.velocity.x = va.x - (vb.x - b.velocity.x);
    b.velocity.y = va.y - (vb.y - b.velocity.y);
#ifdef SIMTEST_DEBUG
    printf("PPCOL a(%d)@(%.2f,%.2f) v=(%.1f,%.1f) <-> b(%d)@(%.2f,%.2f) v=(%.1f,%.1f)\n",
           a.index, a.predictedPosition.x, a.predictedPosition.y, a.velocity.x, a.velocity.y,
           b.index, b.predictedPosition.x, b.predictedPosition.y, b.velocity.x, b.velocity.y);
#endif

    {
        const double rSum = a.getRadius() + b.getRadius();
        const double dist = 1.0 / invDist;
        const double overlap = rSum - dist;
        if (overlap > 0.0) {
            const double push = overlap * 0.5 + 1e-6;
            a.predictedPosition.x += normal.x * push;
            a.predictedPosition.y += normal.y * push;
            b.predictedPosition.x -= normal.x * push;
            b.predictedPosition.y -= normal.y * push;
        }
    }
}

inline void Prediction::PieceState::calcVelocity() {
    if (!isMovingOrSpinning()) return;
    const double radius = getRadius();
    const double relativeY = radius * spin.x - velocity.y;
    const double relativeX = -velocity.x - spin.y * radius;
    const double slidingSpeed = std::sqrt(relativeX * relativeX + relativeY * relativeY);
    const double equilibriumTime = slidingSpeed * 0.0008789064581287676;

    if (slidingSpeed > 1e-9 && equilibriumTime > 1e-5) {
        const double dt = std::min(equilibriumTime, TIME_PER_TICK);
        const double factor = 196.0 * dt / slidingSpeed;
        const double dvx = relativeX * factor;
        const double dvy = relativeY * factor;
        velocity.x += dvx;
        velocity.y += dvy;
        spin.x -= dvy * (2.5 / radius);
        spin.y += dvx * (2.5 / radius);
    }

    if (equilibriumTime < TIME_PER_TICK) {
        const double speed = std::sqrt(velocity.square());
        if (speed > 1e-9) {
            double scale = 1.0 - ((TIME_PER_TICK - equilibriumTime) * 11.0 / speed);
            scale = std::clamp(scale, 0.0, 1.0);
            velocity.x *= scale;
            velocity.y *= scale;
            spin.x = velocity.y / radius;
            spin.y = -velocity.x / radius;
        }
    }

    const double decay = 0.05 * TIME_PER_TICK;
    spin.z = spin.z > 0.0 ? std::max(0.0, spin.z - decay) : std::min(0.0, spin.z + decay);
}

inline void Prediction::PieceState::calcVelocityPostCollision(double angle) {
    constexpr double CUSHION_RESTITUTION = 0.76;
    const double c = std::cos(angle);
    const double s = std::sin(angle);
    const double radius = getRadius();

    const double vx = c * velocity.x - s * velocity.y;
    const double vy = s * velocity.x + c * velocity.y;
    const double spinFactor = vx - radius * spin.z;
    const double absSpinFactor = std::abs(spinFactor);
    const double velocityFactor = absSpinFactor / 2.5;
    const double minSpinFactor = std::min(0.4 * std::abs(vy), velocityFactor);
    const double spinChange = (spinFactor >= 0.0 ? 1.0 : -1.0) * minSpinFactor;

    const double newVx = vx - spinChange / 2.5;
    const double newVy = -CUSHION_RESTITUTION * vy;
    velocity.x = s * newVy + c * newVx;
    velocity.y = c * newVy - s * newVx;

    const double newSpinX = s * spin.x + c * spin.y;
    const double newSpinY = c * spin.x - s * spin.y - spinChange / radius;
    spin.x = s * newSpinX + c * newSpinY;
    spin.y = c * newSpinX - s * newSpinY;
    spin.z -= spinChange / radius;
}

inline bool Prediction::PieceState::isPiecePieceCollision(double* smallestTime, PieceState& other) const {
    if (!smallestTime || !onTable || !other.onTable) return false;
    const double radius = getRadius() + other.getRadius();
    const double dx = other.predictedPosition.x - predictedPosition.x;
    const double dy = other.predictedPosition.y - predictedPosition.y;
    const double dvx = other.velocity.x - velocity.x;
    const double dvy = other.velocity.y - velocity.y;
    const double a = dvx * dvx + dvy * dvy;
    const double b = 2.0 * (dx * dvx + dy * dvy);
    const double c = dx * dx + dy * dy - radius * radius;
    if (a < 1e-12) return false;

    if (b >= 0.0) return false;
    const double disc = b * b - 4.0 * a * c;
    if (disc < 0.0) return false;

    const double root = std::sqrt(disc);
    const double t1 = (-b - root) / (2.0 * a);
    const double t2 = (-b + root) / (2.0 * a);
    double t = std::numeric_limits<double>::max();
    if (t1 >= 0.0) t = t1;
    if (t2 >= 0.0) t = std::min(t, t2);
    if (!std::isfinite(t) || t >= *smallestTime) return false;
    *smallestTime = t;
    return true;
}

inline bool Prediction::PieceState::isPiecePointCollision(double* smallestTime, const Point2D& point) const {
    if (!smallestTime) return false;
    const double vx = velocity.x, vy = velocity.y;
    const double speedSq = vx * vx + vy * vy;
    if (speedSq < 1e-12) return false;

    const double dx = point.x - predictedPosition.x;
    const double dy = point.y - predictedPosition.y;
    const double b = -2.0 * (vx * dx + vy * dy);
    const double c = dx * dx + dy * dy - getRadius() * getRadius();
    const double disc = b * b - 4.0 * speedSq * c;
    if (disc < 0.0) return false;

    const double t = (-b - std::sqrt(disc)) / (2.0 * speedSq);
    if (t < 0.0 || t >= *smallestTime) return false;
    *smallestTime = std::max(0.0, t - 1e-11);
    return true;
}

inline bool Prediction::PieceState::isPieceLineCollision(double* smallestTime, const Point2D& a, const Point2D& b) const {
    if (!smallestTime || velocity.square() < 1e-12) return false;
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    const double len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-12) return false;

    const double nx = dy / len;
    const double ny = -dx / len;
    const double relX = predictedPosition.x - a.x;
    const double relY = predictedPosition.y - a.y;
    const double signedDist = relX * nx + relY * ny;
    const double normalVel = velocity.x * nx + velocity.y * ny;
    if (normalVel >= -1e-12) return false;

    const double t = (getRadius() - signedDist) / normalVel;
    if (t <= 0.0 || t >= *smallestTime) return false;

    const double hitX = predictedPosition.x + velocity.x * t - nx * getRadius();
    const double hitY = predictedPosition.y + velocity.y * t - ny * getRadius();
    const double along = ((hitX - a.x) * dx + (hitY - a.y) * dy) / (len * len);
    if (along < 0.0 || along > 1.0) return false;

    *smallestTime = std::max(0.0, t - 1e-11);
    return true;
}

inline bool Prediction::PieceState::willCollideWithBoard(const double* smallestTime) const {
    if (!smallestTime) return false;
    const auto& frame = getTableShape();
    for (size_t i = 0; i < frame.size(); ++i) {
        const Point2D& a = frame[i];
        const Point2D& b = frame[(i + 1) % frame.size()];
        double lineTime = *smallestTime;
        if (isPieceLineCollision(&lineTime, a, b)) return true;
        double pointTime = *smallestTime;
        if (isPiecePointCollision(&pointTime, a)) return true;
    }
    return false;
}

inline void Prediction::PieceState::determinePieceBoardCollision(void* dataPtr, double* smallestTime) {
    if (!dataPtr || !smallestTime) return;
    auto* data = static_cast<Prediction::SceneData*>(dataPtr);
    const auto& frame = getTableShape();
    double best = *smallestTime;
    int kind = -1;
    Point2D point{};
    double angle = 0.0;

    for (size_t i = 0; i < frame.size(); ++i) {
        const Point2D& a = frame[i];
        const Point2D& b = frame[(i + 1) % frame.size()];
        double lt = best;
        if (isPieceLineCollision(&lt, a, b)) {
            best = lt;
            kind = 0;

            angle = NumberUtils::calcAngle({b.x - a.x, b.y - a.y});
        }
        double pt = best;
        if (isPiecePointCollision(&pt, a)) {
            best = pt;
            kind = 1;
            point = a;
        }
    }

    if (kind == -1) return;
    *smallestTime = best;
    data->collision.valid = true;
    data->collision.pieceA = this;
    data->collision.pieceB = nullptr;
    if (kind == 0) {
        data->collision.type = Prediction::Collision::Type::LINE;
        data->collision.angle = angle;
    } else {
        data->collision.type = Prediction::Collision::Type::POINT;
        data->collision.point = point;
    }
}

inline void Prediction::PieceState::findNextCollision(void* dataPtr, double* time) {
    if (!dataPtr || !time || *time < 0.0 || !onTable) return;
    auto* data = static_cast<Prediction::SceneData*>(dataPtr);

    if (state == CarromPieceEnums::State::DEFAULT || state == CarromPieceEnums::State::IN_POCKET) {
        for (int i = index + 1; i < data->piecesCount; ++i) {
            PieceState& other = data->pieces[i];
            if (!other.onTable) continue;
            if (other.state == CarromPieceEnums::State::DEFAULT || other.state == CarromPieceEnums::State::IN_POCKET) {
                if (isPiecePieceCollision(time, other)) {
                    data->collision.valid = true;
                    data->collision.pieceA = this;
                    data->collision.pieceB = &other;
                    data->collision.type = Collision::Type::PIECE;
                }
            }
        }
    }

    if (!willCollideWithBoard(time)) return;

    const auto& pockets = getPockets();

    bool inPocketZone = false;
    for (int pi = 0; pi < BOARD_POCKETS_COUNT; ++pi) {
        Point2D pd = pockets[pi] - predictedPosition;
        if (pd.square() < POCKET_RADIUS_SQUARE) { inPocketZone = true; break; }
    }
    if (state == CarromPieceEnums::State::DEFAULT || state == CarromPieceEnums::State::IN_POCKET) {
        for (int i = 0; i < BOARD_POCKETS_COUNT; ++i) {
            Point2D d = pockets[i] - predictedPosition;
            const double dsq = d.square();
            if (dsq < POCKET_RADIUS_SQUARE) {
                const double pull = *time * 120.0;
                velocity.x += d.x * pull;
                velocity.y += d.y * pull;

                if (dsq < 4.95) {
                    state = CarromPieceEnums::State::IN_POCKET;
                    pocketIndex = i;
                    Prediction::pocketStatus[i] = true;
                    onTable = false;
                    velocity.nullify();
                    spin.nullify();
                    return;
                }
            }
        }
    }

    if (!inPocketZone) determinePieceBoardCollision(dataPtr, time);
}

inline void Prediction::determineShotState() {
    guiData.shotState = false;
    if (!guiData.collision.firstHitPiece || guiData.piecesCount <= 0 || !guiData.pieces[0].onTable) return;

    const auto target = CarromPieceEnums::Type::ANY;
    const auto first = guiData.collision.firstHitPiece->classification;
    if (target != CarromPieceEnums::Type::ANY && first != target) return;

    for (int i = 1; i < guiData.piecesCount; ++i) {
        if (guiData.pieces[i].originalOnTable != guiData.pieces[i].onTable) {
            guiData.shotState = true;
            return;
        }
    }
}
