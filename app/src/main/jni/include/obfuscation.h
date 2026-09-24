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
#include <cstdint>

#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define EXPORT_SYMBOL extern "C" __attribute__((visibility("default")))
#define EXPORT extern "C" __attribute__((visibility("default")))

#define GET_N(n) _GET_N_STR(n)
#define _GET_N_STR(n) "_ZN11ObfuscatedN" #n "Ev"

namespace NativeUtils {

    ALWAYS_INLINE char* StringCopy(char* dest, const char* src) {
        char* ptr = dest;
        while ((*ptr++ = *src++));
        return dest;
    }

    ALWAYS_INLINE size_t StringLength(const char* str) {
        const char* s = str;
        while (*s) ++s;
        return static_cast<size_t>(s - str);
    }

    ALWAYS_INLINE uint64_t ParseHexToUint64(const char* nptr, char** endptr = nullptr) {
        uint64_t result = 0;
        while (*nptr) {
            char c = *nptr;
            if (c >= '0' && c <= '9')      result = (result << 4) | (c - '0');
            else if (c >= 'a' && c <= 'f') result = (result << 4) | (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') result = (result << 4) | (c - 'A' + 10);
            else break;
            nptr++;
        }
        if (endptr) *endptr = const_cast<char*>(nptr);
        return result;
    }

    ALWAYS_INLINE int StringCompare(const char* s1, const char* s2) {
        while (*s1 && (*s1 == *s2)) {
            s1++;
            s2++;
        }
        return static_cast<unsigned char>(*s1) - static_cast<unsigned char>(*s2);
    }

    ALWAYS_INLINE char* StringSearch(const char* haystack, const char* needle) {
        if (!*needle) return const_cast<char*>(haystack);

        for (; *haystack; ++haystack) {
            if (*haystack == *needle) {
                const char* h = haystack + 1;
                const char* n = needle + 1;
                while (*h && *n && *h == *n) {
                    h++;
                    n++;
                }
                if (!*n) return const_cast<char*>(haystack);
            }
        }
        return nullptr;
    }
}
