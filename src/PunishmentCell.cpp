#include "PunishmentCell.hpp"
#include "ReplyCell.hpp"
#include "ReplyPunishUserLayer.hpp"

bool PunishmentCell::init(){
    if (!CCNode::init()) return false;

    this->setContentSize({335.f,26.f});

    this->setContentSize({335.f,36.f});

    auto line = CCLayerColor::create();
    line->setColor({0,0,0});
    line->setContentSize({this->getContentSize().width,.625f});
    line->setOpacity(125);
    this->addChild(line);

    auto line2 = CCLayerColor::create();
    line2->setColor({0,0,0});
    line2->setContentSize({.425f,this->getContentHeight()});
    line2->setOpacity(125);
    this->addChild(line2);

    auto bg = CCLayerColor::create();
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

    float descOffset = 7.f;

    std::string iconTexture = "GJ_infoIcon_001.png";

    auto icon = CCSprite::createWithSpriteFrameName(iconTexture.c_str());
    icon->setPosition({this->getContentWidth()/16,this->getContentHeight()/2});
    icon->setScale(15.f/icon->getContentWidth());
    this->addChild(icon);

    auto titleLabel = CCLabelBMFont::create(m_punishment.account_name.c_str(),"bigFont.fnt");
    titleLabel->setAlignment(kCCTextAlignmentLeft);
    titleLabel->limitLabelWidth(267.f,0.45f,0.01f);
    titleLabel->setAnchorPoint({0,0.5});
    titleLabel->setPosition({this->getContentWidth()/16+15.f,this->getContentHeight()/2+descOffset});
    this->addChild(titleLabel);

    auto descLabel = CCLabelBMFont::create(m_punishment.reason.c_str(),"chatFont.fnt");
    descLabel->setAlignment(kCCTextAlignmentLeft);
    descLabel->limitLabelWidth(267.f,0.6f,0.01f);
    descLabel->setAnchorPoint({0,0.5});
    descLabel->setPosition({this->getContentWidth()/16+15.f,this->getContentHeight()/2-descOffset});
    this->addChild(descLabel);

    std::string timestamp = toAgoString(m_punishment.timestamp/1000);
    auto dateLabel = CCLabelBMFont::create(timestamp.c_str(),"chatFont.fnt");
    dateLabel->setAlignment(kCCTextAlignmentRight);
    dateLabel->setAnchorPoint({1,0});
    dateLabel->setPosition({this->getContentWidth()-5.f,2.f});
    dateLabel->setScale(0.45f);
    dateLabel->setColor({0,0,0});
    dateLabel->setOpacity(125);
    this->addChild(dateLabel);

    return true;
}

PunishmentCell* PunishmentCell::create(RepliesBasePaginatedLayer* rl,Punishment punishment,ReplyBackgroundColor bgColor){
    auto ret = new PunishmentCell();
    ret->m_rl = rl;
    ret->m_punishment = punishment;
    ret->m_bgColor = bgColor;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}