#include "src/config.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>

#define CHECK(value) do { if (!(value)) { std::cerr << "Check failed at line " << __LINE__ << ": " #value "\n"; return 1; } } while (false)

static void write_file(const std::string& text) {
    std::ofstream file("config-test.ini", std::ios::binary);
    file << text;
}

static bool rejects(const std::string& text) {
    write_file(text);
    try { eu4dlc::read_config("config-test.ini"); }
    catch (const std::exception&) { return true; }
    return false;
}

int main(int argc, char** argv) {
    CHECK(argc == 2);
    auto profile = eu4dlc::read_config(argv[1]);
    CHECK(profile.dlcs.size() == 88);
    CHECK(std::get<0>(profile.dlcs.front()) == 241360);
    CHECK(std::get<0>(profile.dlcs.back()) == 2854750);
    CHECK(std::get<1>(profile.dlcs.back()) == "Europa Universalis IV: Winds of Change");

    write_file("[steam]\r\nappid = 236850\r\n[dlc]\r\n241360 = First\r\n2854750 = Last\r\n");
    auto config = eu4dlc::read_config("config-test.ini");
    CHECK(config.dlcs.size() == 2);
    CHECK(std::get<1>(config.dlcs.back()) == "Last");
    std::uint32_t id = 0;
    bool available = false;
    char name[8] = {};
    CHECK(eu4dlc::copy_dlc_data(config.dlcs, 0, &id, &available, name, 8));
    CHECK(id == 241360 && available && std::string(name) == "First");
    CHECK(eu4dlc::copy_dlc_data(config.dlcs, 0, &id, &available, name, 1));
    CHECK(name[0] == '\0');
    CHECK(eu4dlc::copy_dlc_data(config.dlcs, 0, &id, &available, name, 3));
    CHECK(std::string(name) == "Fi");
    CHECK(!eu4dlc::copy_dlc_data(config.dlcs, -1, &id, &available, name, 8));
    CHECK(!eu4dlc::copy_dlc_data(config.dlcs, 2, &id, &available, name, 8));
    CHECK(!eu4dlc::copy_dlc_data(config.dlcs, 0, &id, &available, name, 0));
    CHECK(!eu4dlc::copy_dlc_data(config.dlcs, 0, &id, &available, name, -1));
    CHECK(!eu4dlc::copy_dlc_data(config.dlcs, 0, nullptr, &available, name, 8));
    CHECK(!eu4dlc::copy_dlc_data(config.dlcs, 0, &id, nullptr, name, 8));
    CHECK(!eu4dlc::copy_dlc_data(config.dlcs, 0, &id, &available, nullptr, 8));
    CHECK(rejects("[dlc]\n"));
    CHECK(rejects("[dlc]\n123garbage = Invalid\n"));
    CHECK(rejects("[dlc]\n-1 = Invalid\n"));
    CHECK(rejects("[dlc]\n4294967296 = Invalid\n"));
    CHECK(rejects("[dlc]\n999999999999999999999999 = Invalid\n"));
    CHECK(rejects("[dlc]\n0 = Invalid\n"));
    CHECK(rejects("[dlc]\n1 = \n"));
    std::remove("config-test.ini");
    try { eu4dlc::read_config("config-test.ini"); CHECK(false); }
    catch (const std::exception&) { }
    std::cout << "Configuration and DLC buffer tests passed.\n";
}
