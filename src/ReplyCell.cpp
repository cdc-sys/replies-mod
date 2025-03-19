#include "ReplyCell.hpp"
#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Structs.hpp"

void ReplyCell::onReply(CCObject* sender){
    auto rl = ReplyLayer::create(m_reply);
    rl->show();
}

bool ReplyCell::init(){
    if (!CCNode::init()) return false;
    float offset = 36*m_replyLevel;
    this->setContentSize({335.f-offset,36.f});

    auto line = CCLayerColor::create();
    line->setColor({0,0,0});
    line->setContentSize({this->getContentSize().width,.425f});
    line->setOpacity(125);
    this->addChild(line);

    auto bg2 = CCLayerColor::create();
    bg2->setColor({0,0,0});
    bg2->setOpacity(120);
    bg2->setContentSize({offset,this->getContentSize().height});
    bg2->setAnchorPoint({0,0});
    bg2->setPosition({-offset,0});
    this->addChild(bg2);
    
    for (int i = 0; i<m_replyLevel-m_skipLines; i++){

        auto spriteName = fmt::format("reply-{}.png"_spr,(i>0 ? 1 : (int)this->m_spriteType+1));
        auto sprite = CCSprite::createWithSpriteFrameName(spriteName.c_str());
        sprite->setScale(36/sprite->getContentWidth());
        sprite->setOpacity(50);
        sprite->setAnchorPoint({0,0});
        sprite->setPosition({-36.f*(i+1),0});
        this->addChild(sprite);
    }

    auto bg = CCLayerColor::create();
    geode::log::info("{}",(int)m_bgColor);
    switch (m_bgColor){
        case ReplyBackgroundColor::Highlighted: {
            bg->setColor({ 255, 208, 0 });
            break;
        }
        case ReplyBackgroundColor::Darker: {
            bg->setColor({0,0,0});
            break;
        }
        case ReplyBackgroundColor::Regular: {
            bg->setVisible(false);
            break;
        }
    }
    bg->setOpacity(50);
    bg->setContentSize(this->getContentSize());
    this->addChild(bg);

    auto playerIcon = SimplePlayer::create(m_reply.icon.type);
    playerIcon->setPosition({5.f,4.f});
    playerIcon->setScale(0.9f);
    playerIcon->setColors(GameManager::get()->colorForIdx(m_reply.icon.primaryColor), GameManager::get()->colorForIdx(m_reply.icon.secondaryColor));
    playerIcon->updatePlayerFrame(m_reply.icon.id, (IconType)m_reply.icon.type);
    if (m_reply.icon.glow) playerIcon->setGlowOutline(GameManager::get()->colorForIdx(m_reply.icon.glowColor));
    for (auto child : CCArrayExt<CCNode*>(playerIcon->getChildren())){
        child->ignoreAnchorPointForPosition(true);
    }
    this->addChild(playerIcon);

    auto authorLabel = CCLabelBMFont::create(m_reply.author_name.c_str(),"goldFont.fnt");
    authorLabel->setAlignment(kCCTextAlignmentLeft);
    authorLabel->setAnchorPoint({0,0.5});
    authorLabel->setPosition({36.f,26.f});
    authorLabel->setScale(0.5f);
    this->addChild(authorLabel);

    auto contentLabel = CCLabelBMFont::create(m_reply.content.c_str(),"chatFont.fnt",200.f,kCCTextAlignmentLeft);
    contentLabel->setAnchorPoint({0,0.5});
    contentLabel->setPosition({36.f,13.f});
    contentLabel->setScale(0.65f);
    this->addChild(contentLabel);
    
    auto dateLabel = CCLabelBMFont::create(toAgoString(m_reply.timestamp/1000).c_str(),"chatFont.fnt");
    dateLabel->setAlignment(kCCTextAlignmentRight);
    dateLabel->setAnchorPoint({1,0});
    dateLabel->setPosition({this->getContentWidth()-5.f,2.f});
    dateLabel->setScale(0.45f);
    dateLabel->setColor({0,0,0});
    dateLabel->setOpacity(125);
    this->addChild(dateLabel);

    auto replySpr = CCSprite::createWithSpriteFrameName("GJ_undoBtn_001.png");
    replySpr->setScale(.45f);
    auto replyBtn = CCMenuItemSpriteExtra::create(replySpr,this,menu_selector(ReplyCell::onReply));
    auto replyMenu = CCMenu::create();
    replyMenu->addChild(replyBtn);
    replyMenu->setPosition({0,0});
    replyBtn->setPosition({this->getContentWidth()-25.f,this->getContentHeight()/2});
    this->addChild(replyMenu);

    return true;
}

ReplyCell* ReplyCell::create(Reply reply,ReplyBackgroundColor bgColor, int replyLevel,ReplySpriteType spriteType, int skipLines){
    auto ret = new ReplyCell();
    ret->m_reply = reply;
    ret->m_bgColor = bgColor;
    ret->m_replyLevel = replyLevel;
    ret->m_spriteType = spriteType;
    ret->m_skipLines = skipLines;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}