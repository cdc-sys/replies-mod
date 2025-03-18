#include "ReplyCell.hpp"
#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Structs.hpp"

bool ReplyCell::init(){
    if (!CCNode::init()) return false;
    this->setContentSize({335.f-36*m_replyLevel,36.f});

    auto line = CCLayerColor::create();
    line->setColor({0,0,0});
    line->setContentSize({this->getContentSize().width,.425f});
    line->setOpacity(125);
    this->addChild(line);

    auto bg2 = CCLayerColor::create();
    bg2->setColor({0,0,0});
    bg2->setOpacity(120);
    bg2->setContentSize({36.f*m_replyLevel,this->getContentSize().height});
    bg2->setAnchorPoint({0,0});
    bg2->setPosition({-36.f*m_replyLevel,0});
    this->addChild(bg2);
    
    auto spriteName = fmt::format("reply-{}.png"_spr,(int)this->m_spriteType+1);
    auto sprite = CCSprite::createWithSpriteFrameName(spriteName.c_str());
    sprite->setScale(36.f*m_replyLevel/sprite->getContentWidth());
    sprite->setOpacity(100);
    sprite->setAnchorPoint({0,0});
    sprite->setPosition({-36.f*m_replyLevel,0});
    this->addChild(sprite);

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

    auto contentLabel = CCLabelBMFont::create(m_reply.content.c_str(),"chatFont.fnt",200.f,kCCTextAlignmentLeft);
    contentLabel->setAnchorPoint({0,0.5});
    contentLabel->setPosition({36.f,13.f});
    contentLabel->setScale(0.65f);
    this->addChild(contentLabel);

    auto authorLabel = CCLabelBMFont::create(m_reply.author_name.c_str(),"goldFont.fnt");
    authorLabel->setAlignment(kCCTextAlignmentLeft);
    authorLabel->setAnchorPoint({0,0.5});
    authorLabel->setPosition({36.f,26.f});
    authorLabel->setScale(0.5f);
    this->addChild(authorLabel);

    return true;
}

ReplyCell* ReplyCell::create(Reply reply,ReplyBackgroundColor bgColor, int replyLevel,ReplySpriteType spriteType){
    auto ret = new ReplyCell();
    ret->m_reply = reply;
    ret->m_bgColor = bgColor;
    ret->m_replyLevel = replyLevel;
    ret->m_spriteType = spriteType;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}