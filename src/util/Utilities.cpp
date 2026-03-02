#include "Utilities.hpp"
#include <Geode/Geode.hpp>
#include <string>
#include <filesystem>

using namespace geode::prelude;
using namespace std; // :D



//Parses given preset into json and returns it
matjson::Value PresetToJson(const AdvancedFollowPreset& preset) {
    matjson::Value json;
    json["highlightColor"] = preset.tabColor;
    json["name"] = preset.name;
    json["description"] = preset.description;

    for (auto& map : memberMappings) {
        if (map.intMember) 
            json[map.name] = preset.*(map.intMember);
        else if (map.floatMember) 
            json[map.name] = preset.*(map.floatMember);
        else if (map.boolMember) 
            json[map.name] = preset.*(map.boolMember);
        else if (map.enumAFMMember) 
            json[map.name] = static_cast<int>(preset.*(map.enumAFMMember));
        else if (map.enumAFP2Member) 
            json[map.name] = static_cast<int>(preset.*(map.enumAFP2Member));
    }

    return json;
}

//Parses given json into an advanced follow preset and returns it
AdvancedFollowPreset PresetFromJson(const matjson::Value& json) {
    AdvancedFollowPreset preset{}; 
    preset.tabColor = static_cast<int>(json["highlightColor"].asInt().unwrap());
    preset.name = static_cast<std::string>(json["name"].asString().unwrap());
    preset.description = static_cast<std::string>(json["description"].asString().unwrap());

    for (auto& map : memberMappings) {
        if (json[map.name].isNull()) continue;

        if (map.intMember) {
            auto res = json[map.name].asInt();
            if (res) preset.*(map.intMember) = static_cast<int>(*res);
        } 
        else if (map.floatMember) {
            auto res = json[map.name].asDouble();
            if (res) preset.*(map.floatMember) = static_cast<float>(*res);
        } 
        else if (map.boolMember) {
            auto res = json[map.name].asBool();
            if (res) preset.*(map.boolMember) = *res;
        } 
        else if (map.enumAFMMember) {
            auto res = json[map.name].asInt();
            if (res) preset.*(map.enumAFMMember) = static_cast<AdvancedFollowMode>(*res);
        } 
        else if (map.enumAFP2Member) {
            auto res = json[map.name].asInt();
            if (res) preset.*(map.enumAFP2Member) = static_cast<AdvancedFollowPage2Mode>(*res);
        }
    }

    return preset;
}

//Parses json from path and returns it
matjson::Value FileToJson(const filesystem::path& filepath) {
    try {
        ifstream file(filepath);
        if (!file.is_open()) {
            log::warn("Failed to open file at {}", filepath.string());
            return matjson::Value();
        }

        std::string jsonStr((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

        auto parsed = matjson::parse(jsonStr);
        if (!parsed) {
            log::warn("Failed to parse json at {}: {}", filepath.string(), parsed.unwrapErr());
            return matjson::Value();
        }

        return parsed.unwrap();
    }
    catch (const exception& e) {
        log::warn("Exception parsing json at {}: {}", filepath.string(), e.what());
        return matjson::Value();
    }
}

//Writes given json to given filepath
void JsonToFile(const matjson::Value& json, const filesystem::path& filepath) {
    try {
        ofstream file(filepath);
        if (!file.is_open()) {
            log::warn("Failed to open file at {}", filepath.string());
            return;
        }
        file << json.dump(4);
        file.close();
    } catch (const exception& e) {
        log::warn("Failed to write json to path {}: {}", filepath, e.what());
    }
}

//Deletes json file with given filepath (maybe dangerous)
void FileToTrash(const filesystem::path& filepath) {
    try {
        filesystem::remove(filepath);
    } catch (const exception& e) {
        log::warn("Failed to write delete file at path {}: {}", filepath.string(), e.what());
    }
}

//I'm not really sure why i made this but it looks nicer (probably not)
bool DoesFileOrPathExist(const filesystem::path& filepath) {
    return (filesystem::exists(filepath));
}

//Wrapper for create path (cleaner)
void CreatePath(const filesystem::path& filepath) {
    filesystem::create_directories(filepath);
}


//Unwrap presets into a vector of keys and values (also vectors)
pair<vector<int>, vector<float>> UnwrapPreset(AdvancedFollowPreset& preset) {
    vector<int> presetIDs;
    vector<float> presetValues;

    for (const auto& entry : memberMappings) {

        presetIDs.push_back(entry.propID);

        if (entry.intMember) presetValues.push_back(static_cast<float>(preset.*(entry.intMember)));
        else if (entry.floatMember) presetValues.push_back(preset.*(entry.floatMember));
        else if (entry.boolMember) presetValues.push_back(preset.*(entry.boolMember) ? 1.0f : 0.0f);
        else if (entry.enumAFMMember) presetValues.push_back(static_cast<float>(static_cast<int>(preset.*(entry.enumAFMMember))));
        else if (entry.enumAFP2Member) presetValues.push_back(static_cast<float>(static_cast<int>(preset.*(entry.enumAFP2Member))));
        else log::warn("unknown property {}", entry.propID);
    }

    return {presetIDs, presetValues};

}

//Wraps into a preset from a trigger
AdvancedFollowPreset WrapPreset(SetupAdvFollowPopup* trigger) {
    AdvancedFollowPreset preset = {};

    for (const auto& entry : memberMappings) {

        float value = trigger->getValue(entry.propID);
        if (!value) continue;

        if (entry.intMember) preset.*(entry.intMember) = static_cast<int>(value);
        else if (entry.floatMember) preset.*(entry.floatMember) = value;
        else if (entry.boolMember) preset.*(entry.boolMember) = (value != 0.f);
        else if (entry.enumAFMMember) preset.*(entry.enumAFMMember) = static_cast<AdvancedFollowMode>(static_cast<int>(value));
        else if (entry.enumAFP2Member) preset.*(entry.enumAFP2Member) = static_cast<AdvancedFollowPage2Mode>(static_cast<int>(value));
        else log::warn("unknown property {}", entry.propID);
    }

    return preset;
}
