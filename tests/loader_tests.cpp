#include <dlfcn.h>

int main() {
    void* handle = dlopen(nullptr, RTLD_NOW);
    if (!handle) return 1;
    const bool found = dlsym(handle, "malloc") != nullptr;
    dlclose(handle);
    return found ? 0 : 2;
}
