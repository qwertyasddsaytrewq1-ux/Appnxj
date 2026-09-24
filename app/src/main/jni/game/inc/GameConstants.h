/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

inline constexpr int MAX_PIECES_COUNT = 20;

inline constexpr double STRIKER_RADIUS = 2.10;
inline constexpr double STRIKER_RADIUS_SQUARE = STRIKER_RADIUS * STRIKER_RADIUS;

inline constexpr double PIECE_RADIUS = 1.55;
inline constexpr double PIECE_RADIUS_SQUARE = PIECE_RADIUS * PIECE_RADIUS;

inline constexpr double BOARD_WIDTH = 74.0;
inline constexpr double BOARD_HEIGHT = 74.0;

inline constexpr double BOARD_HALF_WIDTH = BOARD_WIDTH / 2.0;
inline constexpr double BOARD_HALF_HEIGHT = BOARD_HEIGHT / 2.0;

inline constexpr double BOARD_BOUND_LEFT = -BOARD_HALF_WIDTH + PIECE_RADIUS;
inline constexpr double BOARD_BOUND_TOP = -BOARD_HALF_HEIGHT + PIECE_RADIUS;
inline constexpr double BOARD_BOUND_RIGHT = BOARD_HALF_WIDTH - PIECE_RADIUS;
inline constexpr double BOARD_BOUND_BOTTOM = BOARD_HALF_HEIGHT - PIECE_RADIUS;

inline constexpr double BASELINE_Y_OFFSET = 23.5;
inline constexpr double BASELINE_X_BOUND = 22.0;

inline constexpr double TABLE_WIDTH = BOARD_WIDTH;
inline constexpr double TABLE_HEIGHT = BOARD_HEIGHT;
inline constexpr double TABLE_HALF_WIDTH = BOARD_HALF_WIDTH;
inline constexpr double TABLE_HALF_HEIGHT = BOARD_HALF_HEIGHT;
inline constexpr double TABLE_BOUND_LEFT = BOARD_BOUND_LEFT;
inline constexpr double TABLE_BOUND_TOP = BOARD_BOUND_TOP;
inline constexpr double TABLE_BOUND_RIGHT = BOARD_BOUND_RIGHT;
inline constexpr double TABLE_BOUND_BOTTOM = BOARD_BOUND_BOTTOM;

inline constexpr int BOARD_POCKETS_COUNT = 4;
inline constexpr int TABLE_POCKETS_COUNT = BOARD_POCKETS_COUNT;

inline constexpr double POCKET_RADIUS = 3.3;
inline constexpr double POCKET_RADIUS_SQUARE = POCKET_RADIUS * POCKET_RADIUS;

inline constexpr int BOARD_SHAPE_SIZE = 32;
inline constexpr int TABLE_SHAPE_SIZE = BOARD_SHAPE_SIZE;

inline constexpr int MAX_SHOT_RESULT_SIZE = 50000;

inline constexpr double TIME_PER_TICK = 0.002;
inline constexpr double MIN_TIME = 1E-11;

inline constexpr double PI = 3.14159265358979;
inline constexpr double PI_0_5 = PI / 2.0;
inline constexpr double PI_1_5 = PI * 1.5;

inline constexpr double MAX_ANGLE_RADIANS = 360.0 / (180.0 / PI);
inline constexpr double MIN_ANGLE_STEP_RADIANS = 0.0174;

struct Candidate {
    int idx;
    double angle;
    double score;
    int pocketIndex;
    double power;
    bool operator<(const Candidate& other) const {
        return score < other.score;
    }
};
