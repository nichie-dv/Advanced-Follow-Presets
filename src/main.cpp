#include <Geode/Geode.hpp>
#include <Geode/utils/string.hpp>
#include <vector>
#include <iostream>

#include "main.hpp"
#include "ui/PresetPopup.hpp"
#include "util/Utilities.hpp"
#include "util/Templates.hpp"

using namespace geode::prelude;

MainContainer* OptionsContainer = nullptr;

//Selection modes for menu items
enum SelectionMode  {
    SELECT_NORMAL, //select as normal button
    SELECT_DELETE, //select for deletion
    SELECT_GRAB //select for toggling
};


$on_mod(Loaded) {
    //Paths
    if (!DoesFileOrPathExist(Mod::get()->getSettingValue<std::filesystem::path>("template-path"))) {
        CreatePath(Mod::get()->getSettingValue<std::filesystem::path>("template-path"));
    }
    if (!DoesFileOrPathExist(Mod::get()->getSettingValue<std::filesystem::path>("presets-path"))) {
        CreatePath(Mod::get()->getSettingValue<std::filesystem::path>("presets-path"));
    }

    if (!DoesFileOrPathExist(Mod::get()->getSettingValue<std::filesystem::path>("template-path") / "Homing Missile"_custom_ext) ||
        !DoesFileOrPathExist(Mod::get()->getSettingValue<std::filesystem::path>("template-path") / "Homing Ball"_custom_ext) ||
        !DoesFileOrPathExist(Mod::get()->getSettingValue<std::filesystem::path>("template-path") / "Template"_custom_ext)) {
        
        log::debug("creating new templates at {}", utils::string::pathToString(Mod::get()->getSettingValue<std::filesystem::path>("template-path")));
        CreateTemplates();
    }


    //disabled list
    if (!DoesFileOrPathExist(Mod::get()->getSaveDir() / "disabled"_custom_ext)) {
        matjson::Value disabledJson;
        disabledJson["disabled"] = std::vector<std::string>{};
        auto path = Mod::get()->getSaveDir() / "disabled"_custom_ext;
        JsonToFile(disabledJson, path);
        log::debug("created disabled list at {}", utils::string::pathToString(Mod::get()->getSaveDir() / "disabled"_custom_ext));
    }
}

#include <Geode/modify/SetupAdvFollowPopup.hpp>
class $modify(MySetupAdvFollowPopup, SetupAdvFollowPopup) {
    
    //Initialize Trigger
    bool init(AdvancedFollowTriggerObject* object, CCArray* objects) {
        if (!SetupAdvFollowPopup::init(object, objects)) return false;

        if (!OptionsContainer) {
           OptionsContainer = new MainContainer(); 
           OptionsContainer->Trigger = this;
        }

        return true;
    }

    void onClose(CCObject* sender) {
        log::debug("Clearing containers");
        OptionsContainer = nullptr;
        SetupAdvFollowPopup::onClose(sender);
    }

    //Apply preset
    void selectPremadeClosed(SelectPremadeLayer* layer, int type) {
        AdvancedFollowPreset preset;
        if (!OptionsContainer->PresetMap[type]) {
            log::warn("No preset found with tag {}", type);
            return;
        }

        preset = OptionsContainer->PresetMap[type]->preset;
        auto [entries, mode] = UnwrapPreset(preset);
        

        for (const auto& kv : entries) {

            int propID = kv.id;
            float value = kv.value;

            auto toggleObj = this->m_valueToggles->objectForKey(propID);

            if (!toggleObj)
                this->updateValueControls(propID, value);
            else
                this->valueDidChange(propID, value);

            this->valueChanged(propID, value);
            this->updateValue(propID, value);
        }

        //Update follow checkboxes
        this->updateValueControls(138, preset.follow_p1 ? 1.0f : 0.0f);
        this->updateValueControls(200, preset.follow_p2 ? 1.0f : 0.0f);
        this->updateValueControls(201, preset.follow_c ? 1.0f : 0.0f);

        //Update follow gid last because the previous 3 will overwrite this one even if they're = 0
        this->valueChanged(71, preset.follow_group);

        this->updateMode(mode);
    }

};



#include <Geode/modify/SelectPremadeLayer.hpp>
class $modify(MySelectPremadeLayer, SelectPremadeLayer) {
    struct Fields {
        int m_selectionMode = SelectionMode::SELECT_NORMAL;

        int m_tabColorSelection = 0;
        TextInput* m_nameInput = nullptr;
        TextInput* m_descInput = nullptr;

        PresetPopup* m_createPopup;


        CCLabelBMFont* m_topText = nullptr;

        CCMenu* m_ScrollingButtonMenu = nullptr;
        CCMenu* m_PresetButtonMenu = nullptr;
        CCMenuItemSpriteExtra* m_disableButton;
    };

    CCMenuItemSpriteExtra* CreatePresetButton(PresetItemBundle* bundle) {

        
        auto btnSprite = CCSprite::create(TabIconForIndex(bundle->preset.tabColor));
        btnSprite->setContentSize({40, 40});
        auto btnLabel = CCLabelBMFont::create(bundle->preset.name.c_str(), "goldFont.fnt");

        float maxWidth = 33.5f;
        btnLabel->setWidth(maxWidth);  
        btnLabel->setAlignment(kCCTextAlignmentCenter);

        float labelWidth = btnLabel->getContentSize().width;
        float scale = 0.4f; 
        if (labelWidth * scale > maxWidth) {
            scale = maxWidth / labelWidth; 
        }

        btnLabel->setScale(scale);
        btnLabel->setZOrder(2);
        btnSprite->addChildAtPosition(btnLabel, Anchor::Center);
        bundle->buttonSprite = btnSprite;

        auto button = CCMenuItemSpriteExtra::create(
            btnSprite,
            this,
            menu_selector(MySelectPremadeLayer::onSelectPremadeInt)
        );

        return button;
    }
    

    void LoadPresets() {
        if (!OptionsContainer) return;
        auto fields = m_fields.self();
        bool isReload = false;

        //Check for reloading
        if (!OptionsContainer->PresetMap.empty()) {
            for (auto& [tag, bundle] : OptionsContainer->PresetMap) {
                delete bundle;
            }
            OptionsContainer->PresetMap.clear();
            isReload = true;
        }


        std::vector<std::string> disabledNames;
        if (!DoesFileOrPathExist(OptionsContainer->DisabledPath)) return;
        matjson::Value DisabledJson = FileToJson(OptionsContainer->DisabledPath);

        if (DisabledJson["disabled"].isArray()) {
            for (auto& item : *DisabledJson["disabled"].asArray()) {
                if (auto name = item.asString()) {
                    disabledNames.push_back(*name);
                    log::debug("Found disabled preset: {}", name);
                }
            }
        } else {
            log::warn("'disabled' key is not an array in disabled.json, recreating file...");
            matjson::Value tmp;
            tmp["disabled"] = std::vector<std::string>{};
            JsonToFile(tmp, OptionsContainer->DisabledPath);

            std::string msg = "Dumped disabled"_custom_ext;
            log::debug("{}", msg);

            for (auto& item : *DisabledJson["disabled"].asArray()) {
                if (auto name = item.asString()) {
                    disabledNames.push_back(*name);
                    log::debug("Found disabled preset: {}", name);
                }
            }
        }


        //Set up PresetMap with all items (enabled and disabled) and set their state
        int index = 0;
        {
            //Disable error logging (will probably clog console)
            matjson::Value json = FileToJson(OptionsContainer->TemplatesPath / "Homing Missile"_custom_ext, false);
            if (!json["name"].asString().unwrapOrDefault().empty()) {
                AdvancedFollowPreset preset = PresetFromJson(json);
                OptionsContainer->PresetMap[index] = new PresetItemBundle(preset, nullptr, nullptr, false, false, OptionsContainer->TemplatesPath / "Homing Missile"_custom_ext);
                if (!IsInVector(disabledNames, preset.name)) {
                    OptionsContainer->PresetMap[index]->enabled = true;
                }
                index++;
            }
            
           
        }
            

        {
            //Disable error logging (will probably clog console)
            matjson::Value json = FileToJson(OptionsContainer->TemplatesPath / "Homing Ball"_custom_ext, false);
            if (!json["name"].asString().unwrapOrDefault().empty()) {
                AdvancedFollowPreset preset = PresetFromJson(json);
                OptionsContainer->PresetMap[index] = new PresetItemBundle(preset, nullptr, nullptr, false, false, OptionsContainer->TemplatesPath / "Homing Ball"_custom_ext);
                if (!IsInVector(disabledNames, preset.name)) {
                    OptionsContainer->PresetMap[index]->enabled = true;
                }
                index++;
            }
            
            
            
        }
        int count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(OptionsContainer->PresetsPath)) {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() != OptionsContainer->CustomExt) continue;

           
            matjson::Value json = FileToJson(entry.path());
            if (!json["name"].asString().unwrapOrDefault().empty()) {
                AdvancedFollowPreset preset = PresetFromJson(json);
                OptionsContainer->PresetMap[index] = new PresetItemBundle(preset, nullptr, nullptr, false, false, entry.path());
                if (!IsInVector(disabledNames, preset.name)) {
                    OptionsContainer->PresetMap[index]->enabled = true;
                }
                index++;
            } else {
                count++;
            }
        
        }
        if (Mod::get()->getSettingValue<bool>("display-notifs")) {
            if (count > 1) 
                Notification::create(fmt::format("{} Presets had errors loading. Check the console for details.", count), CCSprite::createWithSpriteFrameName("geode.loader/info-warning.png"))->show();
            else if (count == 1)  
                Notification::create("1 Preset had errors loading. Check the console for details.", CCSprite::createWithSpriteFrameName("geode.loader/info-warning.png"))->show();
        }
        
        
        
        
        if (isReload) fields->m_ScrollingButtonMenu->removeAllChildrenWithCleanup(true);
        
        for (const auto& entry : OptionsContainer->PresetMap) {
            int index = entry.first;
            PresetItemBundle* bundle = entry.second;
            
            auto button = CreatePresetButton(bundle);

            button->setTag(index); //Very important, do not delete

            button->setID(fmt::format("{}-{}", bundle->preset.name, index));
            bundle->button = button;
            
            auto sprite = bundle->buttonSprite;
            sprite->setCascadeColorEnabled(true);
            if (!bundle->enabled) {
                sprite->setColor(DisabledColor);
                
            }
            
            if ((bundle->enabled && fields->m_selectionMode == SelectionMode::SELECT_NORMAL) || (bundle->enabled && fields->m_selectionMode == SelectionMode::SELECT_DELETE) || fields->m_selectionMode == SelectionMode::SELECT_GRAB) {
                fields->m_ScrollingButtonMenu->addChild(button);
            }
                
            
            
            

        }


        auto contentLayer = fields->m_ScrollingButtonMenu->getParent();




        //Calculate content height for scroll content layer
        float buttonWidth = 45;
        float containerWidth = contentLayer->getContentWidth();

        
        int buttonsPerRow = containerWidth / (buttonWidth + 6);
        int totalButtons = fields->m_ScrollingButtonMenu->getChildrenCount();
        int rows = (totalButtons + buttonsPerRow - 1) / buttonsPerRow;

        contentLayer->updateLayout();
        fields->m_ScrollingButtonMenu->updateLayout();
        
        contentLayer->setContentHeight(fields->m_ScrollingButtonMenu->getContentHeight() + 6);
        contentLayer->updateLayout();

        static_cast<ScrollLayer*>(contentLayer->getParent())->scrollToTop();

        this->updateLayout();
        fields->m_PresetButtonMenu->updateLayout(); 



    }

    //Small helper function to change top label
    void UpdateTopText(std::string text) {
        m_fields->m_topText->setString(text.c_str());
    }

    //Get amount of items in bin
    int GetBinCount() {
        int count = 0;
        for (const auto& pair : OptionsContainer->PresetMap)
        {
            PresetItemBundle* bundle = pair.second;
            if (bundle && bundle->inBin)
                count++;
        }
        return count;
    }


    //Settings button presses
    void onFinalizePreset(CCObject* sender) {
        //Create preset
        if (!m_fields->m_nameInput->getString().empty()) {
            auto name = m_fields->m_nameInput->getString();
            if (!IsInPresetMap(OptionsContainer->PresetMap, name)) {
                //Create preset
                AdvancedFollowPreset preset = WrapPreset(OptionsContainer->Trigger);
                preset.name = name;
                preset.description = m_fields->m_descInput->getString();
                preset.tabColor = m_fields->m_tabColorSelection;

                m_fields->m_createPopup->removeFromParentAndCleanup(true);
                m_fields->m_ScrollingButtonMenu->setTouchEnabled(true);
                matjson::Value json = PresetToJson(preset);
                JsonToFile(json, OptionsContainer->PresetsPath / (preset.name + OptionsContainer->CustomExt));
                if (Mod::get()->getSettingValue<bool>("display-notifs"))
                    Notification::create(fmt::format("Created preset {}.", preset.name), CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))->show();
                log::debug("Saved new preset {}.", utils::string::pathToString(OptionsContainer->PresetsPath / (preset.name + OptionsContainer->CustomExt)));
                LoadPresets();

            } else {
                FLAlertLayer::create("Preset Exists", "A preset with this name already exists, please choose another name.", "OK")->show();
                log::warn("Preset exists: {}", name);
            }
        } else {
            FLAlertLayer::create("Missing Name", "Name of preset cannot be empty.", "OK")->show();
        }
    }


    //Creates the creation menu popup
    void onCreateNewPreset(CCObject* sender) {
        auto popup = PresetPopup::create(m_fields->m_ScrollingButtonMenu);
        m_fields->m_tabColorSelection = 0;
        m_fields->m_createPopup = popup;
        auto menu = popup->getChildByID("popup-base");

        //Prevents clicking buttons through menu
        m_fields->m_ScrollingButtonMenu->setTouchEnabled(false);

        auto buttonMenu = CCMenu::create();
        buttonMenu->ignoreAnchorPointForPosition(false);
        buttonMenu->setLayoutOptions(AnchorLayoutOptions::create());
        buttonMenu->setID("button-menu");
        buttonMenu->setAnchorPoint({0.5f, 0});
        buttonMenu->setContentSize({430, 40});
        buttonMenu->setLayout(AxisLayout::create()->setGap(2));

        auto createBtnSpr = NineSlice::create("GJ_button_01.png");
        auto createBtnLbl = CCLabelBMFont::create("create", "goldFont.fnt");
        createBtnSpr->addChildAtPosition(createBtnLbl, Anchor::Center, {0, 3});
        createBtnSpr->setContentSize(createBtnLbl->getContentSize());
        createBtnLbl->setScale(0.75f);
        createBtnSpr->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Center));
        createBtnSpr->updateLayout();
        auto createButton = CCMenuItemSpriteExtra::create(
            createBtnSpr,
            this,
            menu_selector(MySelectPremadeLayer::onFinalizePreset)
        );
        buttonMenu->addChildAtPosition(createButton, Anchor::Center);
        buttonMenu->updateLayout();


        auto iconMenu = CCMenu::create();
        iconMenu->ignoreAnchorPointForPosition(false);
        iconMenu->setLayoutOptions(AnchorLayoutOptions::create());
        iconMenu->setID("icon-menu");
        iconMenu->setAnchorPoint({0.5f, 0});
        iconMenu->setContentSize({300, 40});
        iconMenu->setLayout(AxisLayout::create()->setGap(1.5f));
        iconMenu->setZOrder(5);

        auto iconMenuBG = NineSlice::create("square02_001.png");
        iconMenuBG->setOpacity(100);
        iconMenuBG->setZOrder(2);
        iconMenuBG->setAnchorPoint({0.5f, 0.15f});

        auto iconMenuLabel = CCLabelBMFont::create("icon Type", "bigFont.fnt");
        iconMenuLabel->setScale(0.65f);


        //Tab color icons
        for (int i = 0; i < 11; i++) {
            if (i == 10 && !Mod::get()->getSettingValue<bool>("wokemode")) {
                break;
            }

            auto item = CCMenuItemSpriteExtra::create(
                CCSprite::create(TabIconForIndex(i)),
                this,
                menu_selector(MySelectPremadeLayer::changeSelectedIconType)
            );
            item->setTag(i);
            item->setID(fmt::format("icon-{}", TabIconForIndex(i)));
            item->setZOrder(10);

            if (i == 0) {
                static_cast<CCSprite*>(item->getChildByIndex(0))->setColor(DisabledColor);
            }
 
            popup->iconButtons.push_back(item);
            iconMenu->addChild(item);
        }

        auto nameInput = TextInput::create(250, "Enter a short name", "bigFont.fnt");
        nameInput->setFilter(" abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-_,'");
        nameInput->setLabel("Name");
        nameInput->setID("name-input");
        m_fields->m_nameInput = nameInput;

        auto descInput = TextInput::create(325, "Enter a brief description (optional)", "bigFont.fnt");
        descInput->setLabel("Description");
        descInput->setID("description-input");
        m_fields->m_descInput = descInput;
        

        menu->addChildAtPosition(nameInput, Anchor::Top, {0, -60});
        menu->addChildAtPosition(descInput, Anchor::Center, {0, 13});

        menu->addChildAtPosition(iconMenuBG, Anchor::Bottom, {0, 55});
        menu->addChildAtPosition(iconMenuLabel, Anchor::Center, {0 ,-30});

        menu->addChildAtPosition(buttonMenu, Anchor::Bottom, {0, 10});
        menu->addChildAtPosition(iconMenu, Anchor::Bottom, {0, 55});

        menu->updateLayout();
        iconMenu->updateLayout();

        if (Mod::get()->getSettingValue<bool>("wokemode")) {
            iconMenuBG->setContentSize({313, 40});
        } else {
            iconMenuBG->setContentSize({310, 40});
        }

        this->addChildAtPosition(popup, Anchor::Center);

    }


    //Changes selected tab color
    void changeSelectedIconType(CCObject* sender) {
        m_fields->m_tabColorSelection = sender->getTag();
        for (auto item : m_fields->m_createPopup->iconButtons) {
            if (item->getTag() == sender->getTag()) {
                static_cast<CCSprite*>(item->getChildByIndex(0))->setColor(DisabledColor);
            } else {
                static_cast<CCSprite*>(item->getChildByIndex(0))->setColor(EnabledColor);
            }
        }
    }


    //Deletes selected nodes/presets from bin
    void deleteSelectedFromBin(CCObject* sender) {
        int binSize = GetBinCount();
        if (binSize > 0) {
            createQuickPopup(fmt::format("Delete {} Presets", binSize).c_str(), "Do you want to delete the selected presets?", "no", "yes", [this, binSize](auto, bool btn2) {
                if (btn2) {
                    for (const auto& [tag, bundle] : OptionsContainer->PresetMap)
                    {
                        if (bundle->inBin) {
                            bundle->button->removeFromParent();
                            FileToTrash(bundle->PresetPath);
                        }
                    }
                    if (Mod::get()->getSettingValue<bool>("display-notifs")) {
                        if (binSize > 1)
                            Notification::create(fmt::format("Deleted {} presets.", binSize), CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))->show();
                        else if (binSize == 1)
                            Notification::create("Deleted 1 preset.", CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))->show();
                    }
                        
                    UpdateTopText("Select Preset");
                    m_fields->m_selectionMode = SelectionMode::SELECT_NORMAL;

                    m_fields->m_PresetButtonMenu->getChildByID("delete-preset-button")->setVisible(true);
                    m_fields->m_PresetButtonMenu->getChildByID("normal-preset-button")->setVisible(false);  
                    m_fields->m_PresetButtonMenu->getChildByID("confirm-delete-preset-button")->setVisible(false);
                    m_fields->m_PresetButtonMenu->updateLayout();
                    LoadPresets();
                }
            });
            
        } else {
            FLAlertLayer::create("Empty Bin", "Cannot delete 0 elements, please select some presets to delete", "OK")->show();
        }
    }

    //Toggles selection mode for enabling/disabling
    void toggleDisableSelectMode(CCObject* sender) {
        
        //If in normal mode
        if (m_fields->m_selectionMode == SelectionMode::SELECT_NORMAL) { 
            UpdateTopText("Toggle Presets");
            m_fields->m_selectionMode = SelectionMode::SELECT_GRAB;
            static_cast<CCSprite*>(m_fields->m_disableButton->getChildByIndex(0)->getChildByIndex(0))->setColor(DisabledColor);
            static_cast<CCSprite*>(m_fields->m_disableButton->getChildByIndex(0)->getChildByIndex(1))->setColor(DisabledColor);



            
        //If in disable mode
        } else if (m_fields->m_selectionMode == SelectionMode::SELECT_GRAB) { 
            UpdateTopText("Select Preset");
            m_fields->m_selectionMode = SelectionMode::SELECT_NORMAL;

            
            matjson::Value disabledJson;
            std::vector<std::string> disabledNames;
            for (const auto& [tag, bundle] : OptionsContainer->PresetMap) {
                if (!bundle->enabled) {
                    disabledNames.push_back(bundle->preset.name);
                }
            }

            disabledJson["disabled"] = disabledNames;

            JsonToFile(disabledJson, OptionsContainer->DisabledPath);
            log::debug("saved {} disabled presets to {}", disabledNames.size(), OptionsContainer->DisabledPath);
            static_cast<CCSprite*>(m_fields->m_disableButton->getChildByIndex(0)->getChildByIndex(0))->setColor(EnabledColor);
            static_cast<CCSprite*>(m_fields->m_disableButton->getChildByIndex(0)->getChildByIndex(1))->setColor(EnabledColor);
        
            //Unknown mode
        } else {
            log::warn("Unknown mode selected.");
        }
        
        LoadPresets();

    }


    //Toggle delete selection mdoe
    void toggleDeleteSelectMode(CCObject* sender) {
        if (m_fields->m_selectionMode != SelectionMode::SELECT_GRAB) {
            UpdateTopText("Delete Presets");
            m_fields->m_selectionMode = SelectionMode::SELECT_DELETE;

            m_fields->m_PresetButtonMenu->getChildByID("delete-preset-button")->setVisible(false);
            m_fields->m_PresetButtonMenu->getChildByID("normal-preset-button")->setVisible(true);
            m_fields->m_PresetButtonMenu->getChildByID("confirm-delete-preset-button")->setVisible(true);
            m_fields->m_PresetButtonMenu->updateLayout();
            LoadPresets();
        }
        

    }

    //Normal select mode
    void toggleNormalSelectMode(CCObject* sender) {
        UpdateTopText("Select Preset");
        m_fields->m_selectionMode = SelectionMode::SELECT_NORMAL;

        m_fields->m_PresetButtonMenu->getChildByID("delete-preset-button")->setVisible(true);
        m_fields->m_PresetButtonMenu->getChildByID("normal-preset-button")->setVisible(false);  
        m_fields->m_PresetButtonMenu->getChildByID("confirm-delete-preset-button")->setVisible(false);
        m_fields->m_PresetButtonMenu->updateLayout();

        for (const auto& [tag, bundle] : OptionsContainer->PresetMap)
        {
            bundle->inBin = false;
            if (bundle->buttonSprite != nullptr && bundle->enabled) bundle->buttonSprite->setColor(EnabledColor); 
        }
        LoadPresets();
    }

    //Preset button presses
    //Renamed to prevent looping (Internal)
    void onSelectPremadeInt(CCObject* sender) {
        auto object = static_cast<CCMenuItemSpriteExtra*>(sender);
        switch (m_fields->m_selectionMode) {
            case SelectionMode::SELECT_NORMAL:
                if (Mod::get()->getSettingValue<bool>("confirm-use")) {
                    auto presetName = OptionsContainer->PresetMap[sender->getTag()]->preset.name;
                    createQuickPopup("Select Preset", fmt::format("Do you want to use the preset {}?", presetName), "no", "yes", [this, sender, presetName](auto, bool btn2) {
                        if (btn2) {
                            SelectPremadeLayer::onSelectPremade(sender);
                            if (Mod::get()->getSettingValue<bool>("display-notifs"))
                                Notification::create(fmt::format("Selected preset {}.", presetName), CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))->show();
                            
                        }
                    }); 
                } else {
                    SelectPremadeLayer::onSelectPremade(sender);
                    if (Mod::get()->getSettingValue<bool>("display-notifs"))
                        Notification::create(fmt::format("Selected preset {}.", OptionsContainer->PresetMap[sender->getTag()]->preset.name), CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"))->show();
                }
                
                
                break;

            case SelectionMode::SELECT_DELETE: 
                if (!OptionsContainer->PresetMap[object->getTag()]->inBin) {
                    OptionsContainer->PresetMap[object->getTag()]->inBin = true;
                    OptionsContainer->PresetMap[object->getTag()]->buttonSprite->setColor(DisabledColor);  
                } else {
                    OptionsContainer->PresetMap[object->getTag()]->inBin = false;
                    OptionsContainer->PresetMap[object->getTag()]->buttonSprite->setColor(EnabledColor); 
                }
                break;
            
            
            case SelectionMode::SELECT_GRAB:
                if (OptionsContainer->PresetMap[object->getTag()]->enabled) {
                    OptionsContainer->PresetMap[object->getTag()]->enabled = false;
                    OptionsContainer->PresetMap[object->getTag()]->buttonSprite->setColor(DisabledColor);  
                } else {
                    OptionsContainer->PresetMap[object->getTag()]->enabled = true;
                    OptionsContainer->PresetMap[object->getTag()]->buttonSprite->setColor(EnabledColor); 
                }
                break;
        }  
    }





    //Hooked Funcitons
    




    bool init() {
        if (!SelectPremadeLayer::init()) return false; 
        if (!OptionsContainer) return false;
        auto fields = m_fields.self();

        if (OptionsContainer->PresetMap.size() > 0) OptionsContainer->PresetMap.clear();

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto popupContentLayer = this->getChildByIndex(0);

        //Get existing nodes
        auto popupBgNode = static_cast<CCScale9Sprite*>(popupContentLayer->getChildByIndex(0));
        auto popupText = static_cast<CCLabelBMFont*>(popupContentLayer->getChildByIndex(1));
        auto presetButtonMenu = static_cast<CCMenu*>(popupContentLayer->getChildByIndex(2));
        
        //Create additional nodes
        auto disableButtonMenu = CCMenu::create();

        //Add nodes to fields
        fields->m_PresetButtonMenu = presetButtonMenu;
        fields->m_topText = popupText;

        //Set exitsing nodes ids
        popupContentLayer->setID("popup-content");
        popupText->setID("preset-text");
        popupBgNode->setID("node-bg");
        presetButtonMenu->setID("preset-options-menu");

        //Set additional nodes ids
        disableButtonMenu->setID("disable-button-menu"_spr);

        //Adjust existing nodes settings (spaced based on setting type)
        popupBgNode->setZOrder(-5);

        UpdateTopText("Select Preset");

        popupText->setScale(0.75f);

        presetButtonMenu->ignoreAnchorPointForPosition(false);

        popupContentLayer->setPosition({0, 0});
        popupBgNode->setPosition({0, 0});
        popupText->setPosition({0, 0});
        presetButtonMenu->setPosition({0, 0});

        presetButtonMenu->setContentSize({342, 32});

        popupContentLayer->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Center));
        popupBgNode->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Center));
        popupText->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Top)->setOffset({0, -46}));
        presetButtonMenu->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Center)->setOffset({0, -103}));

        presetButtonMenu->setLayout(AxisLayout::create()->setAxisAlignment(AxisAlignment::Start));

        this->ignoreAnchorPointForPosition(false);
        this->setLayout(AnchorLayout::create());
        this->setPosition({winSize.width / 2, winSize.height / 2});

        //Update created menu
        disableButtonMenu->ignoreAnchorPointForPosition(false);
        disableButtonMenu->setContentSize({32, 32});
        popupContentLayer->addChildAtPosition(disableButtonMenu, Anchor::BottomRight, {-130, 57});



        float width = popupBgNode->getContentWidth();
        float height = popupBgNode->getContentHeight();

        //Create a new scroll layer for preset buttons
        auto presetScrollLayer = ScrollLayer::create({width - 30, height - 80});
        auto presetScrollBar = Scrollbar::create(presetScrollLayer);
        presetScrollLayer->setID("preset-scroll-layer"_spr);
        presetScrollBar->setID("preset-scroll-bar"_spr);
        presetScrollLayer->ignoreAnchorPointForPosition(false);
        presetScrollBar->ignoreAnchorPointForPosition(false);

        //Create a BG for the scroll layer
        auto presetScrollBG = NineSlice::create("square02_001.png");
        presetScrollBG->setContentSize({width - 25, height - 80});
        presetScrollBG->setID("scroll-layer-bg"_spr);
        presetScrollBG->setZOrder(-2);
        presetScrollBG->setOpacity(100);

        
        auto scrollingContentLayer = presetScrollLayer->getChildByID("content-layer");


        //Create a new CCMenu for preset buttons on the scroll layer
        auto scrollingButtonMenu = CCMenu::create();
        fields->m_ScrollingButtonMenu = scrollingButtonMenu;
        scrollingButtonMenu->setID("preset-menu");

        //Evil
        scrollingButtonMenu->setTouchPriority(-520);


        //Create and edit buttons for originall button menu
        auto closeButton = static_cast<CCMenuItemSpriteExtra*>(presetButtonMenu->getChildByIndex(2));
        closeButton->setID("close-button");
        closeButton->setPosition({0, 0});
        closeButton->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Center));

        //Now add custom buttons

        //Create
        auto createBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        createBtnSpr->setScale(0.675f);
        auto createButton = CCMenuItemSpriteExtra::create(
            createBtnSpr,
            this,
            menu_selector(MySelectPremadeLayer::onCreateNewPreset)
        );
        createButton->setID("create-preset-button");
        presetButtonMenu->addChild(createButton);

        //Delete (First)
        auto deleteBtnSpr = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
        deleteBtnSpr->setScale(0.675f);
        auto deleteButton = CCMenuItemSpriteExtra::create(
            deleteBtnSpr,
            this,
            menu_selector(MySelectPremadeLayer::toggleDeleteSelectMode)
        );
        deleteButton->setID("delete-preset-button");
        deleteButton->setLayoutOptions(AnchorLayoutOptions::create());
        presetButtonMenu->addChild(deleteButton);

        //Confirm Delete (Second)
        //Create a composite sprite
        auto confirmDelSprComp = CCNode::create(); 
        auto confirmDelSprBase = CCSprite::createWithSpriteFrameName("geode.loader/baseCircle_BigAlt_Green.png");
        confirmDelSprComp->addChild(confirmDelSprBase);
        auto confirmDelSprIcon = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
        confirmDelSprIcon->setZOrder(3);
        confirmDelSprIcon->setScale(1.25f);
        confirmDelSprComp->addChild(confirmDelSprIcon);

        auto confirmDeleteButton = CCMenuItemSpriteExtra::create(
            confirmDelSprComp,
            this,
            menu_selector(MySelectPremadeLayer::deleteSelectedFromBin)
        );
        confirmDeleteButton->setLayout(AnchorLayout::create());
        confirmDelSprComp->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Center));
        confirmDelSprComp->setScale(0.575f);
        confirmDeleteButton->updateAnchoredPosition(Anchor::Center);
        confirmDeleteButton->setID("confirm-delete-preset-button");
        confirmDeleteButton->setContentSize(deleteButton->getContentSize());
        presetButtonMenu->addChild(confirmDeleteButton);

        confirmDeleteButton->setVisible(false);
        confirmDeleteButton->updateLayout();

        //Exit Delete (Third)
        auto returnBtnSpr = CCSprite::createWithSpriteFrameName("GJ_backBtn_001.png");
        returnBtnSpr->setScale(0.625f);
        auto returnButton = CCMenuItemSpriteExtra::create(
            returnBtnSpr,
            this,
            menu_selector(MySelectPremadeLayer::toggleNormalSelectMode)
        );
        returnButton->setID("normal-preset-button");
        returnButton->setContentSize(deleteButton->getContentSize());
        returnButton->setPosition(deleteButton->getPosition());
        returnButton->setLayoutOptions(AnchorLayoutOptions::create());
        returnButton->setVisible(false);
        presetButtonMenu->addChild(returnButton);

        //Disable
        //Create a composite sprite
        auto disableButtonSprComp = CCNode::create();
        auto disableSprBase = CCSprite::createWithSpriteFrameName("geode.loader/baseCircle_BigAlt_Green.png");
        disableButtonSprComp->addChild(disableSprBase);
        auto disableSprIcon = CCSprite::createWithSpriteFrameName("edit_downBtn2_001.png");
        disableSprIcon->setZOrder(3);
        disableSprIcon->setScale(1.25f);
        disableButtonSprComp->addChild(disableSprIcon);
        disableButtonSprComp->setScale(0.575f);
        disableButtonSprComp->setContentSize({32, 32});

        auto disableButton = CCMenuItemSpriteExtra::create(
            disableButtonSprComp,
            this,
            menu_selector(MySelectPremadeLayer::toggleDisableSelectMode)
        );
        fields->m_disableButton = disableButton;
        disableButton->setContentSize({32, 32});
        disableButtonSprComp->ignoreAnchorPointForPosition(true);
        disableButton->setLayoutOptions(AnchorLayoutOptions::create());

        //Awful things
        disableButtonMenu->setTouchEnabled(true);
        disableButtonMenu->setTouchPriority(-520);
        disableButtonMenu->addChildAtPosition(disableButton, Anchor::Center);


        //Get original 2 preset buttons and remove them
        presetButtonMenu->getChildByTag(0)->removeFromParent();
        presetButtonMenu->getChildByTag(1)->removeFromParent();

        presetButtonMenu->updateLayout();

        //Setup scroll layer
        scrollingContentLayer->setLayout(AnchorLayout::create());
        scrollingButtonMenu->ignoreAnchorPointForPosition(false);
        scrollingButtonMenu->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Center));
        scrollingButtonMenu->setPosition({0, 0});

        scrollingButtonMenu->setLayout(AxisLayout::create(Axis::Row)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisAlignment(AxisAlignment::End)
            ->setCrossAxisLineAlignment(AxisAlignment::End)
            ->setGap(6)
            ->ignoreInvisibleChildren(true)
            ->setAutoScale(false)
            ->setGrowCrossAxis(true)
            ->setCrossAxisOverflow(true)
        );

        scrollingButtonMenu->setContentSize({
            scrollingContentLayer->getContentWidth() - 13,
            scrollingContentLayer->getContentHeight()
        });
        scrollingButtonMenu->setTouchPriority(-510);
        scrollingButtonMenu->setTouchEnabled(true);
        scrollingContentLayer->addChild(scrollingButtonMenu);
        

        LoadPresets();

        
        popupContentLayer->addChildAtPosition(presetScrollLayer, Anchor::Center, {-5, 7});
        popupContentLayer->addChildAtPosition(presetScrollBar, Anchor::Center, {169.75f, 7});
        popupContentLayer->addChildAtPosition(presetScrollBG, Anchor::Center, {-5, 7});
        
        this->setTouchMode(kCCTouchesOneByOne);

        this->setKeypadEnabled(true);
        this->setTouchEnabled(true);

        return true;
    }
};