#include "ReplyCell.hpp"
#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/binding/GJAccountManager.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/SimpleAxisLayout.hpp"
#include "Structs.hpp"
#include "ReplyVotingLayer.hpp"

void ReplyCell::onReply(CCObject* sender){
    auto rl = ReplyLayer::create(m_reply);
    rl->show();
}

void ReplyCell::onVote(CCObject* sender){
    if (VECTOR_HAS_ITEM(g_votedOn, this->m_reply.id)) return;

    if (m_reply.from_comment) return;
    auto likeLayer = ReplyVotingLayer::create(this->m_reply.id,this);
    likeLayer->show();
}
void ReplyCell::doDelete(){
    if (m_reply.from_comment) return;
    auto req = web::WebRequest();
    auto url = fmt::format("{}/replies/{}/",SERVER_URL,m_reply.id);
    req.header("Authorization", Mod::get()->getSavedValue<std::string>("token"));
    req.header("mod-version",MOD_VERSION_HEADER);
    this->m_webListener.spawn(req.send("DELETE", url),[this](web::WebResponse res){
        if (!res.ok()) {
            auto json = res.json().unwrapOrDefault();
            if (json.contains("err")){
                auto errorText = json["err"]["text"].asString().unwrapOr("Unknown");
                auto notif = geode::Notification::create(fmt::format("Failed to delete: {}",errorText),NotificationIcon::Error);
                notif->show();
            }
        } else {
            m_rl->m_page = std::ceil((m_rl->m_totalReplies-2)/10)+1;
            m_rl->loadReplies(true);
        }
    });
}
void ReplyCell::onDelete(CCObject* sender){
    createQuickPopup("Delete Reply","Are you sure you want to <cr>delete</c> this reply?","No","Yes",[this](auto alert, bool btn2){
        if (btn2){
            doDelete();
        }
    });
}
void scaleAreaToFit(SimpleTextArea* area,float max){
    // cant decide rn, but im keeping the bad version for now..
    bool scaled = false;
    while (area->getScaledContentHeight() > max){
        area->setScale(area->getScale()-0.01f);
        scaled = true;
    }
    if (scaled) {
        if (area->getLines().size() == 1){
            area->setPositionY(area->getPositionY()-area->getScaledContentHeight()/4);
        }
    }
    /*if (area->getLines().size()==1)return;
    float height = area->getLineHeight();
    float padding = area->getLinePadding();
    float total = height+padding;
    int max_lines = std::ceil(max/total);
    int area_lines = area->getLines().size();
    float a = max_lines*height;
    float c = a/area_lines;
    float offset = 0;
    if (max_lines==area_lines) offset = (max_lines-1)*0.2f;
    area->setScale((c/height)*area->getScale()-offset);
    if (area->getLines().size() == 1){
        area->setPositionY(area->getPositionY()-area->getScaledContentHeight()/4);
    }*/

}

void ReplyCell::fetchContent() {
    auto loadingSpinner = LoadingCircle::create();
    loadingSpinner->setContentSize(this->getContentSize());
    loadingSpinner->setPosition(this->getPosition());

    // why do i have to do this?
    auto spinnerSprite = loadingSpinner->m_sprite;
    spinnerSprite->setPosition({loadingSpinner->getContentWidth()/2,loadingSpinner->getContentHeight()/2.5f});
    spinnerSprite->setScale(0.3f);

    // there's apparently no way to stop the fading without just.. remaking the entire show function LOLLL
    this->addChild(loadingSpinner);
    spinnerSprite->runAction(CCRepeatForever::create(CCRotateBy::create(1,360)));
    spinnerSprite->setBlendFunc({ GL_ONE, GL_ONE });
    spinnerSprite->setOpacity(200);

    auto req = web::WebRequest();

    req.header("mod-version",MOD_VERSION_HEADER);
    auto url = fmt::format("{}/reply/{}/",SERVER_URL,m_reply.id);
    geode::log::info("{}",url);
    m_webListener.spawn(req.get(url),[this,loadingSpinner](web::WebResponse res){
        loadingSpinner->removeFromParent();
        if (res.ok()){
            auto json = res.json().unwrapOrDefault();
            if (json.contains("replies")){
                auto json_reply = json.as<Reply>().unwrapOrDefault();
                json_reply.needs_web_fetch = false;
                this->m_reply = json_reply;
                this->init(true);
            }
        } else {
            auto json = res.json().unwrapOrDefault();
            if (json.contains("err")){
                auto error = json["err"]["text"].asString().unwrapOr("");
                this->m_reply.needs_web_fetch = false;
                this->m_reply.author_name = "Unknown";
                this->m_reply.content = fmt::format("[ Failed to load: {} ]",error);
                this->init(true);
            } else {
                geode::log::error("Failed to load replies: {}",res.string().unwrapOr("Unknown"));
                this->m_reply.needs_web_fetch = false;
                this->m_reply.author_name = "Unknown";
                this->m_reply.content = "[ Failed to load ]";
                this->init(true);
            }
        }
    });
}

bool ReplyCell::init(bool fromFetch){
    bool moreReplies = this->m_spriteType == ReplySpriteType::MoreReplies;

    if (!fromFetch) {
        if (!CCNode::init()) return false;

        // og width 36
        float offset = 23*m_replyLevel;
        this->setContentSize({335.f-offset,36.f});
        if (m_rl && m_rl->m_displayMode==Mode::LargeCells){
            this->setContentHeight(90.f);
        }

        if (moreReplies) {
            this->setContentHeight(23.f);
        }

        auto line = CCLayerColor::create();
        line->setColor({0,0,0});
        line->setContentSize({this->getContentSize().width,.425f});
        line->setOpacity(125);
        this->addChild(line);

        auto line2 = CCLayerColor::create();
        line2->setColor({0,0,0});
        line2->setContentSize({.425f,this->getContentHeight()});
        line2->setOpacity(125);
        this->addChild(line2);

        auto bg2 = CCLayerColor::create();
        bg2->setColor({0,0,0});
        bg2->setOpacity(120);
        bg2->setContentSize({offset,this->getContentSize().height});
        bg2->setAnchorPoint({0,0});
        bg2->setPosition({-offset,0});
        this->addChild(bg2);

        std::vector<ccColor3B> line_colours = {
            {255, 0, 255},
            {0,255,0},
            {0, 234, 255},
            {255, 242, 0}
        };

        bool colouredBranchesEnabled = Mod::get()->getSettingValue<bool>("coloured-branches");
        
        for (int i = m_skipLinesRight; i<m_replyLevel-m_skipLines; i++){
            if (moreReplies && i == 0) continue;
            auto spriteName = fmt::format("reply-{}.png"_spr,(i>0 ? 1 : (int)this->m_spriteType+1));
            auto sprite = CCSprite::createWithSpriteFrameName(spriteName.c_str());
            sprite->setScale(23/sprite->getContentWidth());
            if (moreReplies) sprite->setScaleY(23/sprite->getContentHeight());
            //sprite->setScale(4.f);
            sprite->setOpacity(50);
            sprite->setAnchorPoint({0,0});
            sprite->setPosition({-23.f*(i+1),0});
            if (colouredBranchesEnabled) {
                sprite->setColor(line_colours[abs(m_replyLevel-m_skipLines-i) % 4]);
                sprite->setOpacity(100);
            }
            this->addChild(sprite);
        }

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

    }

    if (m_reply.needs_web_fetch) {
        this->fetchContent();
        return true;
    }
    
    float playerIconOffset = 0.f;
    if (!m_reply.account_comment && !moreReplies){
        auto playerIcon = SimplePlayer::create(m_reply.icon.type);
        playerIcon->setPosition({5.f,this->getContentHeight()-(30.f*0.45f)-4.f});
        playerIcon->setScale(0.45f);
        playerIcon->setColors(GameManager::get()->colorForIdx(m_reply.icon.primaryColor), GameManager::get()->colorForIdx(m_reply.icon.secondaryColor));
        playerIcon->updatePlayerFrame(m_reply.icon.id, (IconType)m_reply.icon.type);
        if (m_reply.icon.glow) playerIcon->setGlowOutline(GameManager::get()->colorForIdx(m_reply.icon.glowColor));
        for (auto child : CCArrayExt<CCNode*>(playerIcon->getChildren())){
            child->ignoreAnchorPointForPosition(true);
        }
        this->addChild(playerIcon);
        playerIconOffset = 30.f*0.45f;
    }

    auto authorMenu = CCMenu::create();

    auto authorLabel = CCLabelBMFont::create(m_reply.author_name.c_str(),"goldFont.fnt");
    if (moreReplies) authorLabel->setString(fmt::format("+ {} Repl{}",m_reply.reply_count,m_reply.reply_count == 1 ? "y" : "ies").c_str());
    authorLabel->setAlignment(kCCTextAlignmentLeft);
    authorLabel->setScale(0.5f);

    auto clickableAuthor = CCMenuItemExt::createSpriteExtra(authorLabel, [this,moreReplies](auto) {
        if (moreReplies){
            auto rl = ReplyLayer::create(m_reply);
            rl->show();
        } else {
            auto profile = ProfilePage::create(m_reply.author_id,m_reply.author_id == GJAccountManager::get()->m_accountID);
            profile->show();
        }
    });
    clickableAuthor->setAnchorPoint({0,0.5});
    clickableAuthor->setSizeMult(1.1f);

    authorMenu->addChild(clickableAuthor);

    authorMenu->setPosition({playerIconOffset+6.f+(playerIconOffset!=0 ? 2.0f : 0.f),this->getContentHeight()-10.f});

    this->addChild(authorMenu);

    if (moreReplies) return true;

    // come back to this idea later maybe
    //if (m_reply.likes < 0) return true;


    /*auto contentLabel = CCLabelBMFont::create(m_reply.content.c_str(),"chatFont.fnt",200.f,kCCTextAlignmentLeft);
    contentLabel->setAnchorPoint({0,0.5});
    contentLabel->setPosition({36.f,13.f});
    contentLabel->setScale(0.65f);*/

    //auto contentLabel = TextArea::create(m_reply.content,"chatFont.fnt",0.65f,200.f,{0,1},10.f,false);
    auto contentLabel = SimpleTextArea::create(m_reply.content,"chatFont.fnt",0.65f);
    contentLabel->setWidth(this->getContentWidth()-70.f);
    //contentLabel->setMaxLines(2);
    contentLabel->setWrappingMode(WrappingMode::WORD_WRAP);
    if (m_reply.content.find(" ")==-1) contentLabel->setWrappingMode(WrappingMode::CUTOFF_WRAP);
    contentLabel->setPosition({5.f,this->getContentHeight()-18.f});
    if (m_rl && m_rl->m_displayMode==Mode::LargeCells) contentLabel->setPosition({5.f,this->getContentHeight()/2});
    contentLabel->setAnchorPoint({0,1});
    if (m_rl && m_rl->m_displayMode==Mode::LargeCells) contentLabel->setAnchorPoint({0,0.5});
    //if (contentLabel->getLines().size()>=2) {contentLabel->setScale(0.45f);if (contentLabel->getLines().size()==1){contentLabel->setPositionY(contentLabel->getPositionY()-contentLabel->getScaledContentHeight()/2);}}
    scaleAreaToFit(contentLabel,16.f);
    this->addChild(contentLabel);

    std::string timestamp = (m_reply.from_comment ? m_reply.comment_timestamp+" ago" : toAgoString(m_reply.timestamp/1000));
    auto dateLabel = CCLabelBMFont::create(timestamp.c_str(),"chatFont.fnt");
    dateLabel->setAlignment(kCCTextAlignmentRight);
    dateLabel->setAnchorPoint({1,0});
    dateLabel->setPosition({this->getContentWidth()-5.f,2.f});
    dateLabel->setScale(0.45f);
    dateLabel->setColor({0,0,0});
    dateLabel->setOpacity(125);
    this->addChild(dateLabel);

    auto replySpr = CCSprite::createWithSpriteFrameName("GJ_undoBtn_001.png");
    replySpr->setScale(.6f);
    auto replyBtn = CCMenuItemSpriteExtra::create(replySpr,this,menu_selector(ReplyCell::onReply));
    //auto replyMenu = CCMenu::create();
    //replyMenu->addChild(replyBtn);
    //replyMenu->setPosition({0,0});
    //replyBtn->setPosition({this->getContentWidth()-25.f,this->getContentHeight()/2});
    //this->addChild(replyMenu);

    auto likeMenu = CCMenu::create();
    likeSpr = CCSprite::createWithSpriteFrameName("GJ_likesIcon_001.png");
    if (m_reply.likes < 0) {
        auto cs = likeSpr->getContentSize();
        likeSpr = CCSprite::createWithSpriteFrameName("GJ_dislikesIcon_001.png");
        likeSpr->setContentSize(cs);
    }
    auto likeBtn = CCMenuItemSpriteExtra::create(likeSpr,this,menu_selector(ReplyCell::onVote));
    likeLabel = CCLabelBMFont::create("0","bigFont.fnt");
    likeMenu->addChild(likeLabel);
    likeMenu->addChild(likeBtn);
    if ((g_permissions >= ModerationPermissions::CommentModeration || m_reply.author_id == GJAccountManager::get()->m_accountID) && !m_reply.from_comment){
        auto deleteSpr = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
        auto deleteBtn = CCMenuItemSpriteExtra::create(deleteSpr,this,menu_selector(ReplyCell::onDelete));
        likeMenu->addChild(deleteBtn);
    } else {
        auto reportSpr = CCSprite::createWithSpriteFrameName("reportBtn.png"_spr);
        auto reportBtn = CCMenuItemExt::createSpriteExtra(reportSpr, [](auto){});
        likeMenu->addChild(reportBtn);
    }
    if (!m_reply.from_comment && this->m_bgColor != Highlighted) likeMenu->addChild(replyBtn);

    if (!m_reply.comment_id.empty()) {
        auto buttonSprite = ButtonSprite::create(m_reply.comment_id.c_str(),120,50,1.f,false);
        auto parentButton = CCMenuItemExt::createSpriteExtra(buttonSprite, [this](auto) {
            auto fake_reply = Reply();
            fake_reply.needs_web_fetch = true;
            fake_reply.id = m_reply.comment_id;
            auto rl = ReplyLayer::create(fake_reply);
            rl->show();
        });
        likeMenu->addChild(parentButton);
    }
    auto layout = AxisLayout::create(Axis::Row);
    layout->setAxisReverse(true);
    layout->setAutoGrowAxis(1.f);
    layout->setGap(10.f);
    likeMenu->setLayout(layout);
    likeMenu->updateLayout();

    // temp code
    auto cs = likeLabel->getContentWidth();
    likeLabel->setString(fmt::format("{}",m_reply.likes).c_str());
    likeLabel->limitLabelWidth(cs, .5f, .01f);

    likeMenu->setAnchorPoint({1,1});
    likeMenu->setScale(0.55f);
    this->addChildAtPosition(likeMenu,Anchor::TopRight,{-5,-5});

    return true;
}

void ReplyCell::updateLikes(int likes){
    likeLabel->setString(fmt::format("{}",likes).c_str());
    auto temp = CCSprite::createWithSpriteFrameName("GJ_likesIcon_001.png");
    auto cs = temp->getContentSize();
    if (likes < 0) temp = CCSprite::createWithSpriteFrameName("GJ_dislikesIcon_001.png");
    likeSpr->setDisplayFrame(temp->displayFrame());
    likeSpr->setContentSize(cs);
}

ReplyCell* ReplyCell::create(ReplyLayer* rl,Reply reply,ReplyBackgroundColor bgColor, int replyLevel,ReplySpriteType spriteType, int skipLines, int skipLinesRight){
    auto ret = new ReplyCell();
    ret->m_rl = rl;
    ret->m_reply = reply;
    ret->m_bgColor = bgColor;
    ret->m_replyLevel = replyLevel;
    ret->m_spriteType = spriteType;
    ret->m_skipLines = skipLines;
    ret->m_skipLinesRight = skipLinesRight;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}