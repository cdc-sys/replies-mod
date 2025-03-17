#include "ReplyLayer.hpp"
#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/cocos/actions/CCActionInterval.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/cocos.hpp"

ReplyLayer* ReplyLayer::create(std::string commentID) {
    auto ret = new ReplyLayer();
    ret->m_commentID = commentID;
    if (ret && ret->initAnchored(375, 290,commentID)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void ReplyLayer::show(){
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    CCScene::get()->addChild(this);
    this->setZOrder(CCScene::get()->getHighestChildZ()+1);
    this->m_mainLayer->setPosition({winSize.width/2,0});
    this->m_mainLayer->runAction(
        cocos2d::CCEaseElasticOut::create(CCMoveTo::create(0.75f,{winSize.width/2,winSize.height/2-20.f}), 0.6f)
    );
    this->setOpacity(0);
    this->runAction(CCFadeTo::create(0.25f,125));
}

void ReplyLayer::onClose(CCObject* sender){
    this->m_mainLayer->runAction(
        CCSequence::create(
            cocos2d::CCEaseIn::create(CCMoveTo::create(0.5f,{CCDirector::sharedDirector()->getWinSize().width/2,-this->getContentSize().height/2}), 3.f),
            cocos2d::CCCallFunc::create(this, callfunc_selector(ReplyLayer::removeFromParent)),
            nullptr
        )
    );
    this->runAction(cocos2d::CCFadeTo::create(0.5f,0));
}

bool ReplyLayer::setup(std::string const& commentID){
    this->setTitle("Replies");

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto newContentHeight = ((winSize.height-m_bgSprite->getContentHeight())/2)+100;
    m_bgSprite->setContentHeight(m_bgSprite->getContentHeight()+newContentHeight);
    m_bgSprite->setPositionY(m_bgSprite->getPositionY()-newContentHeight/2);

    auto newSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    newSprite->setRotation(-90.f);
    newSprite->setScale(.8f);
    this->m_closeBtn->setNormalImage(newSprite);
    this->m_closeBtn->updateSprite();
    this->m_closeBtn->setPosition({this->m_buttonMenu->getContentSize().width/2,this->m_buttonMenu->getContentSize().height+15.f});

    m_replyTextInput = geode::TextInput::create(300.f,"Type your reply here...","chatFont.fnt");
    m_replyTextInput->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()-_=+[]{};:'\",.<>/?\\|`~ ");
    m_replyTextInput->setPosition({160.f,30.f});
    m_replyTextInput->setAnchorPoint({0.5f,0.5f});
    this->m_mainLayer->addChild(m_replyTextInput);
    return true;
}