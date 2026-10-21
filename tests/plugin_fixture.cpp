#include <dlfcn.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "ext/steam/isteamapps.h"

int main(int argc, char** argv) {
    void* process = dlopen(nullptr, RTLD_NOW);
    auto init = reinterpret_cast<bool (*)()>(dlsym(process, "SteamAPI_Init"));
    if (!init) return 2;
    const bool expected = argc < 2 || std::strcmp(argv[1], "--failure");
    if (init() != expected) return 3;
    if (!expected) return 0;
    auto calls = reinterpret_cast<int (*)()>(dlsym(RTLD_DEFAULT, "MockInitCalls"));
    auto apps = reinterpret_cast<ISteamApps* (*)()>(dlsym(process, "SteamApps"));
    // The mock is loaded by the plugin, without a linked or preloaded API.
    if (!calls || calls() != 1 || !apps) return 4;
    auto original = apps();
    const auto count = argc == 2 ? std::atoi(argv[1]) : 88;
    if (!original || original->GetDLCCount() != count || original->GetAppBuildId() != 31415) return 5;
    std::cout << "Plugin configuration and original Steam API forwarding passed\n";
}
