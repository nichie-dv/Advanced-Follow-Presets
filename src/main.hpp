#pragma once
#include <Geode/Geode.hpp>
#include <unordered_map>
#include <string>

#include "util/Utilities.hpp"

using namespace geode::prelude;

inline ccColor3B EnabledColor = { 255, 255, 255 };
inline ccColor3B DisabledColor = { 129, 129, 129 };

std::string operator""_custom_ext(const char* str, size_t len)
{
    return std::string(str, len) + "." + Mod::get()->getSettingValue<std::string>("file-extension");
}

struct PresetItemBundle {
    AdvancedFollowPreset preset;
    CCMenuItemSpriteExtra* button;
    bool enabled = false;
    bool inBin = false;
    std::filesystem::path PresetPath;
};

struct MainContainer {
    SetupAdvFollowPopup* Trigger;
    std::unordered_map<int, PresetItemBundle*> PresetMap;
    std::filesystem::path PresetsPath = Mod::get()->getSettingValue<std::filesystem::path>("presets-path");
    std::filesystem::path TemplatesPath = Mod::get()->getSettingValue<std::filesystem::path>("template-path");
    std::filesystem::path DisabledPath = Mod::get()->getSaveDir() / "disabled"_custom_ext;
    std::string CustomExt = "." + Mod::get()->getSettingValue<std::string>("file-extension");
};

