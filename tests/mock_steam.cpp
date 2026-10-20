#include "ext/steam/isteamapps.h"
#include <cstring>

class MockApps final : public ISteamApps {
public:
    bool BIsSubscribed() override { return true; }
    bool BIsLowViolence() override { return false; }
    bool BIsCybercafe() override { return false; }
    bool BIsVACBanned() override { return false; }
    const char* GetCurrentGameLanguage() override { return "english"; }
    const char* GetAvailableGameLanguages() override { return "english,german"; }
    bool BIsSubscribedApp(AppId_t id) override { return id == 999; }
    bool BIsDlcInstalled(AppId_t) override { return false; }
    uint32 GetEarliestPurchaseUnixTime(AppId_t) override { return 123; }
    bool BIsSubscribedFromFreeWeekend() override { return false; }
    int GetDLCCount() override { return 0; }
    bool BGetDLCDataByIndex(int, AppId_t*, bool*, char*, int) override { return false; }
    void InstallDLC(AppId_t) override { }
    void UninstallDLC(AppId_t) override { }
    void RequestAppProofOfPurchaseKey(AppId_t) override { }
    bool GetCurrentBetaName(char*, int) override { return false; }
    bool MarkContentCorrupt(bool) override { return false; }
    uint32 GetInstalledDepots(AppId_t, DepotId_t*, uint32) override { return 0; }
    uint32 GetAppInstallDir(AppId_t, char*, uint32) override { return 0; }
    bool BIsAppInstalled(AppId_t id) override { return id == 236850; }
    CSteamID GetAppOwner() override { return CSteamID(uint64(76561198000000000)); }
    const char* GetLaunchQueryParam(const char*) override { return "mock-query"; }
    bool GetDlcDownloadProgress(AppId_t, uint64*, uint64*) override { return false; }
    int GetAppBuildId() override { return 31415; }
    void RequestAllProofOfPurchaseKeys() override { }
    SteamAPICall_t GetFileDetails(const char*) override { return 0; }
    int GetLaunchCommandLine(char*, int) override { return 0; }
    bool BIsSubscribedFromFamilySharing() override { return false; }
    bool BIsTimedTrial(uint32*, uint32*) override { return false; }
};

static MockApps apps;
static int init_calls = 0;
extern "C" bool SteamAPI_Init() { ++init_calls; return true; }
extern "C" int MockInitCalls() { return init_calls; }
extern "C" ISteamApps* SteamApps() { return &apps; }
extern "C" void* SteamInternal_FindOrCreateUserInterface(HSteamUser, const char* version) {
    return version && !std::strcmp(version, STEAMAPPS_INTERFACE_VERSION) ? &apps : nullptr;
}
extern "C" void* SteamInternal_CreateInterface(const char* version) {
    return SteamInternal_FindOrCreateUserInterface(0, version);
}
extern "C" int SteamAPI_ISteamApps_GetDLCCount(ISteamApps* self) { return self->GetDLCCount(); }
extern "C" bool SteamAPI_ISteamApps_BIsDlcInstalled(ISteamApps* self, AppId_t id) { return self->BIsDlcInstalled(id); }
extern "C" bool SteamAPI_ISteamApps_BIsSubscribedApp(ISteamApps* self, AppId_t id) { return self->BIsSubscribedApp(id); }
extern "C" bool SteamAPI_ISteamApps_BGetDLCDataByIndex(ISteamApps* self, int index, AppId_t* id, bool* available, char* name, int size) {
    return self->BGetDLCDataByIndex(index, id, available, name, size);
}
