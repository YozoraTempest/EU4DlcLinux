#include "ext/steam/isteamapps.h"
#include <dlfcn.h>
#include <cstring>
#include <iostream>

extern "C" bool SteamAPI_Init();
extern "C" int MockInitCalls();
extern "C" ISteamApps* SteamApps();
extern "C" void* SteamInternal_FindOrCreateUserInterface(HSteamUser, const char*);
extern "C" void* SteamInternal_CreateInterface(const char*);
extern "C" int SteamAPI_ISteamApps_GetDLCCount(ISteamApps*);
extern "C" bool SteamAPI_ISteamApps_BIsDlcInstalled(ISteamApps*, AppId_t);
extern "C" bool SteamAPI_ISteamApps_BIsSubscribedApp(ISteamApps*, AppId_t);
extern "C" bool SteamAPI_ISteamApps_BGetDLCDataByIndex(ISteamApps*, int, AppId_t*, bool*, char*, int);

#define CHECK(value) do { if (!(value)) { std::cerr << "Check failed at line " << __LINE__ << ": " #value "\n"; return 1; } } while (false)

int main(int argc, char** argv) {
    CHECK(argc == 2);
    if (!std::strcmp(argv[1], "--bad-config")) {
        CHECK(!SteamAPI_Init());
        CHECK(MockInitCalls() == 0);
        return 0;
    }
    CHECK(SteamAPI_Init());
    CHECK(SteamAPI_Init());
    CHECK(MockInitCalls() == 2);
    auto apps = static_cast<ISteamApps*>(SteamInternal_FindOrCreateUserInterface(1, STEAMAPPS_INTERFACE_VERSION));
    CHECK(apps);
    CHECK(apps->GetDLCCount() == 88);
    CHECK(SteamApps() == apps);
    CHECK(SteamInternal_CreateInterface(STEAMAPPS_INTERFACE_VERSION) == apps);
    CHECK(SteamInternal_FindOrCreateUserInterface(1, "UnknownInterface") == nullptr);
    CHECK(SteamInternal_CreateInterface(nullptr) == nullptr);
    CHECK(apps->BIsDlcInstalled(2854750));
    CHECK(apps->BIsSubscribedApp(241360));
    CHECK(apps->BIsSubscribedApp(999));
    CHECK(!apps->BIsSubscribedApp(123456));
    CHECK(apps->GetAppBuildId() == 31415);
    CHECK(!std::strcmp(apps->GetCurrentGameLanguage(), "english"));

    AppId_t id = 0;
    bool available = false;
    char name[128] = {};
    CHECK(apps->BGetDLCDataByIndex(87, &id, &available, name, sizeof(name)));
    CHECK(id == 2854750 && available);
    CHECK(!std::strcmp(name, "Europa Universalis IV: Winds of Change"));
    CHECK(!apps->BGetDLCDataByIndex(-1, &id, &available, name, sizeof(name)));
    CHECK(!apps->BGetDLCDataByIndex(88, &id, &available, name, sizeof(name)));
    CHECK(!apps->BGetDLCDataByIndex(0, &id, &available, name, 0));
    CHECK(!apps->BGetDLCDataByIndex(0, nullptr, &available, name, sizeof(name)));
    CHECK(SteamAPI_ISteamApps_GetDLCCount(apps) == 88);
    CHECK(SteamAPI_ISteamApps_BIsDlcInstalled(apps, 2854750));
    CHECK(SteamAPI_ISteamApps_BIsSubscribedApp(apps, 241360));
    CHECK(SteamAPI_ISteamApps_BGetDLCDataByIndex(apps, 87, &id, &available, name, sizeof(name)));
    CHECK(id == 2854750);
    CHECK(!SteamAPI_ISteamApps_BGetDLCDataByIndex(nullptr, 0, &id, &available, name, sizeof(name)));

    void* handle = dlopen(argv[1], RTLD_NOW);
    CHECK(handle);
    auto count = reinterpret_cast<int (*)(ISteamApps*)>(dlsym(handle, "SteamAPI_ISteamApps_GetDLCCount"));
    auto find_apps = reinterpret_cast<void* (*)(HSteamUser, const char*)>(dlsym(handle, "SteamInternal_FindOrCreateUserInterface"));
    auto enumerate = reinterpret_cast<bool (*)(ISteamApps*, int, AppId_t*, bool*, char*, int)>(dlsym(handle, "SteamAPI_ISteamApps_BGetDLCDataByIndex"));
    CHECK(count && find_apps && enumerate);
    CHECK(count(apps) == 88);
    CHECK(find_apps(1, STEAMAPPS_INTERFACE_VERSION) == apps);
    CHECK(enumerate(apps, 87, &id, &available, name, sizeof(name)) && id == 2854750);
    CHECK(dlsym(handle, "MockInitCalls") != nullptr);
    dlclose(handle);
    std::cout << "Preload, C++ interface, flat API and dlsym tests passed.\n";
}
