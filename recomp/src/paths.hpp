#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

namespace N64 {

class PathManager {
public:
    static std::string getAppDataPath() {
        std::string base = "";
#ifdef _WIN32
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
            base = std::string(path) + "\\ConkerRecompiled";
        } else {
            base = ".";
        }
#else
        const char* home = getenv("HOME");
        base = home ? std::string(home) + "/.conker_recompiled" : ".";
#endif
        std::filesystem::create_directories(base + "/Saves");
        return base;
    }

    static std::string getCachePath() {
        std::string base = "";
#ifdef _WIN32
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
            base = std::string(path) + "\\ConkerRecompiled\\Cache";
        } else {
            base = "./Cache";
        }
#else
        const char* home = getenv("HOME");
        base = home ? std::string(home) + "/.cache/conker_recompiled" : "./Cache";
#endif
        std::filesystem::create_directories(base);
        return base;
    }

    static std::string getSaveFilePath() {
        return getAppDataPath() + "\\Saves\\conker.eep";
    }

    static std::string getSaveStatePath(int slot = 0) {
        return getAppDataPath() + "\\Saves\\state_" + std::to_string(slot) + ".savestate";
    }
};

} // namespace N64
