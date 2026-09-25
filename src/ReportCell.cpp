#include "ReportCell.hpp"
#include "ReplyCell.hpp"
#include "ReplyPunishUserLayer.hpp"
#include "ReplySendNoticeLayer.hpp"

void ReportCell::doClose(bool accepted){
    auto req = web::WebRequest();
    auto url = fmt::format("{}/moderation/report/{}/",SERVER_URL,m_report.id);
    req.header("Authorization", Mod::get()->getSavedValue<std::string>("token"));
    req.header("mod-version",MOD_VERSION_HEADER);
    req.param("accepted",(accepted ? "true" : "false"));
    this->m_webListener.spawn(req.send("DELETE", url),[this](web::WebResponse res){
        if (!res.ok()) {
            auto json = res.json().unwrapOrDefault();
            if (json.contains("err")){
                auto errorText = json["err"]["text"].asString().unwrapOr("Unknown");
                auto notif = geode::Notification::create(fmt::format("Failed to close: {}",errorText),NotificationIcon::Error);
                notif->show();
            }
        } else {
            if (m_rl) {
                m_rl->m_page = std::ceil((m_rl->m_totalReplies-2)/10)+1;
                m_rl->loadReplies(true);
            }
            auto notif = geode::Notification::create("Report closed!",NotificationIcon::Success);
            notif->show();
        }
    });
}

bool ReportCell::init(){
    if (!CCNode::init()) return false;

    this->setContentSize({335.f,97.f});

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

    auto authorMenu = CCMenu::create();

    auto authorLabel = CCLabelBMFont::create(m_report.author_name.c_str(),"goldFont.fnt");
    authorLabel->setAlignment(kCCTextAlignmentLeft);
    authorLabel->setScale(0.5f);

    auto reportedLabel = CCLabelBMFont::create(m_report.account_name.c_str(),"goldFont.fnt");
    reportedLabel->setAlignment(kCCTextAlignmentLeft);
    reportedLabel->setScale(0.5f);

    auto clickableAuthor = CCMenuItemExt::createSpriteExtra(authorLabel, [this](auto) {
        auto profile = ProfilePage::create(m_report.author_id,m_report.author_id == GJAccountManager::get()->m_accountID);
        profile->show();
    });
    clickableAuthor->setAnchorPoint({0,0.5});
    clickableAuthor->setSizeMult(1.1f);

    authorMenu->addChild(clickableAuthor);

    auto actionLabel = CCLabelBMFont::create("has reported","bigFont.fnt");
    actionLabel->setScale(0.3f);

    authorMenu->addChild(actionLabel);

    auto clickableReported = CCMenuItemExt::createSpriteExtra(reportedLabel, [this](auto) {
        auto profile = ProfilePage::create(m_report.account_id,m_report.account_id == GJAccountManager::get()->m_accountID);
        profile->show();
    });
    clickableReported->setAnchorPoint({0,0.5});
    clickableReported->setSizeMult(1.1f);

    authorMenu->addChild(clickableReported);

    auto reasonLabel = CCLabelBMFont::create(fmt::format("for {}",m_report.reason).c_str(),"bigFont.fnt");
    reasonLabel->setScale(0.3f);

    authorMenu->addChild(reasonLabel);

    auto layout = AxisLayout::create(Axis::Row);
    layout->setGap(4.f);
    layout->setAxisAlignment(AxisAlignment::Start);
    layout->setAutoScale(false);

    authorMenu->setLayout(layout);
    authorMenu->updateLayout();

    authorMenu->setPosition({5.f,this->getContentHeight()-10.f});
    authorMenu->setAnchorPoint({0,0.5});
    authorMenu->setContentWidth(320.f);
    authorMenu->setScale(0.84f);
    this->addChild(authorMenu);

    auto noteLabel = CCLabelBMFont::create(m_report.note.c_str(),"chatFont.fnt");
    noteLabel->setScale(0.6f);
    noteLabel->setPosition({5.f,this->getContentHeight()-25.f});
    noteLabel->setAnchorPoint({0,0.5});
    noteLabel->setAlignment(kCCTextAlignmentLeft);
    this->addChild(noteLabel);

    auto reply = Reply();
    reply.id = m_report.reply_id;
    reply.needs_web_fetch = true;

    auto replyCell = ReplyCell::create(nullptr,reply,Highlighted,0,Line);
    replyCell->setPosition({this->getContentWidth()/2,25.f});
    replyCell->setAnchorPoint({0.5,0});
    replyCell->setScale(0.95f);
    this->addChild(replyCell);

    std::string timestamp = toAgoString(m_report.timestamp/1000);
    auto dateLabel = CCLabelBMFont::create(timestamp.c_str(),"chatFont.fnt");
    dateLabel->setAlignment(kCCTextAlignmentRight);
    dateLabel->setAnchorPoint({1,0});
    dateLabel->setPosition({this->getContentWidth()-5.f,2.f});
    dateLabel->setScale(0.45f);
    dateLabel->setColor({0,0,0});
    dateLabel->setOpacity(125);
    this->addChild(dateLabel);

    auto actionsMenu = CCMenu::create();

    auto sendNoticeSprite = ButtonSprite::create("Send Notice");
    auto muteUserSprite = ButtonSprite::create("Mute User");
    auto banUserSprite = ButtonSprite::create("Ban User");

    auto sendNoticeBtn = CCMenuItemExt::createSpriteExtra(sendNoticeSprite, [this](auto){
        ReplySendNoticeLayer::create(this->m_report.account_name)->show();
    });
    auto muteUserBtn = CCMenuItemExt::createSpriteExtra(muteUserSprite, [this](auto){
        auto rpul = ReplyPunishUserLayer::create("mute",this->m_report.account_name,this->m_report.reason);
        rpul->show();
    });
    auto banUserBtn = CCMenuItemExt::createSpriteExtra(banUserSprite, [this](auto){
        auto rpul = ReplyPunishUserLayer::create("ban",this->m_report.account_name,this->m_report.reason);
        rpul->show();
    });

    actionsMenu->addChild(sendNoticeBtn);
    actionsMenu->addChild(muteUserBtn);
    if (g_permissions >= ModerationPermissions::Administrator) actionsMenu->addChild(banUserBtn);

    layout = AxisLayout::create(Axis::Row);
    layout->setGap(10.f);
    layout->setAxisAlignment(AxisAlignment::Start);
    layout->setAutoScale(false);

    actionsMenu->setLayout(layout);
    actionsMenu->updateLayout();

    actionsMenu->setAnchorPoint({0,0.5});
    actionsMenu->setScale(0.5f);
    actionsMenu->setPosition({5.f,12.5f});

    this->addChild(actionsMenu);

    auto closeMenu = CCMenu::create();

    auto acceptBtn = CCMenuItemExt::createSpriteExtra(CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"), [this](auto){
        createQuickPopup("Are you sure?","Are you sure you want to <co>close</c> this report as <cg>complete</c>?","NO","YES",[this](auto,bool btn2){
            if (btn2) this->doClose(true);
        });
    });

    auto ignoreBtn = CCMenuItemExt::createSpriteExtra(CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png"), [this](auto){
        createQuickPopup("Are you sure?","Are you sure you want to <co>close</c> this report as <cr>ignored</c>?","NO","YES",[this](auto,bool btn2){
            if (btn2) this->doClose(false);
        });
    });

    closeMenu->addChild(acceptBtn);
    closeMenu->addChild(ignoreBtn);

    layout = AxisLayout::create(Axis::Row);
    layout->setGap(5.f);
    layout->setAxisAlignment(AxisAlignment::End);
    layout->setAutoScale(false);

    closeMenu->setLayout(layout);
    closeMenu->updateLayout();
    
    closeMenu->setAnchorPoint({1,0.5});
    closeMenu->setPosition({this->getContentWidth()-5.f,this->getContentHeight()-10.f});
    closeMenu->setScale(0.5f);

    this->addChild(closeMenu);

    return true;
}

ReportCell* ReportCell::create(RepliesBasePaginatedLayer* rl,Report report,ReplyBackgroundColor bgColor){
    auto ret = new ReportCell();
    ret->m_rl = rl;
    ret->m_report = report;
    ret->m_bgColor = bgColor;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}