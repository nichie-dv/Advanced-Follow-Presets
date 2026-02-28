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

    void wrapPreset() {

    }

    void confirmPremadeClosed(SelectPremadeLayer* layer, int type) {

    }

    bool init(AdvancedFollowTriggerObject* object, CCArray* objects) {
        if (!SetupAdvFollowPopup::init(object, objects)) return false;

        if (!Options)
            Options = new PresetsOptions();



        return true;
    }

    void onClose(CCObject* sender) {
        log::info("clearing presets...");
        Options = nullptr;
        SetupAdvFollowPopup::onClose(sender);
    }

    void selectPremadeClosed(SelectPremadeLayer* layer, int type) {
         
        auto dict = static_cast<CCDictionary*>(this->m_valueToggles);
        AdvancedFollowPreset preset;
        std::vector<int> presetIDS;
        std::vector<float> presetValues;

        auto it = std::find_if(
            Options->loadedPresets->begin(),
            Options->loadedPresets->end(),
            [type](const std::pair<AdvancedFollowPreset, int>& p) {
                return p.second == type;
            }
        );

        if (it != Options->loadedPresets->end()) {
            preset = it->first;
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



#include <Geode/modify/SelectPremadeLayer.hpp>
class $modify(MySelectPremadeLayer, SelectPremadeLayer) {
    struct Fields {
        createPresetPopup* m_popup = nullptr;
        CCMenu* m_ScrollingButtonMenu = nullptr;
        int m_itemSpriteSelection = 0;
    };

    void reloadPresets() {
        if (!Options) return;

        auto presetFolderPath = Mod::get()->getSettingValue<std::filesystem::path>("presets-path");
        auto templateFolderPath = Mod::get()->getSettingValue<std::filesystem::path>("template-path");


        if (Options->loadedPresets) {
            Options->loadedPresets->clear();
        } else {
            Options->loadedPresets = new std::vector<std::pair<AdvancedFollowPreset, int>>();
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

        //load robtop presets first
        try {  
            std::ifstream file(templateFolderPath / "robtopHomingMissile.json");
            std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            matjson::Value jsonValue = matjson::parse(jsonStr).unwrap();
            AdvancedFollowPreset preset = presetFromJson(jsonValue);
            if (std::find(disabledNames.begin(), disabledNames.end(), preset.name) == disabledNames.end()) {
                Options->loadedPresets->push_back({preset, index});
                log::info("Loaded preset from {}", (templateFolderPath / "robtopHomingMissile.json").string());
                index++;
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
                Options->loadedPresets->push_back({preset, index});
                log::info("Loaded preset from {}", (templateFolderPath / "robtopHomingBall.json").string());
                index++;
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
                    Options->loadedPresets->push_back({preset, index});           
                    index++;
                    log::info("Loaded preset from {}", entry.path().string());
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
            auto preset = entry.first;
            auto tag = entry.second;

            auto button = createPresetButton(preset, tag);
            m_fields->m_ScrollingButtonMenu->addChild(button);

        }
        
    }

    

    bool init() {
        if (!SelectPremadeLayer::init()) return false;
        
        if (!Options) return false;

        //get premades and store in a vector
        //Options->loadedPresets;
        auto presetFolderPath = Mod::get()->getSettingValue<std::filesystem::path>("presets-path");
        auto templateFolderPath = Mod::get()->getSettingValue<std::filesystem::path>("template-path");
        

        if (Options->loadedPresets) {
            Options->loadedPresets->clear();
        } else {
            Options->loadedPresets = new std::vector<std::pair<AdvancedFollowPreset, int>>();
        }


        
        




        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto popupContentLayer = this->getChildByIndex(0);

        auto popupBgNode = static_cast<CCScale9Sprite*>(popupContentLayer->getChildByIndex(0));
        auto popupText = static_cast<CCLabelBMFont*>(popupContentLayer->getChildByIndex(1));
        auto presetButtonMenu = static_cast<CCMenu*>(popupContentLayer->getChildByIndex(2));
        
        popupContentLayer->setID("popup-content");
        popupText->setID("preset-text");
        popupBgNode->setID("node-bg");
        presetButtonMenu->setID("preset-options-menu");

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

        popupText->setString("Select Preset");




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
        presetButtonMenu->addChild(exportButton);

   
        



        

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
        


        m_fields->m_ScrollingButtonMenu = scrollingButtonMenu;
        reloadPresets();

    
        float buttonWidth = 45;
        float containerWidth = scrollingContentLayer->getContentWidth();

        int buttonsPerRow = containerWidth / (buttonWidth + 6);
        int totalButtons = scrollingButtonMenu->getChildrenCount();
        int rows = (totalButtons + buttonsPerRow - 1) / buttonsPerRow;

        scrollingContentLayer->addChild(scrollingButtonMenu);
        scrollingContentLayer->updateLayout();

        popupContentLayer->addChildAtPosition(popupScrollingBG, Anchor::Center, ccp(0, 7));
        popupContentLayer->addChildAtPosition(popupNodeScrollingLayer, Anchor::Center, ccp(0, 7));


        scrollingButtonMenu->updateLayout();
        
        
        scrollingContentLayer->setContentHeight(scrollingButtonMenu->getContentHeight());
        scrollingContentLayer->updateLayout();
        popupNodeScrollingLayer->scrollToTop();
        this->updateLayout();
        
        presetButtonMenu->updateLayout();

        this->setTouchMode(kCCTouchesOneByOne);
        this->setTouchPriority(-500);

        this->setKeypadEnabled(true);
        this->setTouchEnabled(true);

        return true;
    }



    void onCreateNewPreset(CCObject* sender) {
        auto popup = createPresetPopup::create();
        m_fields->m_popup = popup;
        auto menu = popup->getChildByID("popup-base");

        auto buttonMenu = CCMenu::create();
        buttonMenu->ignoreAnchorPointForPosition(false);
        buttonMenu->setLayoutOptions(AnchorLayoutOptions::create());
        buttonMenu->setID("button-menu");
        buttonMenu->setAnchorPoint({0.5f, 0});
        buttonMenu->setContentSize({430, 40});
        buttonMenu->setLayout(AxisLayout::create()
            ->setGap(2)
        );

        


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
                item->setColor({ 104, 104, 104 });
            }
            
            
            popup->iconButtons.push_back(item);
            iconMenu->addChild(item);
        }
        


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
                item->setColor({ 104, 104, 104 });
            } else {
                item->setColor({ 255, 255, 255 });
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



    void onSelectPremade(CCObject* sender) {
        SelectPremadeLayer::onSelectPremade(sender);
        
    }

    CCMenuItemSpriteExtra* createPresetButton(AdvancedFollowPreset preset, int tag) {
        std::string buttonPath = getIconForIndex(preset.tabColor);
        
        auto btnSprite = NineSlice::create(buttonPath.c_str());
        btnSprite->setContentSize({40, 40});

        auto btnLabel = CCLabelBMFont::create(preset.name.c_str(), "goldFont.fnt");

       
        
        float maxWidth = 35.0f;
        btnLabel->setWidth(maxWidth);  
        btnLabel->setAlignment(kCCTextAlignmentCenter);

        // Compute scale to fit width
        float labelWidth = btnLabel->getContentSize().width;
        float scale = 0.4f; // base scale
        if (labelWidth * scale > maxWidth) {
            scale = maxWidth / labelWidth; // shrink to fit
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
        button->setID(std::format("custom-entry-{}-{}"_spr, preset.name, tag));
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
    popupBase->setContentSize({475, 260});
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