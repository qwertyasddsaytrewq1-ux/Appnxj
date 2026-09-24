/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <typeindex>

namespace rules {
    class IRule;
    enum CarromFoulReason : int;
}

enum CarromPieceClassification : int;
class CarromPlayer;

struct Ruleset {
    int field_0x0;
    int field_0x4;
    std::string field_0x8;
    int field_0x20;

    std::map<std::type_index, rules::IRule *> field_0x28;
    std::vector<rules::CarromFoulReason> field_0x40;
    bool field_0x58;

    double field_0x60;
    int field_0x68;

    void* field_0x70;
    void* field_0x78;
    int field_0x80;

    void* field_0x88;
    int field_0x90;

    std::vector<CarromPieceClassification> field_0x98;
    std::vector<CarromPieceClassification> field_0xB0;
    std::vector<CarromPieceClassification> field_0xC8;
    std::vector<CarromPlayer *> field_0xE0;
    int field_0xF8;
    int field_0xFC;
    int field_0x100;
    int field_0x104;
    bool field_0x108;

    int field_0x10C;
    char field_0x110;
    char field_0x111;
    bool field_0x112;
    bool field_0x113;
    bool field_0x114;

    int field_0x118;
    int field_0x11C;
    char field_0x120;

    int field_0x124;
    bool field_0x128;

    int field_0x12C;
};
