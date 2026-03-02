#pragma once
#include <Geode/Geode.hpp>
#include "../util/Utilities.hpp"

using namespace geode::prelude;

class PresetPopup : public cocos2d::CCLayer {

    public:
    static PresetPopup* create(CCMenu* menu);
    cocos2d::CCMenu* PopupMenuMain;
    std::vector<CCMenuItemSpriteExtra*> iconButtons;


    protected:
    bool init() override;
    void keyBackClicked() override;
    void onBack(CCObject*);
    CCMenu* menu;
   
    void registerWithTouchDispatcher() override;
    ~PresetPopup();

};