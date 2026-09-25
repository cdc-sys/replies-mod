#include "NotificationCell.hpp"
#include "ReplyCell.hpp"
#include "ReplyPunishUserLayer.hpp"

bool NotificationCell::init(){
    if (!CCNode::init()) return false;

    this->setContentSize({335.f,26.f});

    if (!m_notif.description.empty() || m_notif.type == RepliesNotificationType::Punishment) this->setContentSize({335.f,36.f});

    if (!m_notif.read) m_bgColor = Highlighted;

    if (m_notif.type == RepliesNotificationType::Reply) this->setContentSize({335.f,56.f});

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

    if (!m_notif.read) {
        auto newLabel = CCLabelBMFont::create("New!","goldFont.fnt");
        newLabel->setAlignment(cocos2d::kCCTextAlignmentRight);
        newLabel->setAnchorPoint({1,1});
        newLabel->setPosition({this->getContentWidth()-5.f,this->getContentHeight()-2.f});
        newLabel->setScale(0.35f);
        this->addChild(newLabel);
    }

    if (m_notif.type == RepliesNotificationType::Regular) {
        float descOffset = 0.f;

        if (!m_notif.description.empty()) descOffset = 7.f;

        std::string iconTexture = "GJ_infoIcon_001.png";
        if (m_notif.icon_type == RepliesNotificationIcon::Check) iconTexture = "GJ_completesIcon_001.png";
        if (m_notif.icon_type == RepliesNotificationIcon::Cross) iconTexture = "GJ_deleteIcon_001.png";

        auto icon = CCSprite::createWithSpriteFrameName(iconTexture.c_str());
        icon->setPosition({this->getContentWidth()/16,this->getContentHeight()/2});
        icon->setScale(15.f/icon->getContentWidth());
        this->addChild(icon);

        auto titleLabel = CCLabelBMFont::create(m_notif.title.c_str(),"bigFont.fnt");
        titleLabel->setAlignment(kCCTextAlignmentLeft);
        titleLabel->limitLabelWidth(267.f,0.45f,0.01f);
        titleLabel->setAnchorPoint({0,0.5});
        titleLabel->setPosition({this->getContentWidth()/16+15.f,this->getContentHeight()/2+descOffset});
        this->addChild(titleLabel);

        if (!m_notif.description.empty()) {
            auto descLabel = CCLabelBMFont::create(m_notif.description.c_str(),"chatFont.fnt");
            descLabel->setAlignment(kCCTextAlignmentLeft);
            descLabel->limitLabelWidth(267.f,0.6f,0.01f);
            descLabel->setAnchorPoint({0,0.5});
            descLabel->setPosition({this->getContentWidth()/16+15.f,this->getContentHeight()/2-descOffset});
            this->addChild(descLabel);
        }
    } else if (m_notif.type == RepliesNotificationType::Reply) {
        auto icon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        icon->setPosition({this->getContentWidth()/16,this->getContentHeight()-10.f});
        icon->setScale(12.5f/icon->getContentWidth());
        this->addChild(icon);

        auto titleLabel = CCLabelBMFont::create(m_notif.title.c_str(),"bigFont.fnt");
        titleLabel->setAlignment(kCCTextAlignmentLeft);
        titleLabel->limitLabelWidth(267.f,0.40f,0.01f);
        titleLabel->setAnchorPoint({0,0.5});
        titleLabel->setPosition({this->getContentWidth()/16+15.f,this->getContentHeight()-10.f});
        this->addChild(titleLabel);
        
        auto fake_reply = Reply();
        fake_reply.needs_web_fetch = true;
        fake_reply.id = m_notif.reply_id;

        auto replyCell = ReplyCell::create(nullptr, fake_reply,Highlighted,0);
        replyCell->setPosition({this->getContentWidth()/2,3.f});
        replyCell->setAnchorPoint({0.5,0});
        replyCell->setScale(0.95f);
        this->addChild(replyCell);
    } else {
        auto icon = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
        icon->setPosition({this->getContentWidth()/16,this->getContentHeight()/2});
        icon->setScale(15.f/icon->getContentWidth());
        this->addChild(icon);

        auto titleLabel = geode::Label::create("bigFont.fnt");
        titleLabel->setRichText(fmt::format("You have received a <cg>{}</c> for breaking <co>Replies</c> rules!",m_notif.punishment_type));
        titleLabel->setAlignment(geode::Label::Alignment::Left);
        titleLabel->setLimitLabelWidth(267.f,0.45f,0.01f);
        titleLabel->setAnchorPoint({0,0.5});
        titleLabel->setPosition({this->getContentWidth()/16+15.f,this->getContentHeight()/2+7.f});
        this->addChild(titleLabel);

        auto expiryLabel = geode::Label::create("bigFont.fnt");
        expiryLabel->setRichText(fmt::format("It expires <cj>{}</c>.",toAgoString(m_notif.punishment_expiry)));
        if (m_notif.punishment_expiry == -1) expiryLabel->setRichText("It is <cr>permanent</c>");
        expiryLabel->setAlignment(geode::Label::Alignment::Left);
        expiryLabel->setLimitLabelWidth(267.f,0.35f,0.01f);
        expiryLabel->setAnchorPoint({0,0.5});
        expiryLabel->setPosition({this->getContentWidth()/16+15.f,this->getContentHeight()/2-7.f});
        this->addChild(expiryLabel);
    }

    std::string timestamp = toAgoString(m_notif.timestamp/1000);
    auto dateLabel = CCLabelBMFont::create(timestamp.c_str(),"chatFont.fnt");
    dateLabel->setAlignment(kCCTextAlignmentRight);
    dateLabel->setAnchorPoint({1,0});
    dateLabel->setPosition({this->getContentWidth()-5.f,2.f});
    if (m_notif.type == RepliesNotificationType::Reply) dateLabel->setVisible(false);
    dateLabel->setScale(0.45f);
    dateLabel->setColor({0,0,0});
    dateLabel->setOpacity(125);
    this->addChild(dateLabel);

    return true;
}

NotificationCell* NotificationCell::create(RepliesBasePaginatedLayer* rl,RepliesNotification notif,ReplyBackgroundColor bgColor){
    auto ret = new NotificationCell();
    ret->m_rl = rl;
    ret->m_notif = notif;
    ret->m_bgColor = bgColor;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}