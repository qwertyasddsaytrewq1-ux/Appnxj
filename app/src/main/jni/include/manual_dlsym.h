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
#include <string.h>
#include <cstdint>

struct ModuleLookupContext {
    const char* target_lib;
    uintptr_t base_address = 0;
    ElfW(Dyn)* dynamic_section = nullptr;
};

inline int StandardModuleCallback(struct dl_phdr_info *info, size_t size, void *data) {
    auto* context = static_cast<ModuleLookupContext*>(data);

    if (context->target_lib && strstr(info->dlpi_name, context->target_lib)) {
        context->base_address = info->dlpi_addr;
        for (int i = 0; i < info->dlpi_phnum; i++) {
            if (info->dlpi_phdr[i].p_type == PT_DYNAMIC) {
                context->dynamic_section = reinterpret_cast<ElfW(Dyn)*>(info->dlpi_addr + info->dlpi_phdr[i].p_vaddr);
                return 1;
            }
        }
    }
    return 0;
}

class SystemSymbolResolver {
public:

    static constexpr uint32_t HashFNV1a(const char* str) {
        uint32_t hash = 2166136261u;
        while (*str) {
            hash ^= static_cast<uint8_t>(*str++);
            hash *= 16777619u;
        }
        return hash;
    }

    static void* ResolveSymbol(const char* library_name, uint32_t symbol_hash) {
        ModuleLookupContext ctx{library_name};
        dl_iterate_phdr(StandardModuleCallback, &ctx);

        if (!ctx.dynamic_section || !ctx.base_address) return nullptr;

        ElfW(Sym)* symtab = nullptr;
        const char* strtab = nullptr;

        for (ElfW(Dyn)* dyn = ctx.dynamic_section; dyn->d_tag != DT_NULL; ++dyn) {
            if (dyn->d_tag == DT_SYMTAB) {
                symtab = reinterpret_cast<ElfW(Sym)*>(ctx.base_address + dyn->d_un.d_ptr);
            } else if (dyn->d_tag == DT_STRTAB) {
                strtab = reinterpret_cast<const char*>(ctx.base_address + dyn->d_un.d_ptr);
            }
        }

        if (!symtab || !strtab) return nullptr;

        return nullptr;
    }
};
