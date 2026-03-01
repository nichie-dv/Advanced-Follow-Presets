#include <Geode/Geode.hpp>
#include <unordered_map>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>


enum TabColor {
    GRAY, //"GJ_button_04.png"
    COOL_GRAY, //"GJ_button_05.png"
    BLACK, //"GE_button_05.png"
    RED, //"GJ_button_06.png"
    PINK, //"GJ_button_03.png"
    LIGHT_PINK, //"GE_button_02.png"
    PURPLE, //"GE_button_03.png"
    CYAN, //"GJ_button_02.png"
    BLUE, //"GE_button_01.png"
    GREEN, //"GJ_button_01.png"
    A_SECRET_THIRD_OPTION //"GE_button_04.png"
};

            
enum AdvancedFollowMode {
    init,
    set,
    add
};

enum AdvancedFollowPage2Mode {
    m0,
    m1,
    m2
};
            
struct AdvancedFollowPreset {     
    int tabColor = 0;
    std::string name;
    std::string description;
    


    int target_group;
    int follow_group;
    int priority;
    bool x_only;
    bool y_only;
    bool follow_p1;
    bool follow_p2;
    bool follow_c;
    int rot_offset;
    int rot_easing;
    int rot_dead_z;
    bool ignore_disabled;
    bool rotate_dir;
    AdvancedFollowMode init_set_add;
    bool exclusive;
    AdvancedFollowPage2Mode page_2_mode;
    float delay;
    float delay_pm;
    float max_speed;
    float max_speed_pm;
    int max_range;
    int max_range_pm;
    int max_range_unmarked;
    float follow_easing;
    float follow_easing_pm;
    float start_speed;
    float start_speed_pm;
    int start_speed_unmarked;
    int start_dir;
    int start_dir_pm;
    int start_dir_unmarked;
    float acceleration;
    float acceleration_pm;
    float friction;
    float friction_pm;
    float near_dist;
    float near_dist_pm;
    float near_friction;
    float near_friction_pm;
    float near_accel;
    float near_accel_pm;
    float steer_force;
    float steer_force_pm;
    float steer_force_low;
    float steer_force_low_pm;
    bool steer_force_low_checkbox;
    float speed_range_low;
    float speed_range_low_pm;
    float steer_force_high;
    float steer_force_high_pm;
    bool steer_force_high_checkbox;
    float speed_range_high;
    float speed_range_high_pm;
    int slow_dist;
    int slow_dist_pm;
    float break_force;
    float break_force_pm;
    int break_angle;
    int break_angle_pm;
    float break_steer_force;
    float break_steer_force_pm;
    float break_steer_speed_limit;
    float break_steer_speed_limit_pm;
    bool target_dir;
};  
    
struct MemberMapEntry {
    const char* name;
    const int propID;
    int AdvancedFollowPreset::*intMember = nullptr;
    float AdvancedFollowPreset::*floatMember = nullptr;
    bool AdvancedFollowPreset::*boolMember = nullptr;
    AdvancedFollowMode AdvancedFollowPreset::*enumAFMMember = nullptr;
    AdvancedFollowPage2Mode AdvancedFollowPreset::*enumAFP2Member = nullptr;
};

inline constexpr MemberMapEntry memberMappings[] = {
            
    {"target_group", 51, &AdvancedFollowPreset::target_group},
    {"follow_group", 71, &AdvancedFollowPreset::follow_group},
    {"priority", 365, &AdvancedFollowPreset::priority},
    {"x_only", 306, nullptr, nullptr, &AdvancedFollowPreset::x_only},
    {"y_only", 307, nullptr, nullptr, &AdvancedFollowPreset::y_only},
    {"follow_p1", 138, nullptr, nullptr, &AdvancedFollowPreset::follow_p1},
    {"follow_p2", 200, nullptr, nullptr, &AdvancedFollowPreset::follow_p2},
    {"follow_c", 201, nullptr, nullptr, &AdvancedFollowPreset::follow_c},
    {"rot_offset", 340, &AdvancedFollowPreset::rot_offset},
    {"rot_easing", 363, &AdvancedFollowPreset::rot_easing},
    {"rot_dead_z", 364, &AdvancedFollowPreset::rot_dead_z},
    {"ignore_disabled", 336, nullptr, nullptr, &AdvancedFollowPreset::ignore_disabled},
    {"rotate_dir", 339, nullptr, nullptr, &AdvancedFollowPreset::rotate_dir},
    {"init_set_add", 572, nullptr, nullptr, nullptr, &AdvancedFollowPreset::init_set_add},
    {"exclusive", 571, nullptr, nullptr, &AdvancedFollowPreset::exclusive},
    {"page_2_mode", 367, nullptr, nullptr, nullptr, nullptr, &AdvancedFollowPreset::page_2_mode},
    {"delay", 292, nullptr, &AdvancedFollowPreset::delay},
    {"delay_pm", 293, nullptr, &AdvancedFollowPreset::delay_pm},
    {"max_speed", 298, nullptr, &AdvancedFollowPreset::max_speed},
    {"max_speed_pm", 299, nullptr, &AdvancedFollowPreset::max_speed_pm},
    {"max_range", 308, &AdvancedFollowPreset::max_range},
    {"max_range_pm", 309, &AdvancedFollowPreset::max_range_pm},
    {"max_range_unmarked", 366, &AdvancedFollowPreset::max_range_unmarked},
    {"follow_easing", 361, nullptr, &AdvancedFollowPreset::follow_easing},
    {"follow_easing_pm", 362, nullptr, &AdvancedFollowPreset::follow_easing_pm},
    {"start_speed", 300, nullptr, &AdvancedFollowPreset::start_speed},
    {"start_speed_pm", 301, nullptr, &AdvancedFollowPreset::start_speed_pm},
    {"start_speed_unmarked", 560, &AdvancedFollowPreset::start_speed_unmarked},
    {"start_dir", 563, &AdvancedFollowPreset::start_dir},
    {"start_dir_pm", 564, &AdvancedFollowPreset::start_dir_pm},
    {"start_dir_unmarked", 565, &AdvancedFollowPreset::start_dir_unmarked},
    {"acceleration", 334, nullptr, &AdvancedFollowPreset::acceleration},
    {"acceleration_pm", 335, nullptr, &AdvancedFollowPreset::acceleration_pm},
    {"friction", 558, nullptr, &AdvancedFollowPreset::friction},
    {"friction_pm", 559, nullptr, &AdvancedFollowPreset::friction_pm},
    {"near_dist", 359, nullptr, &AdvancedFollowPreset::near_dist},
    {"near_dist_pm", 360, nullptr, &AdvancedFollowPreset::near_dist_pm},
    {"near_friction", 561, nullptr, &AdvancedFollowPreset::near_friction},
    {"near_friction_pm", 562, nullptr, &AdvancedFollowPreset::near_friction_pm},
    {"near_accel", 357, nullptr, &AdvancedFollowPreset::near_accel},
    {"near_accel_pm", 358, nullptr, &AdvancedFollowPreset::near_accel_pm},
    {"steer_force", 316, nullptr, &AdvancedFollowPreset::steer_force},
    {"steer_force_pm", 317, nullptr, &AdvancedFollowPreset::steer_force_pm},
    {"steer_force_low", 318, nullptr, &AdvancedFollowPreset::steer_force_low},
    {"steer_force_low_pm", 319, nullptr, &AdvancedFollowPreset::steer_force_low_pm},
    {"steer_force_low_checkbox", 337, nullptr, nullptr, &AdvancedFollowPreset::steer_force_low_checkbox},
    {"speed_range_low", 322, nullptr, &AdvancedFollowPreset::speed_range_low},
    {"speed_range_low_pm", 323, nullptr, &AdvancedFollowPreset::speed_range_low_pm},
    {"steer_force_high", 320, nullptr, &AdvancedFollowPreset::steer_force_high},
    {"steer_force_high_pm", 321, nullptr, &AdvancedFollowPreset::steer_force_high_pm},
    {"steer_force_high_checkbox", 338, nullptr, nullptr, &AdvancedFollowPreset::steer_force_high_checkbox},
    {"speed_range_high", 324, nullptr, &AdvancedFollowPreset::speed_range_high},
    {"speed_range_high_pm", 325, nullptr, &AdvancedFollowPreset::speed_range_high_pm},
    {"slow_dist", 359, &AdvancedFollowPreset::slow_dist},
    {"slow_dist_pm", 360, &AdvancedFollowPreset::slow_dist_pm},
    {"break_force", 326, nullptr, &AdvancedFollowPreset::break_force},
    {"break_force_pm", 327, nullptr, &AdvancedFollowPreset::break_force_pm},
    {"break_angle", 328, &AdvancedFollowPreset::break_angle},
    {"break_angle_pm", 329, &AdvancedFollowPreset::break_angle_pm},
    {"break_steer_force", 330, nullptr, &AdvancedFollowPreset::break_steer_force},
    {"break_steer_force_pm", 331, nullptr, &AdvancedFollowPreset::break_steer_force_pm},
    {"break_steer_speed_limit", 332, nullptr, &AdvancedFollowPreset::break_steer_speed_limit},
    {"break_steer_speed_limit_pm", 333, nullptr, &AdvancedFollowPreset::break_steer_speed_limit_pm},
    {"target_dir", 305, nullptr, nullptr, &AdvancedFollowPreset::target_dir},
};
            
matjson::Value presetToJson(const AdvancedFollowPreset& preset) {
    matjson::Value obj;
    obj["highlightColor"] = preset.tabColor;
    obj["name"] = preset.name;
    obj["description"] = preset.description;

    for (auto& map : memberMappings) {
        if (map.intMember) 
            obj[map.name] = preset.*(map.intMember);
        else if (map.floatMember) 
            obj[map.name] = preset.*(map.floatMember);
        else if (map.boolMember) 
            obj[map.name] = preset.*(map.boolMember);
        else if (map.enumAFMMember) 
            obj[map.name] = static_cast<int>(preset.*(map.enumAFMMember));
        else if (map.enumAFP2Member) 
            obj[map.name] = static_cast<int>(preset.*(map.enumAFP2Member));
    }

    return obj;
};

AdvancedFollowPreset presetFromJson(const matjson::Value& obj) {
    AdvancedFollowPreset preset{}; 
    preset.tabColor = static_cast<int>(obj["highlightColor"].asInt().unwrap());
    preset.name = static_cast<std::string>(obj["name"].asString().unwrap());
    preset.description = static_cast<std::string>(obj["description"].asString().unwrap());

    for (auto& map : memberMappings) {
        if (obj[map.name].isNull()) continue;

        if (map.intMember) {
            auto res = obj[map.name].asInt();
            if (res) preset.*(map.intMember) = static_cast<int>(*res);
        } 
        else if (map.floatMember) {
            auto res = obj[map.name].asDouble();
            if (res) preset.*(map.floatMember) = static_cast<float>(*res);
        } 
        else if (map.boolMember) {
            auto res = obj[map.name].asBool();
            if (res) preset.*(map.boolMember) = *res;
        } 
        else if (map.enumAFMMember) {
            auto res = obj[map.name].asInt();
            if (res) preset.*(map.enumAFMMember) = static_cast<AdvancedFollowMode>(*res);
        } 
        else if (map.enumAFP2Member) {
            auto res = obj[map.name].asInt();
            if (res) preset.*(map.enumAFP2Member) = static_cast<AdvancedFollowPage2Mode>(*res);
        }
    }

    return preset;
};



inline std::string createTemplates() {
    /* Returns path of saved template presets */


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
    
    
    matjson::Value jsonMissile = presetToJson(robtopHomingMissile);
    {
        auto path = geode::Mod::get()->getSettingValue<std::filesystem::path>("template-path") / "robtopHomingMissile.json";
        std::ofstream file(path); 
        file << jsonMissile.dump(4);      
        file.close();
    }
    
    matjson::Value jsonBall = presetToJson(robtopHomingBall);
    {
        auto path = geode::Mod::get()->getSettingValue<std::filesystem::path>("template-path") / "robtopHomingBall.json";
        std::ofstream file(path);
        file << jsonBall.dump(4);
        file.close();
    }
    
    matjson::Value jsonTemp = presetToJson(templatePreset);
    {
        auto path = geode::Mod::get()->getSettingValue<std::filesystem::path>("template-path") / "templatePreset.json";
        std::ofstream file(path);
        file << jsonTemp.dump(4);
        file.close();
    }
    
    return geode::Mod::get()->getSaveDir().string() + "/templates";
};


struct LoadedPreset {
    AdvancedFollowPreset preset;
    int tag;
    std::filesystem::path filePath;
};

struct PresetsOptions {
    std::vector<LoadedPreset>* loadedPresets; 
    std::vector<LoadedPreset>* disabledPresets;
    SetupAdvFollowPopup* trigger;
};


class createPresetPopup : public cocos2d::CCLayer {

    public:
    static createPresetPopup* create();
    cocos2d::CCMenu* PopupMenuMain;
    std::vector<CCMenuItemSpriteExtra*> iconButtons;


    protected:
    AdvancedFollowPreset* presetBase;
    bool init() override;
    void keyBackClicked() override;
    void onBack(CCObject*);
   
   
    void registerWithTouchDispatcher() override;
    ~createPresetPopup();

};
