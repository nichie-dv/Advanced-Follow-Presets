#pragma once
#include <Geode/Geode.hpp>
#include <string>

#include "Utilities.hpp"
#include "../main.hpp"

using namespace geode::prelude;
using namespace std;

//Save robtop presets + template
inline void CreateTemplates() {

    //hardcoded to prevent accidental deletion

    AdvancedFollowPreset robtopHomingMissile = {};
    robtopHomingMissile.name = "Homing Missile";
    robtopHomingMissile.tabColor = TabColor::GRAY;
    robtopHomingMissile.rotate_dir = 1.0f;
    robtopHomingMissile.delay = 0.0f;
    robtopHomingMissile.delay_pm = 0.0f;
    robtopHomingMissile.max_speed = 5.0f;
    robtopHomingMissile.max_speed_pm = 0.0f;
    robtopHomingMissile.acceleration = 0.5f;
    robtopHomingMissile.acceleration_pm = 0.0f;
    robtopHomingMissile.steer_force = 1.4f;
    robtopHomingMissile.steer_force_pm = 0.0f;
    robtopHomingMissile.steer_force_low_checkbox = true;
    robtopHomingMissile.steer_force_low = 5.0f;
    robtopHomingMissile.steer_force_low_pm = 0.0f;
    robtopHomingMissile.speed_range_low = 0.0f;
    robtopHomingMissile.page_2_mode = AdvancedFollowPage2Mode::m2;

    AdvancedFollowPreset robtopHomingBall = {};
    robtopHomingBall.name = "Homing Ball";
    robtopHomingBall.tabColor = TabColor::GRAY;
    robtopHomingBall.delay = 0.0f;
    robtopHomingBall.delay_pm = 0.0f;
    robtopHomingBall.max_speed = 0.6f;
    robtopHomingBall.max_speed_pm = 0.0f;
    robtopHomingBall.acceleration = 0.2f;
    robtopHomingBall.acceleration_pm = 0.0f;
    robtopHomingBall.steer_force = 5.0f;
    robtopHomingBall.steer_force_pm = 0.0f;
    robtopHomingBall.steer_force_low_checkbox = false;
    robtopHomingBall.steer_force_low = 0.0f;
    robtopHomingBall.steer_force_low_pm = 0.0f;
    robtopHomingBall.speed_range_low = 0.0f;
    robtopHomingBall.speed_range_low_pm = 0.0f;
    robtopHomingBall.page_2_mode = AdvancedFollowPage2Mode::m2;
    
    AdvancedFollowPreset templatePreset = {};
    templatePreset.name = "Template";
    
    JsonToFile(PresetToJson(robtopHomingMissile), Mod::get()->getSettingValue<filesystem::path>("template-path") / "Homing Missile"_custom_ext);
    JsonToFile(PresetToJson(robtopHomingBall), Mod::get()->getSettingValue<filesystem::path>("template-path") / "Homing Ball"_custom_ext);
    JsonToFile(PresetToJson(templatePreset), Mod::get()->getSettingValue<filesystem::path>("template-path") / "Template"_custom_ext);

};