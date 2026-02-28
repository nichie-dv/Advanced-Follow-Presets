import json




with open("advFollowTrigger.json", "r") as f:
    data = json.load(f)



with open("header.hpp", "w") as f:
    f.write("""#include <Geode/Geode.hpp>
#include <unordered_map>
#include <string>

struct MemberMapEntry {
    const char* name;
    const int propID;
    int AdvancedFollowpreset::*intMember = nullptr;
    float AdvancedFollowpreset::*floatMember = nullptr;
    bool AdvancedFollowpreset::*boolMember = nullptr;
    AdvancedFollowMode AdvancedFollowpreset::*enumAFMMember = nullptr;
    AdvancedFollowPage2Mode AdvancedFollowpreset::*enumAFP2Member = nullptr;
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
            
struct AdvancedFollowpreset {       
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
            
inline constexpr MemberMapEntry memberMappings[] = {
            """)
    for item in data['AdvancedFollowTrigger']['fields']:
        itemType = 0
        if (item['type'] in ['GroupID', 'Int']):
            itemType = 0
        elif (item['type'] == 'Float'):
            itemType = 1
        elif (item['type'] == 'AdvancedFollowMode'):
            itemType = 2
        elif (item['type'] == 'AdvancedFollowPage2Mode'):
            itemType = 3
        else:
            itemType = 4


        match (itemType):
            case 0:
                f.write(f"""
    {{\"{item['name']}\", {item['id']}, &AdvancedFollowpreset::{item['name']}}},""")
                
            case 1:
                f.write(f"""
    {{\"{item['name']}\", {item['id']}, nullptr, &AdvancedFollowpreset::{item['name']}}},""")
            
            case 2:
                f.write(f"""
    {{\"{item['name']}\", {item['id']}, nullptr, nullptr, nullptr, &AdvancedFollowpreset::{item['name']}}},""")
                
            case 3:
                f.write(f"""
    {{\"{item['name']}\", {item['id']}, nullptr, nullptr, nullptr, nullptr, &AdvancedFollowpreset::{item['name']}}},""")

            case 4:
                f.write(f"""
    {{\"{item['name']}\", {item['id']}, nullptr, nullptr, &AdvancedFollowpreset::{item['name']}}},""")
                
    
    f.write("""
};
            
matjson::Value presetToJson(const AdvancedFollowpreset& preset) {
    matjson::Value obj;

    for (auto& map : presetMapping) {
        if (map.intMember) obj[map.name] = preset.*(map.intMember);
        else if (map.floatMember) obj[map.name] = preset.*(map.floatMember);
        else if (map.boolMember) obj[map.name] = preset.*(map.boolMember);
        else if (map.enumAFMMember) obj[map.name] = static_cast<int>(preset.*(map.enumAFMMember));
        else if (map.enumAFP2Member) obj[map.name] = static_cast<int>(preset.*(map.enumAFP2Member));
    }

    return obj;
};
            
AdvancedFollowpreset presetFromJson(const matjson::Value& obj) {
    AdvancedFollowpreset preset{};

    for (auto& map : presetMapping) {
        if (!obj.hasKey(map.name)) continue;

        if (map.intMember) preset.*(map.intMember) = obj[map.name].asInt();
        else if (map.floatMember) preset.*(map.floatMember) = obj[map.name].asFloat();
        else if (map.boolMember) preset.*(map.boolMember) = obj[map.name].asBool();
        else if (map.enumAFMMember) preset.*(map.enumAFMMember) = static_cast<AdvancedFollowMode>(obj[map.name].asInt());
        else if (map.enumAFP2Member) preset.*(map.enumAFP2Member) = static_cast<AdvancedFollowPage2Mode>(obj[map.name].asInt());
    }

    return preset;
};""")
        
            
    #{"target_group", &AdvancedFollowpreset::target_group, nullptr, nullptr},
    #{"priority", &AdvancedFollowpreset::priority, nullptr, nullptr},
       
