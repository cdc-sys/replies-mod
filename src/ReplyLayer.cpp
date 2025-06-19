#include "ReplyLayer.hpp"
#include "ReplyCell.hpp"
#include "Auth.hpp"

ReplyLayer* ReplyLayer::create(GJComment* comment) {
    auto ret = new ReplyLayer();
    ret->m_commentID = fmt::format("{}",comment->m_commentID);
    ret->m_comment=comment;
    if (ret && ret->initAnchored(375, 290,fmt::format("{}",comment->m_commentID))) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

ReplyLayer* ReplyLayer::create(Reply reply) {
    auto ret = new ReplyLayer();
    ret->m_commentID = reply.id;
    ret->m_comment = nullptr;
    ret->m_reply = reply;
    if (ret && ret->initAnchored(375, 290,reply.id)) {
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
    this->m_mainLayer->setPosition({winSize.width/2,-this->getContentSize().height/2});
    this->m_mainLayer->runAction(
        cocos2d::CCEaseElasticOut::create(CCMoveTo::create(0.75f,{winSize.width/2,winSize.height/2-20.f}), 0.6f)
    );
    this->setOpacity(0);
    this->runAction(CCFadeTo::create(0.25f,125));
}

void ReplyLayer::onAuthenticate(CCObject* sender){
    // auth stuff here oaaaaaaaaaaaaaaaaaaaaaaaaaa
    auto auth = Auth::create(this);
    auth->start();
}

void ReplyLayer::onClose(CCObject* sender){
    this->m_mainLayer->runAction(
        CCSequence::create(
            cocos2d::CCEaseIn::create(CCMoveTo::create(0.5f,{CCDirector::sharedDirector()->getWinSize().width/2,-this->getContentSize().height/2}), 3.f),
            cocos2d::CCCallFunc::create(this, callfunc_selector(ReplyLayer::removeFromParent)),
            nullptr
        )
    );
    this->runAction(cocos2d::CCEaseIn::create(cocos2d::CCFadeTo::create(0.5f,0),3.f));
}

void ReplyLayer::addReplyUI(){
    // remove authenticate ui if it exists, because this is only ran in setup and after auth
    if (authenticateBtn) authenticateBtn->removeFromParent();
    if (authenticateLabel) authenticateLabel->removeFromParent();

    m_replyTextInput = geode::TextInput::create(280.f,"Type your reply here...","chatFont.fnt");
    m_replyTextInput->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()-_=+[]{};:'\",.<>/?\\|`~ ");
    m_replyTextInput->setPosition({160.f,30.f});
    m_replyTextInput->setAnchorPoint({0.5f,0.5f});
    this->m_mainLayer->addChild(m_replyTextInput);

    auto uploadSpr = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png");
    m_uploadBtn = CCMenuItemSpriteExtra::create(uploadSpr,this,menu_selector(ReplyLayer::onUpload));
    m_uploadBtn->setPosition({330.f,30.f});
    uploadSpr->setScale(.825f);
    this->m_buttonMenu->addChild(m_uploadBtn);
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
    
    if (!Mod::get()->getSavedValue<std::string>("token").empty()){
        this->addReplyUI();
    } else {
        authenticateLabel = CCLabelBMFont::create("To reply to comments you must authenticate.","bigFont.fnt");
        authenticateLabel->setScale(0.325f);
        authenticateLabel->setAnchorPoint({0,0.5});
        authenticateLabel->setPosition({20,30});
        this->m_mainLayer->addChild(authenticateLabel);

        auto authMenu = CCMenu::create();
        auto authenticateSpr = ButtonSprite::create("Login");
        authenticateBtn = CCMenuItemSpriteExtra::create(authenticateSpr,this,menu_selector(ReplyLayer::onAuthenticate));
        authenticateSpr->setScale(.55f);
        authenticateSpr->setAnchorPoint({1,0.5});
        authenticateBtn->setPosition({355,30});
        authMenu->setPosition({0,0});
        authMenu->addChild(authenticateBtn);
        this->m_mainLayer->addChild(authMenu);
    }

    m_scrollLayer = geode::ScrollLayer::create({335.f,200.f});
    m_scrollLayer->setPosition({20.f,55.f});
    this->m_mainLayer->addChild(m_scrollLayer);
    
    auto border = CCScale9Sprite::create("geode.loader/inverseborder.png");
    border->setContentSize(m_scrollLayer->getContentSize());
    border->ignoreAnchorPointForPosition(true);
    border->setPosition({20.f,55.f});
    border->setAnchorPoint({0,0});
    this->m_mainLayer->addChild(border);

    auto reloadSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    reloadBtn = CCMenuItemSpriteExtra::create(reloadSpr,this,menu_selector(ReplyLayer::onReload));
    reloadBtn->setPosition({20.f,270.f});
    reloadSpr->setScale(0.5f);
    this->m_buttonMenu->addChild(reloadBtn);

    auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    nextSprite->setFlipX(true);
    this->m_prevBtn = CCMenuItemExt::createSpriteExtra(prevSprite, [this](auto btn){
        this->m_page -= 1;
        this->loadReplies(false);
    });
    this->m_nextBtn = CCMenuItemExt::createSpriteExtra(nextSprite, [this](auto btn){
        this->m_page += 1;
        this->loadReplies(false);
    });
    this->m_prevBtn->setPosition({-20.f,winSize.height/2.5f});
    this->m_nextBtn->setPosition({395.f,winSize.height/2.5f});
    this->m_buttonMenu->addChild(m_prevBtn);
    this->m_buttonMenu->addChild(m_nextBtn);

    pageLabel = CCLabelBMFont::create("Page ?/? (Total: ?)","chatFont.fnt");
    pageLabel->limitLabelWidth(200.f, 0.6f, 0.2f);
    pageLabel->setColor({0,0,0});
    pageLabel->setOpacity(90);
    pageLabel->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
    pageLabel->setAnchorPoint({1,0.5});
    pageLabel->setPosition({365.f,270.f});
    this->m_mainLayer->addChild(pageLabel);
    
    this->loadReplies(false);
    return true;
}
bool isLastInTree(Reply reply){
    bool ret = false;
    if (reply.parent){
        auto parentReplies = (*reply.parent).replies;
        if (parentReplies[parentReplies.size()-1].id != reply.id) return false;
        ret = isLastInTree(*reply.parent);
    } else return true;
    return ret;
}
float ReplyLayer::iterate(Reply reply,int replyLevel, Reply parentReply,int skip){
    float total = 0.f; 
    int i = 0;
    bool prevNested=false;
    for (auto reply_ : reply.replies){
        bool last = (i+1 == reply.replies.size());
        reply_.last = true;
        reply_.parent = &reply;
        ReplySpriteType spriteType = (last ? ReplySpriteType::Curl : ReplySpriteType::Line);
        if (reply_.replies.size() != 0){
            if (!isLastInTree(reply_)) spriteType = ReplySpriteType::LineCurl;
            prevNested = true;
        }
        if (prevNested&&!last){
            spriteType = ReplySpriteType::LineCurl;
            prevNested = false;
        }
        auto replyCell = ReplyCell::create(this,reply_,(this->_m_darker ? ReplyBackgroundColor::Darker : ReplyBackgroundColor::Regular),replyLevel,spriteType,skip);
        m_scrollLayer->m_contentLayer->addChild(replyCell);
        total += replyCell->getContentHeight();
        this->_m_darker = !this->_m_darker;
        if (last&&isLastInTree(reply_)) skip += 1;
        total += iterate(reply_,replyLevel+1,reply,skip);
        i++;
    }
    return total;
}

void ReplyLayer::populate(std::vector<Reply> const& replies,std::string const& message){
    if (m_page==1) m_prevBtn->setVisible(false);
    else m_prevBtn->setVisible(true);
    if (m_page==m_maxPages) m_nextBtn->setVisible(false);
    else m_nextBtn->setVisible(true);
    reloadBtn->setEnabled(true);

    g_replyCache[m_commentID].message = message;
    g_replyCache[m_commentID].max_pages = this->m_maxPages;
    g_replyCache[m_commentID].total_replies = this->m_totalReplies;
    g_replyCache[m_commentID].cached[this->m_page].replies = replies;
    g_replyCache[m_commentID].time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
    float totalHeight = 0.f;
    m_scrollLayer->m_contentLayer->removeAllChildren();
    m_scrollLayer->m_contentLayer->setLayout(
        geode::ColumnLayout::create()
            ->setGap(0.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(geode::AxisAlignment::End)
            ->setCrossAxisLineAlignment(geode::AxisAlignment::End)
    );
    Reply topReply;
    if (m_comment) topReply = replyFromComment(m_comment,m_totalReplies);
    else topReply = m_reply;
    auto topCell = ReplyCell::create(this,topReply,ReplyBackgroundColor::Highlighted,0);
    m_scrollLayer->m_contentLayer->addChild(topCell);
    totalHeight += topCell->getContentSize().height;
    this->_m_darker = true;
    Reply fakeReply;
    fakeReply.replies = replies;
    totalHeight += iterate(fakeReply,1);
    if (totalHeight < m_scrollLayer->getContentHeight()){
        auto filler = CCLayerColor::create();
        filler->setContentSize({335.f,m_scrollLayer->getContentHeight()-totalHeight});
        filler->setColor({0,0,0});
        filler->setOpacity(120);
        if (!message.empty()){
            auto label = CCLabelBMFont::create(message.c_str(),"goldFont.fnt");
            label->setScale(.5f);
            filler->addChildAtPosition(label,Anchor::Center);
        }
        m_scrollLayer->m_contentLayer->addChild(filler);
        totalHeight = m_scrollLayer->getContentHeight();
    }
    m_scrollLayer->m_contentLayer->setContentSize({335.f,totalHeight});
    m_scrollLayer->m_contentLayer->updateLayout();
    m_scrollLayer->moveToTop();

    pageLabel->setString(fmt::format("Page {}/{} (Total: {})",this->m_page,this->m_maxPages,geode::utils::numToAbbreviatedString(this->m_totalReplies)).c_str());
}

void ReplyLayer::onUpload(CCObject* sender){
    if (m_replyTextInput->getString().length() > 0){
        m_uploadBtn->setEnabled(false);
        auto req = web::WebRequest();
        m_webListener.bind([this](web::WebTask::Event* e){
            if (auto res = e->getValue()){
                m_uploadBtn->setEnabled(true);
                if (res->ok()){
                    this->m_page = std::ceil((m_totalReplies)/10)+1;
                    this->m_scrollLayer->m_contentLayer->removeAllChildren();
                    this->loadReplies(true);
                    m_replyTextInput->setString("");
                } else {
                    auto json = res->json().unwrapOrDefault();
                    if (json.contains("err")){
                        auto error = json["err"]["text"].asString().unwrapOr("Unknown");
                        auto notif = geode::Notification::create(fmt::format("Failed to post: {}",error),NotificationIcon::Error);
                        notif->show();
                        onUploadFailed(json["err"]["code"].asInt().unwrapOrDefault());
                    } else {
                        geode::log::error("Failed to load replies: {}",res->string().unwrapOr("Unknown"));
                        onUploadFailed(0);
                    }
                }
            }
        });
        req.header("Authorization", Mod::get()->getSavedValue<std::string>("token"));
        req.header("mod-version",MOD_VERSION_HEADER);
        auto url = fmt::format("{}/replies/{}",SERVER_URL,m_commentID);
        req.param("c",m_replyTextInput->getString());
        m_webListener.setFilter(req.post(url));
    }
}

void ReplyLayer::loadReplies(bool force){
    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
    auto cache = g_replyCache[this->m_commentID];
    auto diff = now-g_replyCache[this->m_commentID].time;
    if (cache.cached.count(this->m_page)&&diff<std::chrono::seconds(300)&&!force){
        if (now-cache.cached[this->m_page].time<std::chrono::seconds(300)){
            this->m_maxPages = cache.max_pages;
            this->m_totalReplies = cache.total_replies;
            this->populate(cache.cached[this->m_page].replies,cache.message);
        }
        return;
    }

    this->populate({});
    this->m_nextBtn->setVisible(false);
    this->reloadBtn->setEnabled(false);

    auto req = web::WebRequest();
    auto loadingSpinner = LoadingCircle::create();
    loadingSpinner->setParentLayer(this->m_mainLayer);
    loadingSpinner->setContentSize(this->m_scrollLayer->getContentSize());
    loadingSpinner->setPosition(this->m_scrollLayer->getPosition());

    // why do i have to do this?
    auto spinnerSprite = loadingSpinner->m_sprite;
    spinnerSprite->setPosition({loadingSpinner->getContentWidth()/2,loadingSpinner->getContentHeight()/2.5f});

    // there's apparently no way to stop the fading without just.. remaking the entire show function LOLLL
    m_mainLayer->addChild(loadingSpinner);
    spinnerSprite->runAction(CCRepeatForever::create(CCRotateBy::create(1,360)));
    spinnerSprite->setBlendFunc({ GL_ONE, GL_ONE });
    spinnerSprite->setOpacity(200);

    m_webListener.bind([this,loadingSpinner](web::WebTask::Event* e){
        if (auto res = e->getValue()){
            loadingSpinner->removeFromParent();
            if (res->ok()){
                auto json = res->json().unwrapOrDefault();
                if (json.contains("replies")){
                    this->m_maxPages = json["total_pages"].asInt().unwrapOr(1);
                    this->m_totalReplies = json["total"].asInt().unwrapOr(0);
                    auto replies = json["replies"].asArray().unwrap();
                    auto processedReplies = std::vector<Reply>();
                    for (auto _reply : replies){
                       auto reply = _reply.as<Reply>();
                       processedReplies.push_back(reply.unwrap());
                    }
                    this->populate(processedReplies);
                }
            } else {
                auto json = res->json().unwrapOrDefault();
                if (json.contains("err")){
                    auto error = json["err"]["text"].asString().unwrapOr("");
                    this->populate({},error);
                } else {
                    geode::log::error("Failed to load replies: {}",res->string().unwrapOr("Unknown"));
                    this->populate({},"Something went wrong.");
                }
            }
        }
    });
    req.header("mod-version",MOD_VERSION_HEADER);
    auto url = fmt::format("{}/replies/{}/{}",SERVER_URL,m_commentID,this->m_page);
    geode::log::info("{}",url);
    m_webListener.setFilter(req.get(url));
}

void ReplyLayer::onUploadFailed(int code){
    auto pos = m_mainLayer->getPosition();
    m_mainLayer->setPosition(pos+ccp(25.f,0.f));
    auto action = CCEaseElasticOut::create(CCMoveTo::create(1,pos),0.3);
    m_mainLayer->runAction(action);
    switch (code){
        case 102:
            m_replyTextInput->setString("");
            break;
    }
}
void ReplyLayer::onReload(CCObject* sender){
    g_replyCache[this->m_commentID].cached = {};
    this->loadReplies(true);
}