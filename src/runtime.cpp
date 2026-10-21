#include "runtime.hpp"
#include <dlfcn.h>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <vector>
#include "spdlog/spdlog.h"

extern void* (*real_dlsym)(void*, const char*);
extern void ensure_realdlsym();

namespace eu4dlc {
namespace {
std::string parent(const std::string& path) {
    const auto slash = path.rfind('/');
    return slash == std::string::npos ? "." : path.substr(0, slash);
}

std::string game_directory() {
    std::vector<char> path(4096);
    const auto length = readlink("/proc/self/exe", path.data(), path.size());
    if (length <= 0 || static_cast<std::size_t>(length) == path.size())
        throw std::runtime_error("Cannot locate the game executable");
    return parent(std::string(path.data(), static_cast<std::size_t>(length)));
}
}

std::string config_path() {
    if (const auto value = std::getenv("CREAM_CONFIG_PATH"))
        if (*value) return value;
    Dl_info info{};
    if (dladdr(reinterpret_cast<void*>(&config_path), &info) && info.dli_fname) {
        char* resolved = realpath(info.dli_fname, nullptr);
        if (resolved) {
            const auto directory = parent(resolved);
            std::free(resolved);
            const auto packaged = directory + "/creamlinux/cream_api.ini";
            if (std::ifstream(packaged).good()) return packaged;
            const auto adjacent = directory + "/cream_api.ini";
            if (std::ifstream(adjacent).good()) return adjacent;
        }
    }
    // Keep the original game-root configuration location for existing users.
    return game_directory() + "/cream_api.ini";
}

void* original_symbol(const char* name) {
    ensure_realdlsym();
    dlerror();
    const auto configured = std::getenv("CREAM_STEAM_API");
    if (!configured || !*configured)
        if (void* symbol = real_dlsym(RTLD_NEXT, name)) return symbol;
    // EU4 can load its Steam library with RTLD_LOCAL. Promote the original
    // library only when a forwarded symbol is unavailable in the global scope.
    static std::once_flag once;
    static void* original = nullptr;
    std::call_once(once, [] {
        try {
            const auto configured = std::getenv("CREAM_STEAM_API");
            const auto path = configured && *configured ? std::string(configured) :
                game_directory() + "/libsteam_api.so";
            original = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (!original) {
                const char* error = dlerror();
                spdlog::error("Cannot load original Steam API {}: {}", path, error ? error : "unknown error");
            }
        } catch (const std::exception& error) {
            spdlog::error("Cannot locate original Steam API: {}", error.what());
        }
    });
    dlerror();
    return original ? real_dlsym(original, name) : nullptr;
}
}
