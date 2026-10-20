#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include "ext/ini.h"

namespace eu4dlc {
using DlcList = std::vector<std::tuple<std::uint32_t, std::string>>;

struct Config {
    mINI::INIStructure ini;
    DlcList dlcs;
};

inline Config read_config(const std::string& path) {
    Config result;
    mINI::INIFile file(path);
    if (!file.read(result.ini)) {
        throw std::runtime_error("Cannot read configuration: " + path);
    }
    for (const auto& entry : result.ini["dlc"]) {
        const auto& key = entry.first;
        if (key.empty() || !std::all_of(key.begin(), key.end(), [](char c) {
                return c >= '0' && c <= '9';
            })) {
            throw std::runtime_error("Invalid DLC AppID: " + key);
        }
        const auto id = std::stoull(key);
        if (id == 0 || id > std::numeric_limits<std::uint32_t>::max() || entry.second.empty()) {
            throw std::runtime_error("Invalid DLC entry: " + key);
        }
        result.dlcs.emplace_back(static_cast<std::uint32_t>(id), entry.second);
    }
    if (result.dlcs.empty()) {
        throw std::runtime_error("Configuration contains no DLC entries: " + path);
    }
    return result;
}

inline bool copy_dlc_data(const DlcList& dlcs, int index, std::uint32_t* app_id,
                          bool* available, char* name, int size) {
    if (index < 0 || static_cast<std::size_t>(index) >= dlcs.size() ||
        !app_id || !available || !name || size <= 0) {
        return false;
    }
    const auto& dlc = dlcs[static_cast<std::size_t>(index)];
    const auto& title = std::get<1>(dlc);
    const auto length = std::min(static_cast<std::size_t>(size - 1), title.size());
    *app_id = std::get<0>(dlc);
    *available = true;
    std::memcpy(name, title.data(), length);
    name[length] = '\0';
    return true;
}
} // namespace eu4dlc
