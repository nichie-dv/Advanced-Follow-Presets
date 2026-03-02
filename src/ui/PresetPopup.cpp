#include "PresetPopup.hpp"
#include <Geode/Geode.hpp>



bool PresetPopup::init() {
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


void PresetPopup::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(this, -500, true);
}

PresetPopup::~PresetPopup() {
    CCTouchDispatcher::get()->unregisterForcePrio(this);
}



void PresetPopup::onBack(CCObject*) {   
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->menu->setTouchEnabled(true);
    this->menu->setKeypadEnabled(true);
    this->removeFromParent();
}

void PresetPopup::keyBackClicked() {
    this->onBack(nullptr);
}

PresetPopup* PresetPopup::create(CCMenu* menu) {
    auto ret = new PresetPopup();
    ret->menu = menu;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
};