/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include <cstddef>
#include <string>
#include <utility>

namespace Obfuscation {

constexpr uint64_t GenerateSeed() {
    uint64_t seed = 0;
    for (char c : __TIME__) {
        seed = (seed * 131) + static_cast<uint64_t>(c);
    }
    return seed;
}

template <size_t N, uint64_t Key>
class EncryptedString {
public:
    constexpr EncryptedString(const char (&str)[N]) {
        for (size_t i = 0; i < N; ++i) {
            data[i] = str[i] ^ static_cast<char>((Key >> ((i % 8) * 8)) & 0xFF);
        }
    }

    void Decrypt(char* outBuffer) const {
        for (size_t i = 0; i < N; ++i) {
            outBuffer[i] = data[i] ^ static_cast<char>((Key >> ((i % 8) * 8)) & 0xFF);
        }
    }

    constexpr size_t Size() const { return N; }

private:
    char data[N]{};
};

template <size_t N, uint64_t Key>
class RuntimeString {
public:
    RuntimeString(const EncryptedString<N, Key>& enc) {
        enc.Decrypt(buffer);
    }

    ~RuntimeString() {
        for (size_t i = 0; i < N; ++i) {
            buffer[i] = 0;
        }
    }

    const char* c_str() const { return buffer; }
    std::string str() const { return std::string(buffer); }

private:
    char buffer[N]{};
};

}

#define OBFUSCATE_STR(str) \
    []() -> std::string { \
        constexpr static auto enc = Obfuscation::EncryptedString<sizeof(str), Obfuscation::GenerateSeed()>(str); \
        auto dec = Obfuscation::RuntimeString<sizeof(str), Obfuscation::GenerateSeed()>(enc); \
        return dec.str(); \
    }()
