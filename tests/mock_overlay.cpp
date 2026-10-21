#include <dlfcn.h>
#include <unistd.h>

// Model the Steam overlay's RTLD_NEXT lookup for its dlopen interposer.
extern "C" void* dlopen(const char* path, int flags) {
    using Open = void* (*)(const char*, int);
    auto next = reinterpret_cast<Open>(dlsym(RTLD_NEXT, "dlopen"));
    if (!next || next == &dlopen) _exit(86);
    return next(path, flags);
}
