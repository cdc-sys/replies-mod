#include "PunishmentCell.hpp"
#include "ReplyCell.hpp"
#include "ReplyPunishUserLayer.hpp"
#include <Geode/ui/Label.hpp>

bool PunishmentCell::init(){
    if (!CCNode::init()) return false;

    this->setContentSize({335.f,46.f});

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

    if (m_punishment.cancelled) m_bgColor = ReplyBackgroundColor::ReportIgnored;

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
        case ReplyBackgroundColor::ReportIgnored: {
            bg->setColor({ 150,0,0 });
            break;
        }
    }
    bg->setOpacity(50);
    bg->setContentSize(this->getContentSize());
    this->addChild(bg);

    float descOffset = 3.f;

    std::string iconTexture = "GJ_infoIcon_001.png";

    /*auto icon = CCSprite::createWithSpriteFrameName(iconTexture.c_str());
    icon->setPosition({this->getContentWidth()/16,this->getContentHeight()/2});
    icon->setScale(15.f/icon->getContentWidth());
    this->addChild(icon);*/

    auto playerIcon = SimplePlayer::create(m_punishment.icon.type);
    playerIcon->setPosition({5.f,this->getContentHeight()/2});
    playerIcon->setScale(0.8f);
    playerIcon->updatePlayerFrame(m_punishment.icon.id, (IconType)m_punishment.icon.type);
    playerIcon->setColors(GameManager::get()->colorForIdx(m_punishment.icon.primaryColor), GameManager::get()->colorForIdx(m_punishment.icon.secondaryColor));
    if (m_punishment.icon.glow) playerIcon->setGlowOutline(GameManager::get()->colorForIdx(m_punishment.icon.glowColor));
    playerIcon->setAnchorPoint({0.f,0.5f});
    playerIcon->setContentSize({30.f,30.f});
    for (auto child : CCArrayExt<CCNode*>(playerIcon->getChildren())){
        child->ignoreAnchorPointForPosition(true);
    }
    this->addChild(playerIcon);

    auto titleLabel = CCLabelBMFont::create(m_punishment.type.c_str(),"bigFont.fnt");
    titleLabel->setAlignment(kCCTextAlignmentLeft);
    titleLabel->limitLabelWidth(45.f,0.45f,0.01f);
    titleLabel->setAnchorPoint({0,1});
    titleLabel->setPosition({this->getContentWidth()/16+15.f,this->getContentHeight()-descOffset});
    titleLabel->setColor({70,200,255});
    if (m_punishment.type == "ban") titleLabel->setColor({255,50,50});
    this->addChild(titleLabel);

    auto typeLabel = CCLabelBMFont::create(m_punishment.account_name.c_str(),"bigFont.fnt");
    typeLabel->setAlignment(kCCTextAlignmentLeft);
    typeLabel->limitLabelWidth(100.f,0.45f,0.01f);
    typeLabel->setAnchorPoint({0,1});
    typeLabel->setPosition({titleLabel->getPositionX()+titleLabel->getScaledContentWidth()+5.f,this->getContentHeight()-descOffset});
    this->addChild(typeLabel);

    auto activeTypeLabel = CCLabelBMFont::create("Active","bigFont.fnt");
    activeTypeLabel->setAlignment(kCCTextAlignmentLeft);
    activeTypeLabel->limitLabelWidth(25.f,0.25f,0.01f);
    activeTypeLabel->setAnchorPoint({0,0.5});
    activeTypeLabel->setColor({ 255, 100,0 });
    if (!m_punishment.active) {
        if (m_punishment.cancelled) {
            activeTypeLabel->setColor({ 255,255,0 });
            activeTypeLabel->setString("Lifted");
        } else {
            activeTypeLabel->setColor({ 50,255,50 });
            activeTypeLabel->setString("Expired");
        }
    }
    activeTypeLabel->setPosition({typeLabel->getPositionX()+typeLabel->getScaledContentWidth()+5.f,this->getContentHeight()-(typeLabel->getScaledContentHeight()/2)-descOffset});
    this->addChild(activeTypeLabel);

    auto descLabel = geode::Label::create("chatFont.fnt");
    if (m_punishment.length != -1) {
        if (m_punishment.active) {
            descLabel->setRichText(fmt::format("Reason: <cy>{}</c> \nLength <cj>{}</c> <cg>(Expires {})</c>",m_punishment.reason,toLengthString(m_punishment.length),toAgoString(m_punishment.timestamp/1000+m_punishment.length)));
        } else if (m_punishment.cancelled) {
            descLabel->setRichText(fmt::format("Reason: <cy>{}</c> \nLength <cj>{}</c> <cy>(Lifted {})</c>",m_punishment.reason,toLengthString(m_punishment.length),toAgoString(m_punishment.cancelled_at/1000)));
        } else {
            descLabel->setRichText(fmt::format("Reason: <cy>{}</c> \nLength <cj>{}</c> <cg>(Expired {})</c>",m_punishment.reason,toLengthString(m_punishment.length),toAgoString(m_punishment.timestamp/1000+m_punishment.length)));
        }
    } else {
        descLabel->setRichText(fmt::format("Reason: <cy>{}</c> \nLength <cr>Permanent</c>",m_punishment.reason));
    }
    descLabel->setAlignment(geode::Label::Alignment::Left);
    descLabel->setLimitLabelWidth(267.f,0.6f,0.01f);
    descLabel->setAnchorPoint({0,1});
    descLabel->setPosition({this->getContentWidth()/16+15.f,titleLabel->getPositionY()-titleLabel->getScaledContentHeight()-descOffset});
    this->addChild(descLabel);

    if (m_punishment.cancelled) {
        auto cancelDateLabel = CCLabelBMFont::create(fmt::format("lifted by {}",m_punishment.canceller_name).c_str(),"chatFont.fnt");
        cancelDateLabel->setAlignment(kCCTextAlignmentRight);
        cancelDateLabel->setAnchorPoint({1,1});
        cancelDateLabel->setPosition({this->getContentWidth()-5.f,this->getContentHeight()-2.f});
        cancelDateLabel->setScale(0.45f);
        cancelDateLabel->setColor({0,0,0});
        cancelDateLabel->setOpacity(125);
        this->addChild(cancelDateLabel);
    } else {
        auto menu = CCMenu::create();
        menu->setAnchorPoint({1,1});
        menu->setPosition({this->getContentWidth()-5.f,this->getContentHeight()-5.f});
        menu->setScale(0.6f);

        auto layout = RowLayout::create();
        layout->setAxisAlignment(AxisAlignment::End);
        layout->setGap(2.f);
        menu->setLayout(layout);

        auto btnSpr = CCSprite::createWithSpriteFrameName("accountBtn_blocked_001.png");
        auto btn = CCMenuItemExt::createSpriteExtra(btnSpr,[this](auto me){
            if (this->m_punishment.active) {
                auto layer = ReplyPunishUserLayer::createCancel(m_punishment.type,m_punishment.account_name);
                layer->show();
            } else {
                auto layer = ReplyPunishUserLayer::create(m_punishment.type,m_punishment.account_name,m_punishment.reason);
                layer->show();
            }
        });

        auto profileBtnSpr = CCSprite::createWithSpriteFrameName("accountBtn_friends_001.png");
        auto profileBtn = CCMenuItemExt::createSpriteExtra(profileBtnSpr,[this](auto me){
            ProfilePage::create(m_punishment.account_id,GJAccountManager::get()->m_accountID==m_punishment.account_id)->show();
        });

        if (m_punishment.active) menu->addChild(btn);
        menu->addChild(profileBtn);

        menu->updateLayout();
        this->addChild(menu);
    }

    std::string timestamp = toAgoString(m_punishment.timestamp/1000);
    auto dateLabel = CCLabelBMFont::create(fmt::format("by {} {}",m_punishment.author_name,timestamp).c_str(),"chatFont.fnt");
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