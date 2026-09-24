/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include <elf.h>
#include <link.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/types.h>
#include <csignal>
#include <csetjmp>
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <string>
#include <jni.h>
#include <android/log.h>

#include "obfy/instr.h"
#include "oxorany/oxorany.h"
#include "random_names.h"
#include "includes.h"
#include "stacktrace.h"
#include "imgui/inc/persistence.h"

#undef GET_N
#include "obfuscation.h"
#define GET_N(n) _GET_N_STR(n)
#define _GET_N_STR(n) "_ZN11ObfuscatedN" #n "Ev"

#define MY_LOG_TAG "angousana"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  MY_LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MY_LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  MY_LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MY_LOG_TAG, __VA_ARGS__)

#define INLINE __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline))
#define EXPORT extern "C" __attribute__((visibility("default")))
#define EXPORT_MANGLED extern "C"

#define ifl(cond) if ([&](){ bool b = (cond); if (b) LOGI(#cond); return b; }())

static INLINE void inline_strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

static INLINE unsigned long inline_strtoul(const char* nptr, char** endptr) {
    unsigned long result = 0;
    while (*nptr) {
        char c = *nptr++;
        if (c >= '0' && c <= '9') result = result * 16 + (c - '0');
        else if (c >= 'a' && c <= 'f') result = result * 16 + (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') result = result * 16 + (c - 'A' + 10);
        else break;
    }
    if (endptr) *endptr = (char*)nptr;
    return result;
}

static INLINE size_t inline_strlen(const char* str) {
    const char* s;
    for (s = str; *s; ++s);
    return (s - str);
}

static INLINE char* inline_strncat(char* dest, const char* src, size_t n) {
    char* p = dest;
    while (*p != 0) p++;
    while (n > 0 && *src != 0) {
        *p++ = *src++;
        n--;
    }
    *p = 0;
    return dest;
}

static INLINE int inline_strcmp(const char* s1, const char* s2) {
OBF_BEGIN
    WHILE (*s1 == *s2++) IF (*s1++ == 0) RETURN(0); ENDIF ENDWHILE
    RETURN(*(unsigned char*)s1 - *(unsigned char*)--s2);
OBF_END
}

static INLINE int inline_strncmp(const char* s1, const char* s2, size_t n) {
    unsigned char u1, u2;
    while (n-- > 0) {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2) return u1 - u2;
        if (u1 == '\0') return 0;
    }
    return 0;
}

static INLINE char* inline_strstr(const char* s, const char* find) {
    char c, sc;
    size_t len;
    if ((c = *find++) != 0) {
        len = inline_strlen(find);
        do {
            do {
                if ((sc = *s++) == 0) return NULL;
            } while (sc != c);
        } while (inline_strncmp(s, find, len) != 0);
        s--;
    }
    return (char*)s;
}

#define _dgo27 _o53::h525_()

namespace _o53 {
    using da7 = unsigned long long;
    using b96 = unsigned long long;

    constexpr b96 h525_(b96 seed = 0) {
        b96 gd926 = static_cast<int>(__TIME__[7]) + static_cast<int>(__TIME__[6]) * 10 +
                    static_cast<int>(__TIME__[4]) * 60 + static_cast<int>(__TIME__[3]) * 600 +
                    static_cast<int>(__TIME__[1]) * 3600 + static_cast<int>(__TIME__[0]) * 36000;
        if (seed) gd926 += seed;
        gd926 ^= (gd926 >> 33);
        gd926 *= 0xff51afd7ed558ccd;
        gd926 ^= (gd926 >> 33);
        gd926 *= 0xc4ceb9fe1a85ec53;
        gd926 ^= (gd926 >> 33);
        gd926 |= 0x0101010101010101ull;
        return gd926;
    }

    constexpr void ew85(char *e0r_, da7 tag752, b96 gd926) {
        for (da7 i = 0; i < tag752; i++) {
            e0r_[i] ^= char(gd926 >> ((i % 8) * 8));
        }
    }

    template<da7 N, b96 ky9>
    class b9_ {
    public:
        constexpr explicit b9_(const char *e0r_) {
            for (da7 i = 0; i < N; i++) {
                h7982[i] = e0r_[i];
            }
            ew85(h7982, N, ky9);
        }

        [[nodiscard]] constexpr const char *e0r_() const {
            return &h7982[0];
        }

        [[maybe_unused]] [[nodiscard]] constexpr da7 tag752() const {
            return N;
        }

        [[maybe_unused]] [[nodiscard]] constexpr b96 gd926() const {
            return ky9;
        }

    private:
        char h7982[N]{};
    };

    template<da7 N, b96 ky9>
    class _3z {
    public:
        explicit _3z(const b9_<N, ky9> &b9_) {
            for (da7 i = 0; i < N; i++) {
                h7982[i] = b9_.e0r_()[i];
            }
        }

        ~_3z() {
            for (da7 i = 0; i < N; i++) {
                h7982[i] = 0;
            }
        }

        char *chr() {
            wync9();
            return h7982;
        }

        std::string str() {
            wync9();
            return h7982;
        }

        void wync9() {
            if (_yr9) {
                ew85(h7982, N, ky9);
                _yr9 = false;
            }
        }

        [[maybe_unused]] void jfp1o() {
            if (!_yr9) {
                ew85(h7982, N, ky9);
                _yr9 = true;
            }
        }

        [[maybe_unused]] [[nodiscard]] bool h7a9mys() const {
            return _yr9;
        }

    private:
        char h7982[N]{};
        bool _yr9{true};
    };

    template<da7 N, b96 ky9 = _dgo27>
    constexpr auto j62_(const char(&e0r_)[N]) {
        return b9_<N, ky9>(e0r_);
    }
}

#define OO(e0r_) []() -> _o53::_3z<sizeof(e0r_)/sizeof(e0r_[0]), _dgo27>& { constexpr auto n = sizeof(e0r_)/sizeof(e0r_[0]); constexpr auto b9_ = _o53::j62_<n, _dgo27>(e0r_); static auto _3z = _o53::_3z<n, _dgo27>(b9_); return _3z; }()

static uint32_t gnu_hash(const char *s) {
    uint32_t h = 5381;
    for (unsigned char c = *s; c != '\0'; c = *++s)
        h = h * 33 + c;
    return h;
}

inline void deadbeef() {}

struct ModuleInfo {
    const char* name;
    uintptr_t base_addr;
    ElfW(Dyn)* dynamic;
};

INLINE int find_module_callback(struct dl_phdr_info *info, size_t size, void *data) {
    ModuleInfo* target = (ModuleInfo*)data;

    if (target->base_addr != 0) {
        for (int i = 0; i < info->dlpi_phnum; i++) {
            uintptr_t start = info->dlpi_addr + info->dlpi_phdr[i].p_vaddr;
            uintptr_t end = start + info->dlpi_phdr[i].p_memsz;
            if (target->base_addr >= start && target->base_addr < end) {
                target->base_addr = info->dlpi_addr;
                for (int j = 0; j < info->dlpi_phnum; j++) {
                    if (info->dlpi_phdr[j].p_type == PT_DYNAMIC) {
                        target->dynamic = (ElfW(Dyn)*)(info->dlpi_addr + info->dlpi_phdr[j].p_vaddr);
                        return 1;
                    }
                }
                return 1;
            }
        }
    }
    else if (target->name && strstr(info->dlpi_name, target->name)) {
        target->base_addr = info->dlpi_addr;
        for (int j = 0; j < info->dlpi_phnum; j++) {
            if (info->dlpi_phdr[j].p_type == PT_DYNAMIC) {
                target->dynamic = (ElfW(Dyn)*)(info->dlpi_addr + info->dlpi_phdr[j].p_vaddr);
                return 1;
            }
        }
    }
    return 0;
}

class ManualLookup {
public:
    static void* dlsym(uintptr_t module_marker, const char* symbol_name) {
        ModuleInfo info = {0};
        info.base_addr = module_marker;

        dl_iterate_phdr(find_module_callback, &info);

        if (!info.dynamic) return nullptr;

        ElfW(Sym)* symtab = nullptr;
        const char* strtab = nullptr;
        uint32_t* gnu_hash_table = nullptr;

        for (ElfW(Dyn)* dyn = info.dynamic; dyn->d_tag != DT_NULL; ++dyn) {
            switch (dyn->d_tag) {
                case DT_SYMTAB: symtab = (ElfW(Sym)*)(info.base_addr + dyn->d_un.d_ptr); break;
                case DT_STRTAB: strtab = (const char*)(info.base_addr + dyn->d_un.d_ptr); break;
                case DT_GNU_HASH: gnu_hash_table = (uint32_t*)(info.base_addr + dyn->d_un.d_ptr); break;
            }
        }

        if (!symtab || !strtab || !gnu_hash_table) return nullptr;

        uint32_t nbuckets = gnu_hash_table[0];
        uint32_t symndx = gnu_hash_table[1];
        uint32_t maskwords = gnu_hash_table[2];
        uint32_t shift2 = gnu_hash_table[3];

        ElfW(Addr)* bloom = (ElfW(Addr)*)(gnu_hash_table + 4);
        uint32_t* buckets = (uint32_t*)(bloom + maskwords);
        uint32_t* chain = buckets + nbuckets;

        uint32_t hash = gnu_hash(symbol_name);
        uint32_t elf_class_bits = sizeof(ElfW(Addr)) * 8;
        uint32_t word = bloom[(hash / elf_class_bits) % maskwords];
        uint32_t mask = 0 | (uint32_t)1 << (hash % elf_class_bits)
                          | (uint32_t)1 << ((hash >> shift2) % elf_class_bits);

        if ((word & mask) != mask) return nullptr;

        uint32_t sym_idx = buckets[hash % nbuckets];
        if (sym_idx < symndx) return nullptr;

        while (true) {
            uint32_t chain_hash = chain[sym_idx - symndx];

            if ((chain_hash >> 1) == (hash >> 1) &&
                strcmp(strtab + symtab[sym_idx].st_name, symbol_name) == 0) {
                return (void*)(info.base_addr + symtab[sym_idx].st_value);
            }

            if (chain_hash & 1) break;
            sym_idx++;
        }

        return nullptr;
    }

    template <typename Callback>
    static INLINE void walk_exported_symbols(uintptr_t module_marker, Callback callback) {
        static ModuleInfo info = {0};
        static uintptr_t last_marker = 0;

        if (last_marker != module_marker) {
            info = {0};
            info.base_addr = module_marker;
            dl_iterate_phdr(find_module_callback, &info);
            if (info.dynamic) last_marker = module_marker;
        }

        if (!info.dynamic) return;

        ElfW(Sym)* symtab = nullptr;
        const char* strtab = nullptr;
        uint32_t* gnu_hash_table = nullptr;

        for (ElfW(Dyn)* dyn = info.dynamic; dyn->d_tag != DT_NULL; ++dyn) {
            switch (dyn->d_tag) {
                case DT_SYMTAB: symtab = (ElfW(Sym)*)(info.base_addr + dyn->d_un.d_ptr); break;
                case DT_STRTAB: strtab = (const char*)(info.base_addr + dyn->d_un.d_ptr); break;
                case DT_GNU_HASH: gnu_hash_table = (uint32_t*)(info.base_addr + dyn->d_un.d_ptr); break;
            }
        }

        if (!symtab || !strtab || !gnu_hash_table) return;

        uint32_t nbuckets = gnu_hash_table[0];
        uint32_t symndx = gnu_hash_table[1];

        ElfW(Addr)* bloom = (ElfW(Addr)*)(gnu_hash_table + 4);
        uint32_t* buckets = (uint32_t*)(bloom + static_cast<uint32_t>(gnu_hash_table[2]));
        uint32_t* chain = buckets + nbuckets;

        for (uint32_t i = 0; i < nbuckets; ++i) {
            uint32_t sym_idx = buckets[i];
            if (sym_idx == 0) continue;

            while (true) {
                const char* sym_name = strtab + symtab[sym_idx].st_name;
                void* sym_addr = (void*)(info.base_addr + symtab[sym_idx].st_value);

                if (callback(sym_name, sym_addr)) return;

                uint32_t chain_hash = chain[sym_idx - symndx];
                if (chain_hash & 1) break;
                sym_idx++;
            }
        }
    }

    static consteval uint32_t _fnv1a(const char* str) {
        uint32_t hash = 2166136261u;
        for (int i = 0; str[i] != 0; ++i) {
            hash ^= (uint8_t)str[i];
            hash *= 16777619u;
        }
        return hash;
    }

    static INLINE uint32_t fnv1a(const char* str) {
    OBF_BEGIN
        int i;
        uint32_t hash;
        V(hash) = O(2166136261u);
        FOR (V(i) = N(0), V(str[i]) != N(0), ++V(i))
            V(hash) ^= (uint8_t)V(str[i]);
            V(hash) *= O(16777619u);
        ENDFOR
        RETURN(hash);
    OBF_END
    }

    static INLINE void* dlsym_hashed(uint32_t target_hash) {
        void* found_addr = nullptr;
        walk_exported_symbols((uintptr_t)&deadbeef, [&](const char* name, void* addr) -> bool {
            if (fnv1a(name) == target_hash) {
                found_addr = addr;
                return true;
            }
            return false;
        });
        return found_addr;
    }

    static INLINE void dlsym_call_hashed(uint32_t target_hash) {
        walk_exported_symbols((uintptr_t)&deadbeef, [&](const char* name, void* addr) -> bool {
            if (fnv1a(name) == target_hash) {
                if (addr) ((void(*)())addr)();
                return true;
            }
            return false;
        });
    }
};

#define HASHF(x) O(ManualLookup::_fnv1a(x))
#define CALL(x) ManualLookup::dlsym_call_hashed(HASHF(GET_N(x)))
#define GET_S(x) ManualLookup::dlsym_hashed(HASHF(GET_N(x)))

using ptr = uintptr_t;

extern std::string rpart(const std::string& str, char c);
extern std::string whois(const void* address, bool include_offset);
extern std::string whois(ptr address, bool include_offset);

static sigjmp_buf jump_buffer;
static volatile int jump_buffer_active = 0;
static bool segv_handler_set = false;

inline void SignalTraceHandler(int sig) {
    LOGI("SignalTraceHandler %d", sig);
    log_stacktrace(100);
    signal(sig, SIG_DFL);
    raise(sig);
}

inline void SetupSignalTraceHandler() {
    std::signal(SIGABRT, SignalTraceHandler);
    std::signal(SIGILL,  SignalTraceHandler);
    std::signal(SIGFPE,  SignalTraceHandler);
    std::signal(SIGBUS,  SignalTraceHandler);
    std::signal(SIGSYS,  SignalTraceHandler);
    std::signal(SIGTRAP, SignalTraceHandler);
    std::signal(SIGXCPU, SignalTraceHandler);
    std::signal(SIGXFSZ, SignalTraceHandler);
}

inline void segv_handler(int sig) {

    if (jump_buffer_active) {
        jump_buffer_active = 0;
        siglongjmp(jump_buffer, 1);
    }
    LOGI("SEGV caught (non-DrawESP crash)");
    log_stacktrace();
    if (IsInStack("libwolf.so") || IsInStack("[!dladdr]")) {
        LOGI("Wolf founded");
        sleep(10000);
        return;
    }
    signal(SIGSEGV, SIG_DFL);
    raise(SIGSEGV);
}

inline void setup_global_segv_handler() {
    struct sigaction new_action{};
    new_action.sa_handler = segv_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGSEGV, &new_action, nullptr);
    segv_handler_set = true;
}

inline bool is_segv_handler_active() {
    struct sigaction current_action;
    sigaction(SIGSEGV, nullptr, &current_action);
    return current_action.sa_handler == segv_handler;
}

inline void JUMPTHING() {
    ptr smth = 0;
    jump_buffer_active = 1;
    if (!sigsetjmp(jump_buffer, 1)) {
        auto r = (ptr*)(0x0);
        if (r) smth = *r;
    }
    jump_buffer_active = 0;
    LOGI("smth: %p", (void*)smth);
}

#define FA(type, address) (type*)address
#define FO(type, address) (*(type*)(address))
#define F(type, address) (*(type*)(address))
#define M(type, address, ...) ((type(*)(__VA_ARGS__))(address))

#define IMPL_1(cnt) \
    EXPORT NOINLINE void GET_L(cnt)() { \
        volatile int x = O(0); V(x)++; \
        volatile int i = 0; i++; \
    }

#define IMPL_2(cnt) \
    EXPORT NOINLINE void GET_L(cnt)() { \
        volatile int x = O(0); V(x)--; \
        volatile int i = 0; i--; \
    }

#define IMPL_3(cnt) \
    EXPORT NOINLINE void* GET_L(cnt)() { \
        volatile int i = O(10); \
        OBF_BEGIN WHILE(V(i) > 0) V(i)--; ENDWHILE OBF_END \
        volatile int x = 10; while(x > 0) x--; \
        return nullptr; \
    }

#define IMPL_4(cnt) \
    EXPORT NOINLINE void GET_L(cnt)() { \
        volatile int x = O(1), y = O(1); V(x) += V(y); \
        volatile int a = 1, b = 1; a += b; \
    }

#define IMPL_5(cnt) \
    EXPORT NOINLINE void GET_L(cnt)() { \
        volatile int x = O(1), y = O(1); V(x) -= V(y); \
        __asm__ __volatile__("nop"); \
    }

#define DEFINE_DECOY_1 IMPL_1(__COUNTER__)
#define DEFINE_DECOY_2 IMPL_2(__COUNTER__)
#define DEFINE_DECOY_3 IMPL_3(__COUNTER__)
#define DEFINE_DECOY_4 IMPL_4(__COUNTER__)
#define DEFINE_DECOY_5 IMPL_5(__COUNTER__)

#define DEFINE_DECOYS_3 DEFINE_DECOY_5 DEFINE_DECOY_2 DEFINE_DECOY_4
#define DEFINE_DECOYS_5 DEFINE_DECOYS_3 DEFINE_DECOY_3 DEFINE_DECOY_1
#define DEFINE_DECOYS_10 DEFINE_DECOYS_5 DEFINE_DECOYS_3 DEFINE_DECOY_4 DEFINE_DECOY_2
#define DEFINE_DECOYS_25 DEFINE_DECOYS_10 DEFINE_DECOYS_10 DEFINE_DECOY_1 DEFINE_DECOY_2 DEFINE_DECOY_3 DEFINE_DECOY_4 DEFINE_DECOY_5
#define DEFINE_DECOYS_50 DEFINE_DECOYS_25 DEFINE_DECOYS_25
#define DEFINE_DECOYS_100 DEFINE_DECOYS_50 DEFINE_DECOYS_50
