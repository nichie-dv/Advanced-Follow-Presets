#pragma once
#include <Geode/Geode.hpp>
#include <string>



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


//Object format for object property table
struct MemberMapEntry {
    const char* name;
    const int propID;
    int AdvancedFollowPreset::*intMember = nullptr;
    float AdvancedFollowPreset::*floatMember = nullptr;
    bool AdvancedFollowPreset::*boolMember = nullptr;
    AdvancedFollowMode AdvancedFollowPreset::*enumAFMMember = nullptr;
    AdvancedFollowPage2Mode AdvancedFollowPreset::*enumAFP2Member = nullptr;
};

//Map of object properties and data types
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

//Get tab icon for an index (10 >= i >= 0)
inline const char* TabIconForIndex(int index) {
    switch (index) {
        case TabColor::GRAY: return "GJ_button_04.png";
        case TabColor::COOL_GRAY: return "GJ_button_05.png";
        case TabColor::BLACK: return "geode.loader/GE_button_05.png";
        case TabColor::RED: return "GJ_button_06.png";
        case TabColor::PINK: return "GJ_button_03.png";
        case TabColor::LIGHT_PINK: return "geode.loader/GE_button_02.png";
        case TabColor::PURPLE: return "geode.loader/GE_button_03.png";
        case TabColor::CYAN: return "GJ_button_02.png";
        case TabColor::BLUE: return "geode.loader/GE_button_01.png";
        case TabColor::GREEN: return "GJ_button_01.png";
        case TabColor::A_SECRET_THIRD_OPTION: return "geode.loader/GE_button_04.png";
        default: return "GJ_button_04.png";
    }
};


matjson::Value PresetToJson(const AdvancedFollowPreset& preset);
AdvancedFollowPreset PresetFromJson(const matjson::Value& json);

matjson::Value FileToJson(const std::filesystem::path& filepath);
matjson::Value FileToJson(const std::filesystem::path& filepath, bool logErrors);

void JsonToFile(const matjson::Value& json, const std::filesystem::path& filepath);

void FileToTrash(const std::filesystem::path& filepath);

bool DoesFileOrPathExist(const std::filesystem::path& filepath);
void CreatePath(const std::filesystem::path& filepath);

struct KVPair {
    int id;
    float value;
};

std::pair<std::vector<KVPair>, int> UnwrapPreset(AdvancedFollowPreset& preset);
AdvancedFollowPreset WrapPreset(SetupAdvFollowPopup* trigger);

//Vector type and item type must match
//Searches through vector to find item and returns true
//otherwise returns false
template <typename T>
bool IsInVector(const std::vector<T>& vector, const T& item) {
    return std::find(vector.begin(), vector.end(), item) != vector.end();
}



//Searches through a <int, bundle> map to find item and returns true
//otherwise returns false
template <typename MapType>
bool IsInPresetMap(const MapType& map, const std::string& name)
{
    for (typename MapType::const_iterator it = map.begin(); it != map.end(); ++it)
    {
        if (it->second && it->second->preset.name == name)
            return true;
    }
    return false;
}

