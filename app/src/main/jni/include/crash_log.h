/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <android/log.h>
#include <dlfcn.h>
#include <chrono>
#include <ctime>
#include <ucontext.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CRASH_TAG "CrashLog"
#define LOGCI(...) __android_log_print(ANDROID_LOG_INFO, CRASH_TAG, __VA_ARGS__)

static const char* LOG_PATHS[] = {
    "/storage/emulated/0/Download/Crash.txt",
    "/sdcard/Download/Crash.txt",
    "/storage/emulated/0/Crash.txt",
    "/sdcard/Crash.txt",
    "/data/local/tmp/Crash.txt",
    "/data/data/com.miniclip.carrom/files/Crash.txt",
    nullptr
};

static const char* g_log_path = nullptr;
static uintptr_t g_libgame_base = 0;

static inline void set_libgame_base(uintptr_t base) {
    g_libgame_base = base;
}

static inline const char* get_log_path() {
    if (g_log_path) return g_log_path;

    for (int i = 0; LOG_PATHS[i]; i++) {
        FILE* f = fopen(LOG_PATHS[i], "w");
        if (f) {
            fclose(f);
            g_log_path = LOG_PATHS[i];
            return g_log_path;
        }
    }
    g_log_path = "/storage/emulated/0/Download/Crash.txt";
    return g_log_path;
}

static inline std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&time_t_now));
    return std::string(buf);
}

static inline void crash_write(const char* msg) {
    const char* path = get_log_path();
    FILE* f = fopen(path, "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }

    if (!f) {
        for (int i = 0; LOG_PATHS[i]; i++) {
            f = fopen(LOG_PATHS[i], "a");
            if (f) {
                fprintf(f, "%s\n", msg);
                fclose(f);
                break;
            }
        }
    }
}

static inline void logf(const char* fmt, ...) {
    char buf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    LOGCI("%s", buf);

    const char* path = get_log_path();
    FILE* f = fopen(path, "a");
    if (f) {
        fprintf(f, "[%s] %s\n", get_timestamp().c_str(), buf);
        fclose(f);
    }
}

#define LOGF(...) logf(__VA_ARGS__)

static inline void dump_logcat_to_file() {
    const char* path = get_log_path();
    FILE* hdr = fopen(path, "a");
    if (hdr) { fprintf(hdr, "\n=== LOGCAT DUMP (at crash time) ===\n"); fclose(hdr); }
    pid_t pid = fork();
    if (pid == 0) {
        int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0666);
        if (fd >= 0) { dup2(fd, STDOUT_FILENO); dup2(fd, STDERR_FILENO); close(fd); }
        execl("/system/bin/logcat", "logcat", "-d", "-v", "threadtime", nullptr);
        _exit(127);
    } else if (pid > 0) {
        int status = 0, waited = 0;
        while (waited < 30) { if (waitpid(pid, &status, WNOHANG) != 0) break; usleep(100000); waited++; }
        if (waited >= 30) { kill(pid, SIGKILL); waitpid(pid, &status, 0); }
    }
    FILE* ftr = fopen(path, "a");
    if (ftr) { fprintf(ftr, "\n=== END LOGCAT DUMP ===\n"); fclose(ftr); }
}

static inline void crash_log_init() {
    const char* path = get_log_path();

    FILE* f = fopen(path, "w");
    if (f) {
        fprintf(f, "========================================\n");
        fprintf(f, "  NativeOverlay V33-V13 — Crash Log\n");
        fprintf(f, "  Log file: %s\n", path);
        fprintf(f, "  Started: %s\n", get_timestamp().c_str());
        fprintf(f, "========================================\n\n");
        fclose(f);
    }
    LOGF("Crash log initialized at %s", path);
}

static inline void crash_signal_handler(int sig, siginfo_t* info, void* context) {

    const char* sig_name = "UNKNOWN";
    switch (sig) {
        case SIGSEGV: sig_name = "SIGSEGV (Segmentation Fault)"; break;
        case SIGBUS:  sig_name = "SIGBUS (Bus Error)"; break;
        case SIGABRT: sig_name = "SIGABRT (Abort)"; break;
        case SIGILL:  sig_name = "SIGILL (Illegal Instruction)"; break;
        case SIGFPE:  sig_name = "SIGFPE (Floating Point Exception)"; break;
        case SIGTRAP: sig_name = "SIGTRAP (Trace/Breakpoint Trap)"; break;
        case SIGPIPE: sig_name = "SIGPIPE (Broken Pipe)"; break;
        case SIGSYS:  sig_name = "SIGSYS (Bad System Call)"; break;
    }

    char buf[4096];
    int pos = 0;

    pos += snprintf(buf + pos, sizeof(buf) - pos, "\n");
    pos += snprintf(buf + pos, sizeof(buf) - pos, "========================================\n");
    pos += snprintf(buf + pos, sizeof(buf) - pos, "  *** CRASH DETECTED ***\n");
    pos += snprintf(buf + pos, sizeof(buf) - pos, "  Time: %s\n", get_timestamp().c_str());
    pos += snprintf(buf + pos, sizeof(buf) - pos, "  Signal: %d (%s)\n", sig, sig_name);
    pos += snprintf(buf + pos, sizeof(buf) - pos, "  PID: %d, TID: %d\n", getpid(), gettid());
    pos += snprintf(buf + pos, sizeof(buf) - pos, "========================================\n\n");

    if (info) {
        pos += snprintf(buf + pos, sizeof(buf) - pos, "Fault Address: %p\n", info->si_addr);
        pos += snprintf(buf + pos, sizeof(buf) - pos, "si_code: %d\n", info->si_code);
        pos += snprintf(buf + pos, sizeof(buf) - pos, "si_errno: %d\n\n", info->si_errno);
    }

    if (context) {
        ucontext_t* uc = (ucontext_t*)context;
        uint64_t pc = uc->uc_mcontext.pc;
        uint64_t lr = uc->uc_mcontext.regs[30];
        uint64_t sp = uc->uc_mcontext.sp;

        pos += snprintf(buf + pos, sizeof(buf) - pos, "=== REGISTER DUMP ===\n");
        pos += snprintf(buf + pos, sizeof(buf) - pos, "PC (Program Counter): 0x%llx\n", (unsigned long long)pc);
        pos += snprintf(buf + pos, sizeof(buf) - pos, "LR (Link Register):   0x%llx\n", (unsigned long long)lr);
        pos += snprintf(buf + pos, sizeof(buf) - pos, "SP (Stack Pointer):   0x%llx\n", (unsigned long long)sp);
        pos += snprintf(buf + pos, sizeof(buf) - pos, "FP (Frame Pointer):   0x%llx\n", (unsigned long long)uc->uc_mcontext.regs[29]);
        pos += snprintf(buf + pos, sizeof(buf) - pos, "\n");

        for (int i = 0; i < 31; i++) {
            pos += snprintf(buf + pos, sizeof(buf) - pos, "X%-2d: 0x%llx", i, (unsigned long long)uc->uc_mcontext.regs[i]);
            if ((i % 4) == 3) {
                pos += snprintf(buf + pos, sizeof(buf) - pos, "\n");
            } else {
                pos += snprintf(buf + pos, sizeof(buf) - pos, "  ");
            }
        }
        pos += snprintf(buf + pos, sizeof(buf) - pos, "\n\n");

        if (g_libgame_base && pc >= g_libgame_base) {
            uint64_t rel_offset = pc - g_libgame_base;
            pos += snprintf(buf + pos, sizeof(buf) - pos, "=== LIBGAME OFFSET ===\n");
            pos += snprintf(buf + pos, sizeof(buf) - pos, "libgame base: 0x%llx\n", (unsigned long long)g_libgame_base);
            pos += snprintf(buf + pos, sizeof(buf) - pos, "PC offset:    0x%llx (libgame + 0x%llx)\n", (unsigned long long)rel_offset, (unsigned long long)rel_offset);
            if (lr >= g_libgame_base) {
                uint64_t lr_offset = lr - g_libgame_base;
                pos += snprintf(buf + pos, sizeof(buf) - pos, "LR offset:    0x%llx (libgame + 0x%llx)\n", (unsigned long long)lr_offset, (unsigned long long)lr_offset);
            }
            pos += snprintf(buf + pos, sizeof(buf) - pos, "\n");
        }

        Dl_info dlinfo;
        if (dladdr((void*)pc, &dlinfo)) {
            pos += snprintf(buf + pos, sizeof(buf) - pos, "=== SYMBOL INFO (PC) ===\n");
            pos += snprintf(buf + pos, sizeof(buf) - pos, "Library:  %s\n", dlinfo.dli_fname ? dlinfo.dli_fname : "unknown");
            pos += snprintf(buf + pos, sizeof(buf) - pos, "Symbol:   %s\n", dlinfo.dli_sname ? dlinfo.dli_sname : "unknown");
            pos += snprintf(buf + pos, sizeof(buf) - pos, "Base:     %p\n", dlinfo.dli_fbase);
            pos += snprintf(buf + pos, sizeof(buf) - pos, "SymAddr:  %p\n", dlinfo.dli_saddr);
            pos += snprintf(buf + pos, sizeof(buf) - pos, "\n");
        }
    }

    pos += snprintf(buf + pos, sizeof(buf) - pos, "=== STACK TRACE ===\n");
    if (context) {
        ucontext_t* uc = (ucontext_t*)context;
        uint64_t* sp = (uint64_t*)uc->uc_mcontext.sp;
        uint64_t fp = uc->uc_mcontext.regs[29];

        for (int i = 0; i < 32 && fp; i++) {
            uint64_t* fp_ptr = (uint64_t*)fp;

            uint64_t ret_addr = fp_ptr[1];
            if (ret_addr == 0) break;

            Dl_info dlinfo;
            const char* sym = "unknown";
            const char* lib = "unknown";
            if (dladdr((void*)ret_addr, &dlinfo)) {
                sym = dlinfo.dli_sname ? dlinfo.dli_sname : "unknown";
                lib = dlinfo.dli_fname ? dlinfo.dli_fname : "unknown";
            }

            uint64_t offset = g_libgame_base && ret_addr >= g_libgame_base ? ret_addr - g_libgame_base : 0;
            pos += snprintf(buf + pos, sizeof(buf) - pos, "  #%d  0x%llx", i, (unsigned long long)ret_addr);
            if (offset) {
                pos += snprintf(buf + pos, sizeof(buf) - pos, " (libgame+0x%llx)", (unsigned long long)offset);
            }
            pos += snprintf(buf + pos, sizeof(buf) - pos, " [%s] %s\n", lib, sym);

            fp = fp_ptr[0];
        }
    }
    pos += snprintf(buf + pos, sizeof(buf) - pos, "\n");

    pos += snprintf(buf + pos, sizeof(buf) - pos, "=== STATE ===\n");
    pos += snprintf(buf + pos, sizeof(buf) - pos, "libgame base: 0x%llx\n", (unsigned long long)g_libgame_base);
    pos += snprintf(buf + pos, sizeof(buf) - pos, "sharedCarromGameManager.instance: 0x%llx\n",
                    (unsigned long long)0);
    pos += snprintf(buf + pos, sizeof(buf) - pos, "\n");

    pos += snprintf(buf + pos, sizeof(buf) - pos, "========================================\n");
    pos += snprintf(buf + pos, sizeof(buf) - pos, "  *** END CRASH REPORT ***\n");
    pos += snprintf(buf + pos, sizeof(buf) - pos, "========================================\n");

    const char* path = get_log_path();
    FILE* f = fopen(path, "a");
    if (!f) {

        for (int i = 0; LOG_PATHS[i]; i++) {
            f = fopen(LOG_PATHS[i], "a");
            if (f) {
                g_log_path = LOG_PATHS[i];
                break;
            }
        }
    }

    if (f) {
        fprintf(f, "%s", buf);
        fclose(f);
    }

    dump_logcat_to_file();

    LOGCI("*** CRASH: %s (sig=%d) addr=%p ***", sig_name, sig, info ? info->si_addr : nullptr);
    LOGCI("Crash log written to %s", g_log_path);

    signal(sig, SIG_DFL);
    raise(sig);
}

static inline void install_crash_handlers() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = crash_signal_handler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGBUS,  &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGILL,  &sa, nullptr);
    sigaction(SIGFPE,  &sa, nullptr);
    sigaction(SIGTRAP, &sa, nullptr);
    sigaction(SIGPIPE, &sa, nullptr);
    sigaction(SIGSYS,  &sa, nullptr);

    sigaction(SIGQUIT, &sa, nullptr);

    LOGF("Crash signal handlers installed (8 signals)");
}
