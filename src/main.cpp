#include <Geode/Geode.hpp>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "main.hpp"

using namespace geode::prelude;

PresetsOptions* Options = nullptr;

$on_mod(Loaded) {
    //templates
    if (!std::filesystem::exists(Mod::get()->getSettingValue<std::filesystem::path>("template-path"))) {
        std::filesystem::create_directories(Mod::get()->getSaveDir() / "templates");
    }
    if (!std::filesystem::exists(Mod::get()->getSettingValue<std::filesystem::path>("template-path") / "robtopHomingMissile.json") || !std::filesystem::exists(Mod::get()->getSettingValue<std::filesystem::path>("template-path") / "robtopHomingBall.json") || !std::filesystem::exists(Mod::get()->getSettingValue<std::filesystem::path>("template-path") / "template.json")) {
        log::debug("created templates at {}", createTemplates());
    }

    //presets
    if (!std::filesystem::exists(Mod::get()->getSettingValue<std::filesystem::path>("presets-path"))) {
        std::filesystem::create_directories(Mod::get()->getSettingValue<std::filesystem::path>("presets-path"));
    }

    //disabled list
    if (!std::filesystem::exists(Mod::get()->getSaveDir() / "disabled.json")) {

        matjson::Value disabledJson;
        disabledJson["disabled"] = std::vector<std::string>{};


        auto path = Mod::get()->getSaveDir() / "disabled.json";
        std::ofstream file(path); 
        file << disabledJson.dump(4);      
        file.close();
        



        log::debug("created disabled list at {}", Mod::get()->getSaveDir() / "disabled.json");
    }
}

#include <Geode/modify/SetupAdvFollowPopup.hpp>
class $modify(MySetupAdvFollowPopup, SetupAdvFollowPopup) {

    std::pair<std::vector<int>, std::vector<float>> unwrapPreset(AdvancedFollowPreset& preset) {
        std::vector<int> presetIDs;
        std::vector<float> presetValues;

        for (const auto& entry : memberMappings) {

            presetIDs.push_back(entry.propID);

            if (entry.intMember) {
                presetValues.push_back(static_cast<float>(preset.*(entry.intMember)));
            }
            else if (entry.floatMember) {
                presetValues.push_back(preset.*(entry.floatMember));
            }
            else if (entry.boolMember) {
                presetValues.push_back(preset.*(entry.boolMember) ? 1.0f : 0.0f);
            }
            else if (entry.enumAFMMember) {
                presetValues.push_back(static_cast<float>(static_cast<int>(preset.*(entry.enumAFMMember))));
            }
            else if (entry.enumAFP2Member) {
                presetValues.push_back(static_cast<float>(static_cast<int>(preset.*(entry.enumAFP2Member))));
            }
        }

        return {presetIDs, presetValues};

    }

    static AdvancedFollowPreset wrapPreset() {
        
        AdvancedFollowPreset preset = {};

        for (const auto& entry : memberMappings) {

            float value = Options->trigger->getValue(entry.propID);
            if (!value) continue;

            if (entry.intMember) {
                preset.*(entry.intMember) = static_cast<int>(value);
            }
            else if (entry.floatMember) {
                preset.*(entry.floatMember) = value;
            }
            else if (entry.boolMember) {
                preset.*(entry.boolMember) = (value != 0.f);
            }
            else if (entry.enumAFMMember) {
                preset.*(entry.enumAFMMember) =
                    static_cast<AdvancedFollowMode>(static_cast<int>(value));
            }
            else if (entry.enumAFP2Member) {
                preset.*(entry.enumAFP2Member) =
                    static_cast<AdvancedFollowPage2Mode>(static_cast<int>(value));
            }
        }

        return preset;
    }
    

    bool init(AdvancedFollowTriggerObject* object, CCArray* objects) {
        if (!SetupAdvFollowPopup::init(object, objects)) return false;

        if (!Options) {
           Options = new PresetsOptions(); 
           Options->trigger = this;
        }

        return true;
    }


    void onClose(CCObject* sender) {
        log::debug("clearing presets...");
        Options = nullptr;
        SetupAdvFollowPopup::onClose(sender);
    }


    void selectPremadeClosed(SelectPremadeLayer* layer, int type) {
         
        
        AdvancedFollowPreset preset;
        std::vector<int> presetIDS;
        std::vector<float> presetValues;

        auto it = std::find_if(
            Options->loadedPresets->begin(),
            Options->loadedPresets->end(),
            [type](const LoadedPreset& p) {
                return p.tag == type;
            }
        );

        if (it != Options->loadedPresets->end()) {
            preset = it->preset;
        }
        else {
            log::warn("No preset found for type {}", type);
        }

        int mode;

        for (auto mapItem : memberMappings) {
            
            
            if (mapItem.intMember) {
                presetValues.push_back(static_cast<float>(preset.*(mapItem.intMember)));
                presetIDS.push_back(mapItem.propID);
            }
            else if (mapItem.floatMember) {
                presetValues.push_back(preset.*(mapItem.floatMember));
                presetIDS.push_back(mapItem.propID);
            }
            else if (mapItem.boolMember) {
                presetValues.push_back(preset.*(mapItem.boolMember) ? 1.0f : 0.0f);
                presetIDS.push_back(mapItem.propID);
            }
            else if (mapItem.enumAFMMember) {
                presetValues.push_back(static_cast<float>(static_cast<int>(preset.*(mapItem.enumAFMMember))));
                presetIDS.push_back(mapItem.propID);
                
            }
            else if (mapItem.enumAFP2Member) { 
                presetValues.push_back(static_cast<float>(static_cast<int>(preset.*(mapItem.enumAFP2Member))));
                presetIDS.push_back(mapItem.propID);
                mode = static_cast<int>(preset.*(mapItem.enumAFP2Member));  
            }
            
        }


        for (int i = 0; i < presetIDS.size(); ++i) {
            this->updateValueControls(presetIDS[i], presetValues[i]);
            this->valueChanged(presetIDS[i], presetValues[i]);
        }

        this->updateMode(mode);

    }

    
};

enum SelectionMode  {
    SELECT_NORMAL, //click like a normal button
    SELECT_DELETE, //select for deletion
    SELECT_GRAB //select for a button click (maybe)
};

#include <Geode/modify/SelectPremadeLayer.hpp>
class $modify(MySelectPremadeLayer, SelectPremadeLayer) {
    struct Fields {
        createPresetPopup* m_popup = nullptr;
        TextInput* m_nameInput = nullptr;
        TextInput* m_descInput = nullptr;
        CCLabelBMFont* m_topText = nullptr;

        bool m_disableMode = false;
        std::unordered_map<int, bool> m_nodeState; //tag isEnabled

        //bear with me
        ScrollLayer* m_mainScrollLayer = nullptr;
        CCMenu* m_PresetSettingsMenu = nullptr;
        GenericContentLayer* m_scrollContentLayer = nullptr;
        CCMenu* m_ScrollingButtonMenu = nullptr;
        CCNode* m_popupContentLayer = nullptr;

        int m_itemSpriteSelection = 0;
        std::string m_presetName = "temp";


        int m_selectionMode = SelectionMode::SELECT_NORMAL;

        std::vector<CCMenuItemSpriteExtra*> m_NodeBin;
    };

    void updateTopText(std::string text) {
        m_fields->m_topText->setString(text.c_str());
    }

    void reloadPresets() {
        if (!Options) return;

        auto presetFolderPath = Mod::get()->getSettingValue<std::filesystem::path>("presets-path");
        auto templateFolderPath = Mod::get()->getSettingValue<std::filesystem::path>("template-path");


        if (Options->loadedPresets) {
            Options->loadedPresets->clear();
        } else {
            Options->loadedPresets = new std::vector<LoadedPreset>();
            
        }

        if (Options->disabledPresets) {
            Options->disabledPresets->clear();
        } else {
            Options->disabledPresets = new std::vector<LoadedPreset>();
        }

        std::vector<std::string> disabledNames;
        auto disabledPath = Mod::get()->getSaveDir() / "disabled.json";
        if (!std::filesystem::exists(disabledPath)) return;
        std::ifstream file(disabledPath);

        std::string disabledJson((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        auto parsed = matjson::parse(disabledJson);

        if (!parsed) {
            log::warn("Failed to parse disabled.json: {}", parsed.unwrapErr());
        } else {
            auto root = *parsed;
            if (root["disabled"].isArray()) {
                for (auto& val : *root["disabled"].asArray()) {
                    if (auto strVal = val.asString())
                        disabledNames.push_back(*strVal);
                }
            } else {
                log::warn("'disabled' key is not an array in disabled.json, recreating file...");
                matjson::Value disabledJson;
                disabledJson["disabled"] = std::vector<std::string>{};

                auto path = Mod::get()->getSaveDir() / "disabled.json";
                std::ofstream file(path); 
                file << disabledJson.dump(4);      
                file.close();
            }
        }

        int index = 0;
        int disIndex = 5000;

        m_fields->m_nodeState.clear();

        //load robtop presets first
        try {  
            std::ifstream file(templateFolderPath / "robtopHomingMissile.json");
            std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            matjson::Value jsonValue = matjson::parse(jsonStr).unwrap();
            AdvancedFollowPreset preset = presetFromJson(jsonValue);
            if (std::find(disabledNames.begin(), disabledNames.end(), preset.name) == disabledNames.end()) {
                Options->loadedPresets->push_back({preset, index, (templateFolderPath / "robtopHomingMissile.json").string()});
                log::debug("Loaded preset from {}", (templateFolderPath / "robtopHomingMissile.json").string());
                m_fields->m_nodeState[index] = true;
                index++;
            } else {
                Options->disabledPresets->push_back({preset, disIndex, (templateFolderPath / "robtopHomingMissile.json").string()});
                m_fields->m_nodeState[disIndex] = false;
                disIndex++;
            }
                

            
        }
        catch (const std::exception& e) {
            log::warn("Failed to parse preset {}: {}", (templateFolderPath / "robtopHomingMissile.json").string(), e.what());
        }

        try {  
            std::ifstream file(templateFolderPath / "robtopHomingBall.json");
            std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            matjson::Value jsonValue = matjson::parse(jsonStr).unwrap();
            AdvancedFollowPreset preset = presetFromJson(jsonValue);
            
            if (std::find(disabledNames.begin(), disabledNames.end(), preset.name) == disabledNames.end()) {
                Options->loadedPresets->push_back({preset, index, (templateFolderPath / "robtopHomingBall.json").string()});
                log::debug("Loaded preset from {}", (templateFolderPath / "robtopHomingBall.json").string());
                m_fields->m_nodeState[index] = true;
                index++;
            } else {
                Options->disabledPresets->push_back({preset, disIndex, (templateFolderPath / "robtopHomingBall.json").string()});
                m_fields->m_nodeState[disIndex] = false;
                disIndex++;
            }

            
        }
        catch (const std::exception& e) {
            log::warn("Failed to parse preset {}: {}", (templateFolderPath / "robtopHomingBall.json").string(), e.what());
        }


        for (const auto& entry : std::filesystem::directory_iterator(presetFolderPath))
        {   
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() != ".json") continue;

            std::ifstream file(entry.path());
            if (!file.is_open()) {
                log::warn("Failed to open preset file: {}", entry.path().string());
                continue;
            }
            std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            try {  
                matjson::Value jsonValue = matjson::parse(jsonStr).unwrap();
                AdvancedFollowPreset preset = presetFromJson(jsonValue);

                if (std::find(disabledNames.begin(), disabledNames.end(), preset.name) == disabledNames.end()) {
                    Options->loadedPresets->push_back({preset, index, entry.path().string()});        
                    m_fields->m_nodeState[index] = true;   
                    index++;
                    log::debug("Loaded preset from {}", entry.path().string());
                } else {
                    Options->disabledPresets->push_back({preset, disIndex, entry.path().string()});     
                    m_fields->m_nodeState[disIndex] = false;
                    disIndex++;
                }   
                    

                
                
            }
            catch (const std::exception& e) {
                log::warn("Failed to parse preset {}: {}", entry.path().string(), e.what());
            }
            
        }

        if (m_fields->m_ScrollingButtonMenu->getChildrenCount() > 0) {
            m_fields->m_ScrollingButtonMenu->removeAllChildren();
        }
        


        for (const auto& entry : *Options->loadedPresets) {
            auto preset = entry.preset;
            auto tag = entry.tag;

            auto button = createPresetButton(preset, tag);
            static_cast<NineSlice*>(button->getChildByIndex(0))->setCascadeColorEnabled(true);
            
            m_fields->m_ScrollingButtonMenu->addChild(button);

        }

        if (m_fields->m_disableMode) {
            for (const auto& entry : *Options->disabledPresets) {
                auto preset = entry.preset;
                auto tag = (entry.tag);

                auto button = createPresetButton(preset, tag);
                static_cast<NineSlice*>(button->getChildByIndex(0))->setCascadeColorEnabled(true);
                static_cast<NineSlice*>(button->getChildByIndex(0))->setColor({ 129, 129, 129 });
                
                m_fields->m_ScrollingButtonMenu->addChild(button);

            }
        }

        float buttonWidth = 45;
        float containerWidth = m_fields->m_scrollContentLayer->getContentWidth();

        int buttonsPerRow = containerWidth / (buttonWidth + 6);
        int totalButtons = m_fields->m_ScrollingButtonMenu->getChildrenCount();
        int rows = (totalButtons + buttonsPerRow - 1) / buttonsPerRow;
        if (m_fields->m_ScrollingButtonMenu->getParent() != m_fields->m_scrollContentLayer) {
            m_fields->m_scrollContentLayer->addChild(m_fields->m_ScrollingButtonMenu);
        }
        m_fields->m_scrollContentLayer->updateLayout();

        


        m_fields->m_ScrollingButtonMenu->updateLayout();
        
        
        m_fields->m_scrollContentLayer->setContentHeight(m_fields->m_ScrollingButtonMenu->getContentHeight());
        m_fields->m_scrollContentLayer->updateLayout();
        m_fields->m_mainScrollLayer->scrollToTop();
        this->updateLayout();
        
        m_fields->m_PresetSettingsMenu->updateLayout(); 

        
        
        
    }

    

    bool init() {
        if (!SelectPremadeLayer::init()) return false;
        
        if (!Options) return false;

        //get premades and store in a vector
        auto presetFolderPath = Mod::get()->getSettingValue<std::filesystem::path>("presets-path");
        auto templateFolderPath = Mod::get()->getSettingValue<std::filesystem::path>("template-path");
        

        if (Options->loadedPresets) {
            Options->loadedPresets->clear();
        } else {
            Options->loadedPresets = new std::vector<LoadedPreset>();
        }

        if (Options->disabledPresets) {
            Options->disabledPresets->clear();
        } else {
            Options->disabledPresets = new std::vector<LoadedPreset>();
        }





        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto popupContentLayer = this->getChildByIndex(0);

        auto popupBgNode = static_cast<CCScale9Sprite*>(popupContentLayer->getChildByIndex(0));
        auto popupText = static_cast<CCLabelBMFont*>(popupContentLayer->getChildByIndex(1));
        auto presetButtonMenu = static_cast<CCMenu*>(popupContentLayer->getChildByIndex(2));
        auto disableButtonMenu = CCMenu::create();


        m_fields->m_PresetSettingsMenu = presetButtonMenu;
        
        popupContentLayer->setID("popup-content");
        popupText->setID("preset-text");
        popupBgNode->setID("node-bg");
        presetButtonMenu->setID("preset-options-menu");
        disableButtonMenu->setID("disable-button-menu");

        popupBgNode->setZOrder(-5);

        popupText->setScale(0.75f);

        presetButtonMenu->ignoreAnchorPointForPosition(false);

        popupContentLayer->setPosition({0, 0});
        popupBgNode->setPosition({0, 0});
        popupText->setPosition({0, 0});
        presetButtonMenu->setPosition({0, 0});

        popupContentLayer->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Center));
        popupBgNode->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Center));
        popupText->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Top)->setOffset(ccp(0, -46)));
        presetButtonMenu->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Center)->setOffset(ccp(0, -103)));

        this->ignoreAnchorPointForPosition(false);
        this->setLayout(AnchorLayout::create());
        this->setPosition({
            winSize.width / 2,
            winSize.height / 2
        });

        m_fields->m_topText = popupText;
        updateTopText("Select Preset");
        disableButtonMenu->ignoreAnchorPointForPosition(false);
        disableButtonMenu->setContentSize({32, 32});
        popupContentLayer->addChildAtPosition(disableButtonMenu, Anchor::BottomRight, ccp(-130, 57));

        float width = popupBgNode->getContentWidth();
        float height = popupBgNode->getContentHeight();

        auto popupNodeScrollingLayer = ScrollLayer::create({
            width - 30,
            height - 80
        });
        popupNodeScrollingLayer->setID("node-scroll-layer"_spr);

        auto popupScrollingBG = NineSlice::create("square02_001.png");
        popupScrollingBG->setContentSize({
            width - 25,
            height - 80
        });
        popupScrollingBG->setID("scroll-layer-bg"_spr);
        popupScrollingBG->setZOrder(-2);
        popupScrollingBG->setOpacity(100);

        popupNodeScrollingLayer->ignoreAnchorPointForPosition(false);
        auto scrollingContentLayer = popupNodeScrollingLayer->getChildByID("content-layer");

        



        auto scrollingButtonMenu = CCMenu::create();
        //die
        scrollingButtonMenu->setTouchPriority(-520);
        scrollingButtonMenu->setID("preset-menu");

        auto closeButton = static_cast<CCMenuItemSpriteExtra*>(presetButtonMenu->getChildByIndex(2));
        closeButton->setID("close-button");

        presetButtonMenu->setLayout(AxisLayout::create()
            ->setAxisAlignment(AxisAlignment::Start)
    
        );
        presetButtonMenu->setContentSize({
            342,
            32
        });

        

        closeButton->setPosition({0, 0});
        closeButton->setLayoutOptions(AnchorLayoutOptions::create()->setAnchor(Anchor::Center));

        auto exportBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        exportBtnSpr->setScale(0.675f);
        
        auto exportButton = CCMenuItemSpriteExtra::create(
            exportBtnSpr,
            this,
            menu_selector(MySelectPremadeLayer::onCreateNewPreset)
        );
        exportButton->setID("export-preset-button");
        presetButtonMenu->addChild(exportButton);

        auto deleteBtnSpr = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
        deleteBtnSpr->setScale(0.675f);
        
        auto SMdeleteButton = CCMenuItemSpriteExtra::create(
            deleteBtnSpr,
            this,
            menu_selector(MySelectPremadeLayer::toggleDeleteSelectMode)
        );
        SMdeleteButton->setID("SMdelete-preset-button");
        SMdeleteButton->setLayoutOptions(AnchorLayoutOptions::create());
        presetButtonMenu->addChild(SMdeleteButton);


        auto confirmDelComp = CCNode::create();
        auto cmfDelbase = CCSprite::createWithSpriteFrameName("geode.loader/baseCircle_BigAlt_Green.png");
        confirmDelComp->addChild(cmfDelbase);
        auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
        check->setZOrder(3);
        check->setScale(1.25f);
        confirmDelComp->addChild(check);
        
        

        
        auto confDeleteButton = CCMenuItemSpriteExtra::create(
            confirmDelComp,
            this,
            menu_selector(MySelectPremadeLayer::deleteSelectedFromBin)
        );
        
        confDeleteButton->setLayout(AnchorLayout::create());
        confirmDelComp->setLayoutOptions(AnchorLayoutOptions::create()
            ->setAnchor(Anchor::Center)
        );
        confirmDelComp->setScale(0.575f);
        confDeleteButton->updateAnchoredPosition(Anchor::Center);
        
        confDeleteButton->setID("confirm-delete-preset-button");
        
        confDeleteButton->setContentSize(SMdeleteButton->getContentSize());
        presetButtonMenu->addChild(confDeleteButton);
        confDeleteButton->setVisible(false);
        confDeleteButton->updateLayout();

        auto normalBtnSpr = CCSprite::createWithSpriteFrameName("GJ_backBtn_001.png");
        normalBtnSpr->setScale(0.625f);

        auto SMnormalButton = CCMenuItemSpriteExtra::create(
            normalBtnSpr,
            this,
            menu_selector(MySelectPremadeLayer::toggleNormalSelectMode)
        );
        SMnormalButton->setID("SMnormal-preset-button");
        SMnormalButton->setContentSize(SMdeleteButton->getContentSize());
        SMnormalButton->setPosition(SMdeleteButton->getPosition());
        SMnormalButton->setLayoutOptions(AnchorLayoutOptions::create());
        SMnormalButton->setVisible(false);
        presetButtonMenu->addChild(SMnormalButton);

        auto disableButtonSprComp = CCNode::create();
        auto disableSprBase = CCSprite::createWithSpriteFrameName("geode.loader/baseCircle_BigAlt_Green.png");
        disableButtonSprComp->addChild(disableSprBase);
        auto disableIcon = CCSprite::createWithSpriteFrameName("edit_downBtn2_001.png");
        disableIcon->setZOrder(3);
        disableIcon->setScale(1.25f);
        disableButtonSprComp->addChild(disableIcon);
        disableButtonSprComp->setScale(0.575f);
        disableButtonSprComp->setContentSize({32, 32});

        
        auto disableButton = CCMenuItemSpriteExtra::create(
            disableButtonSprComp,
            this,
            menu_selector(MySelectPremadeLayer::toggleDisableSelectMode)
        );
        disableButton->setContentSize({32, 32});
        disableButtonSprComp->ignoreAnchorPointForPosition(true);
        disableButton->setLayoutOptions(AnchorLayoutOptions::create());

        
        disableButtonMenu->setTouchEnabled(true);
        disableButtonMenu->setTouchPriority(-520);
        disableButtonMenu->addChildAtPosition(disableButton, Anchor::Center);

        auto item1 = static_cast<CCMenuItemSpriteExtra*>(presetButtonMenu->getChildByIndex(0));
        auto item2 = static_cast<CCMenuItemSpriteExtra*>(presetButtonMenu->getChildByIndex(1));

        item1->removeFromParent();
        item2->removeFromParent();

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
        

        scrollingButtonMenu->setContentSize({scrollingContentLayer->getContentWidth() - 13, scrollingContentLayer->getContentHeight()});
        
        if (m_fields->m_ScrollingButtonMenu != nullptr) {
            m_fields->m_ScrollingButtonMenu->removeFromParent();
        }

        m_fields->m_ScrollingButtonMenu = scrollingButtonMenu;
        m_fields->m_mainScrollLayer = popupNodeScrollingLayer;
        m_fields->m_scrollContentLayer = static_cast<GenericContentLayer*>(scrollingContentLayer);
        m_fields->m_popupContentLayer = popupContentLayer;
        reloadPresets();

   
        m_fields->m_popupContentLayer->addChildAtPosition(popupNodeScrollingLayer, Anchor::Center, ccp(0, 7));
        m_fields->m_popupContentLayer->addChildAtPosition(popupScrollingBG, Anchor::Center, ccp(0, 7));

        this->setTouchMode(kCCTouchesOneByOne);

        this->setKeypadEnabled(true);
        this->setTouchEnabled(true);

        return true;
    }

    void onFinalizePreset(CCObject* sender) {
        if (!m_fields->m_nameInput->getString().empty()) {
            m_fields->m_popup->removeFromParentAndCleanup(true);
        
            auto name = m_fields->m_nameInput->getString();

            auto it = std::find_if(
                Options->loadedPresets->begin(),
                Options->loadedPresets->end(),
                [name](const LoadedPreset& p) {
                    return p.preset.name == name;
                }
            );

            if (it != Options->loadedPresets->end()) {
                log::warn("Preset exists: {}", name);
            }
            else {
                if (!name.empty() > 0) {
                    
                    AdvancedFollowPreset preset = MySetupAdvFollowPopup::wrapPreset();
                    preset.name = name;
                    preset.description = m_fields->m_descInput->getString();
                    preset.tabColor = m_fields->m_itemSpriteSelection;
                    
                    matjson::Value jsonTemp = presetToJson(preset);
                    {
                        auto path = geode::Mod::get()->getSettingValue<std::filesystem::path>("presets-path") / (fmt::to_string(preset.name) + ".json");
                        std::ofstream file(path);
                        file << jsonTemp.dump(4);
                        file.close();
                    }
                    reloadPresets();
                }
            }
            
            m_fields->m_ScrollingButtonMenu->setTouchEnabled(true);

        } else {
            FLAlertLayer::create("Missing Name", "Name of preset cannot be empty.", "OK")->show();
        }
        
        

        
    }


    void onCreateNewPreset(CCObject* sender) {
        auto popup = createPresetPopup::create();
        m_fields->m_popup = popup;
        auto menu = popup->getChildByID("popup-base");
        m_fields->m_ScrollingButtonMenu->setTouchEnabled(false);
        auto buttonMenu = CCMenu::create();
        buttonMenu->ignoreAnchorPointForPosition(false);
        buttonMenu->setLayoutOptions(AnchorLayoutOptions::create());
        buttonMenu->setID("button-menu");
        buttonMenu->setAnchorPoint({0.5f, 0});
        buttonMenu->setContentSize({430, 40});
        buttonMenu->setLayout(AxisLayout::create()
            ->setGap(2)
        );

        
        auto createBtnSpr = NineSlice::create("GJ_button_01.png");
        auto createBtnLbl = CCLabelBMFont::create("create", "goldFont.fnt");
        createBtnSpr->addChildAtPosition(createBtnLbl, Anchor::Center, ccp(0, 3));
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
        iconMenu->setLayout(AxisLayout::create()
            ->setGap(1.5f)
        );
        iconMenu->setZOrder(5);

        auto iconMenuBG = NineSlice::create("square02_001.png");
        iconMenuBG->setOpacity(100);
        iconMenuBG->setZOrder(2);
        iconMenuBG->setAnchorPoint({0.5f, 0.15f});

        auto iconMenuLabel = CCLabelBMFont::create("icon Type", "bigFont.fnt");
        iconMenuLabel->setScale(0.65f);


        for (int i = 0; i < 11; i++) {
            if (i == 10 && !Mod::get()->getSettingValue<bool>("wokemode")) {
                break;
            }
            std::string imgName = getIconForIndex(i);
            auto item = CCMenuItemSpriteExtra::create(
                CCSprite::create(imgName.c_str()),
                this,
                menu_selector(MySelectPremadeLayer::changeSelectedIconType)
            );
            item->setTag(i);
            item->setZOrder(10);

            if (i == 0) {
                static_cast<NineSlice*>(item->getChildByIndex(0))->setColor({ 104, 104, 104 });
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
        

        menu->addChildAtPosition(nameInput, Anchor::Top, ccp(0, -60));
        menu->addChildAtPosition(descInput, Anchor::Center, ccp(0, 13));

        menu->addChildAtPosition(iconMenuBG, Anchor::Bottom, ccp(0, 55));
        menu->addChildAtPosition(iconMenuLabel, Anchor::Center, ccp(0, -30));

        menu->addChildAtPosition(buttonMenu, Anchor::Bottom, ccp(0, 10));
        menu->addChildAtPosition(iconMenu, Anchor::Bottom, ccp(0, 55));
        

        

        menu->updateLayout();
        iconMenu->updateLayout();

        if (Mod::get()->getSettingValue<bool>("wokemode")) {
            iconMenuBG->setContentSize({313, 40});
        } else {
            iconMenuBG->setContentSize({310, 40});
        }

        

        this->addChildAtPosition(popup, Anchor::Center);
    }

    void changeSelectedIconType(CCObject* sender) {
        m_fields->m_itemSpriteSelection = sender->getTag();
        for (auto item : m_fields->m_popup->iconButtons) {
            if (item->getTag() == sender->getTag()) {
                static_cast<NineSlice*>(item->getChildByIndex(0))->setColor({ 104, 104, 104 });
            } else {
                static_cast<NineSlice*>(item->getChildByIndex(0))->setColor({ 255, 255, 255 });
            }
        }
    }

    std::string getIconForIndex(int i) {
        switch (i) {
            case TabColor::GRAY:
                return "GJ_button_04.png";
            case TabColor::COOL_GRAY:
                return "GJ_button_05.png";
            case TabColor::BLACK:
                return "GE_button_05.png";
            case TabColor::RED:
                return "GJ_button_06.png";
            case TabColor::PINK:
                return "GJ_button_03.png";
            case TabColor::LIGHT_PINK:
                return "GE_button_02.png";
            case TabColor::PURPLE:
                return "GE_button_03.png";
            case TabColor::CYAN:
                return "GJ_button_02.png";
            case TabColor::BLUE:
                return "GE_button_01.png";
            case TabColor::GREEN:
                return "GJ_button_01.png";
            case TabColor::A_SECRET_THIRD_OPTION:
                return "GE_button_04.png";
            default:
                return "GJ_button_04.png";
        }
    }

    void deleteSelectedFromBin(CCObject* sender) {
        if (m_fields->m_NodeBin.size() > 0) {
            createQuickPopup("Delete Presets", "Do you want to delete the selected presets?", "no", "yes", [this](auto, bool btn2) {
            if (btn2) {
                this->updateTopText("Select Preset");
                m_fields->m_selectionMode = SelectionMode::SELECT_NORMAL;

                m_fields->m_PresetSettingsMenu->getChildByID("SMdelete-preset-button")->setVisible(true);
                m_fields->m_PresetSettingsMenu->getChildByID("SMnormal-preset-button")->setVisible(false);  
                m_fields->m_PresetSettingsMenu->getChildByID("confirm-delete-preset-button")->setVisible(false);
                m_fields->m_PresetSettingsMenu->updateLayout();

                //delete jsons
                
                
                    std::vector<int> tagsToRemove;
                    for (auto obj : m_fields->m_NodeBin) {
                        tagsToRemove.push_back(obj->getTag());
                    }

            
                    Options->loadedPresets->erase(
                        std::remove_if(
                            Options->loadedPresets->begin(),
                            Options->loadedPresets->end(),
                            [&](const LoadedPreset& p) {
                                if (std::ranges::find(tagsToRemove, p.tag) != tagsToRemove.end()) {
                                    std::filesystem::remove(p.filePath); 
                                    return true; 
                                }
                                return false;
                            }
                        ),
                        Options->loadedPresets->end()
                    );
                    
                    m_fields->m_NodeBin.clear();

                    reloadPresets();
                }
            
            });
        }

        
        

    }

    void toggleDisableSelectMode(CCObject* sender) {
        

        if (!m_fields->m_disableMode) { 
            //enter toggle mode
            for (const auto& [tag, enabled] : m_fields->m_nodeState) {
                log::debug("Tag {} -> {}", tag, enabled ? "enabled" : "disabled");
            }

            updateTopText("Toggle Presets");
            m_fields->m_selectionMode = SelectionMode::SELECT_GRAB;

            for (const auto& [tag, enabled] : m_fields->m_nodeState) {
                auto node = m_fields->m_ScrollingButtonMenu->getChildByTag(tag);
                if (!node) continue;

                auto button = static_cast<CCMenuItemSpriteExtra*>(node);
                if (!enabled) {
                    static_cast<NineSlice*>(button->getChildByIndex(0))->setColor({129, 129, 129});
                } else {
                    static_cast<NineSlice*>(button->getChildByIndex(0))->setColor({255, 255, 255});
                }
            }

            m_fields->m_disableMode = true;

        } else { 
            //exit toggle mode

            for (const auto& [tag, enabled] : m_fields->m_nodeState) {
                log::debug("Tag {} -> {}", tag, enabled ? "enabled" : "disabled");
            }



            updateTopText("Select Preset");
            m_fields->m_selectionMode = SelectionMode::SELECT_NORMAL;

            
            

            matjson::Value disabledJson;
            std::vector<std::string> disabledNames;
            for (const auto& [tag, enabled] : m_fields->m_nodeState) {
                if (!enabled) {
                    
                    auto it = std::ranges::find_if(*Options->loadedPresets, [&](const LoadedPreset& p) { return p.tag == tag; });
                    if (it != Options->loadedPresets->end()) {
                        disabledNames.push_back(it->preset.name); 
                        continue;
                    }

                    auto dit = std::ranges::find_if(*Options->disabledPresets, [&](const LoadedPreset& p) { return p.tag == tag; });
                    if (dit != Options->disabledPresets->end()) {
                        disabledNames.push_back(dit->preset.name); 
                    }
                }
            }

            disabledJson["disabled"] = disabledNames;

            auto path = Mod::get()->getSaveDir() / "disabled.json";
            std::ofstream file(path);
            file << disabledJson.dump(4);
            file.close();

            log::debug("saved {} disabled presets to {}", disabledNames.size(), path);

            m_fields->m_disableMode = false;
        }
        
        reloadPresets();

    }


    void toggleDeleteSelectMode(CCObject* sender) {
        if (!m_fields->m_disableMode) {
            updateTopText("Delete Presets");
            m_fields->m_selectionMode = SelectionMode::SELECT_DELETE;

            m_fields->m_PresetSettingsMenu->getChildByID("SMdelete-preset-button")->setVisible(false);
            m_fields->m_PresetSettingsMenu->getChildByID("SMnormal-preset-button")->setVisible(true);
            m_fields->m_PresetSettingsMenu->getChildByID("confirm-delete-preset-button")->setVisible(true);
            m_fields->m_PresetSettingsMenu->updateLayout();
        }

    }

    void toggleNormalSelectMode(CCObject* sender) {
        updateTopText("Select Preset");
        m_fields->m_selectionMode = SelectionMode::SELECT_NORMAL;

        m_fields->m_PresetSettingsMenu->getChildByID("SMdelete-preset-button")->setVisible(true);
        m_fields->m_PresetSettingsMenu->getChildByID("SMnormal-preset-button")->setVisible(false);  
        m_fields->m_PresetSettingsMenu->getChildByID("confirm-delete-preset-button")->setVisible(false);
        m_fields->m_PresetSettingsMenu->updateLayout();

        for (auto item : m_fields->m_NodeBin) {
            static_cast<NineSlice*>(item->getChildByIndex(0))->setColor({255, 255, 255});
        }
        m_fields->m_NodeBin.clear();
    }

    void onSelectPremade(CCObject* sender) {
        auto object = static_cast<CCMenuItemSpriteExtra*>(sender);
        switch (m_fields->m_selectionMode) {
            case SelectionMode::SELECT_NORMAL:
                
               
                SelectPremadeLayer::onSelectPremade(sender);
                break;

            case SelectionMode::SELECT_DELETE: 
            {
                
                std::vector<CCMenuItemSpriteExtra*>::iterator it = std::find(m_fields->m_NodeBin.begin(), m_fields->m_NodeBin.end(), object);

                if (it != m_fields->m_NodeBin.end()) {
                    m_fields->m_NodeBin.erase(it);  
                    static_cast<NineSlice*>(object->getChildByIndex(0))->setColor({ 255, 255, 255 });
                    
                } else {
                    m_fields->m_NodeBin.push_back(object);
                    static_cast<NineSlice*>(object->getChildByIndex(0))->setColor({ 129, 129, 129 });
                }

                break;
            }
                
  
            case SelectionMode::SELECT_GRAB:

                if (m_fields->m_nodeState.find(object->getTag()) != m_fields->m_nodeState.end()) {

                    m_fields->m_nodeState[object->getTag()] = !m_fields->m_nodeState[object->getTag()];
                    static_cast<NineSlice*>(object->getChildByIndex(0))->setColor(m_fields->m_nodeState[object->getTag()] ? ccColor3B{255,255,255} : ccColor3B{129,129,129});
                }
                break;
        }  
    }

    CCMenuItemSpriteExtra* createPresetButton(AdvancedFollowPreset preset, int tag) {
        std::string buttonPath = getIconForIndex(preset.tabColor);
        
        auto btnSprite = NineSlice::create(buttonPath.c_str());
        btnSprite->setContentSize({40, 40});

        auto btnLabel = CCLabelBMFont::create(preset.name.c_str(), "goldFont.fnt");

       
        
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

        auto button = CCMenuItemSpriteExtra::create(
            btnSprite,
            this,
            menu_selector(MySelectPremadeLayer::onSelectPreset)
        );
        
        button->setTag(tag);
        button->setID(fmt::format("custom-entry-{}-{}"_spr, preset.name, tag));
       
        return button;

    }

    void onSelectPreset(CCObject* sender) {
        SelectPremadeLayer::onSelectPremade(sender);
    }
 
};



bool createPresetPopup::init() {
    if (!CCLayer::init()) return false;

    CCTouchDispatcher::get()->registerForcePrio(this, 2);
    auto winSize = CCDirector::get()->getWinSize();
    this->setID("create-preset-alert"_spr);  
    
    this->setZOrder(100);
    auto fadeLayer = CCSprite::create("square.png");
    fadeLayer->setScaleX(winSize.width / 2);
    fadeLayer->setScaleY(winSize.height / 2);
    fadeLayer->setColor({0, 0, 0});
    fadeLayer->setOpacity(0);
    fadeLayer->setZOrder(1);
    fadeLayer->setID("fade-node");

    auto bgFadeIn = CCFadeTo::create(0.1f, 150);
    auto bgSeq = CCSequence::create(
        bgFadeIn,
        nullptr
    );

    fadeLayer->runAction(bgSeq);
    this->addChildAtPosition(fadeLayer, Anchor::Center);

    auto popupBase = NineSlice::create("GJ_square01.png");
    popupBase->setID("popup-base");
    popupBase->setContentSize({380, 260});
    popupBase->setScale(0);

    auto ppScaleIn = CCScaleTo::create(0.5f, 1);
    auto eOut = CCEaseElasticOut::create(ppScaleIn, 0.6f);

    auto ppSeq = CCSequence::create(
        eOut,
        nullptr
    );

    popupBase->runAction(ppSeq);
    popupBase->setZOrder(5);

    this->addChildAtPosition(popupBase, Anchor::Center);

    PopupMenuMain = CCMenu::create();
    PopupMenuMain->setZOrder(100);
    PopupMenuMain->setID("exit-button-menu");
    
    

    auto exitButtonComp = CCNode::create();
    auto exitSpriteBase = CCSprite::createWithSpriteFrameName("geode.loader/baseCircle_BigAlt_Green.png");
    auto exitSpriteIcon = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
    
    auto center = ccp(
        exitSpriteBase->getContentSize().width / 2,
        exitSpriteBase->getContentSize().height / 2
    );

    popupBase->addChildAtPosition(PopupMenuMain, Anchor::Center);


    exitSpriteBase->setPosition(center);
    exitSpriteIcon->setPosition(center);
    exitSpriteIcon->setScale(1.15f);

    exitButtonComp->addChild(exitSpriteBase);
    exitButtonComp->addChild(exitSpriteIcon);
    
    exitButtonComp->setScale(0.85f);

    auto exitBtn = CCMenuItemExt::createSpriteExtra(exitButtonComp, [this](auto btn) {
        this->onBack(btn);
    });
    exitBtn->setContentSize(ccp(
        exitSpriteBase->getContentWidth() * 0.85f,
        exitSpriteBase->getContentHeight() * 0.85f
    ));
    exitBtn->setAnchorPoint((ccp(0.5f, 0.5f)));
    PopupMenuMain->setContentWidth(popupBase->getContentWidth() + (exitSpriteBase->getContentWidth() / 2));
    PopupMenuMain->setContentHeight(popupBase->getContentHeight() + (exitSpriteBase->getContentHeight() / 2));
    

    PopupMenuMain->addChildAtPosition(exitBtn, Anchor::TopLeft, ccp(15, -15));
    
    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);

    return true;
}


void createPresetPopup::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(this, -500, true);
}

createPresetPopup::~createPresetPopup() {
    CCTouchDispatcher::get()->unregisterForcePrio(this);
}



void createPresetPopup::onBack(CCObject*) {   
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParent();
}

void createPresetPopup::keyBackClicked() {
    this->onBack(nullptr);
}

createPresetPopup* createPresetPopup::create() {
    auto ret = new createPresetPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
};