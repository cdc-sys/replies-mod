#include "ReplyHistoryLayer.hpp"
#include "ReplyCell.hpp"
#include "Auth.hpp"

ReplyHistoryLayer* ReplyHistoryLayer::create(int accountID) {
    auto ret = new ReplyHistoryLayer();
    ret->m_accountID = accountID;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void ReplyHistoryLayer::show(){
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    CCScene::get()->addChild(this);
    this->setZOrder(CCScene::get()->getHighestChildZ()+1);
    this->m_mainLayer->setPosition({-this->getContentSize().width/2,winSize.height/2});
    this->m_mainLayer->runAction(
        cocos2d::CCEaseElasticOut::create(CCMoveTo::create(0.75f,{winSize.width/2,winSize.height/2}), 0.6f)
    );
    this->setOpacity(0);
    this->runAction(CCFadeTo::create(0.25f,125));
}

bool ReplyHistoryLayer::init(){
    if (!Popup::init(380,300)) return false; 

    std::string savedSortMode = Mod::get()->getSavedValue<std::string>("sort_mode");
    if (!savedSortMode.empty()) {
        sortMode = savedSortMode;
    }

    this->setTitle("Reply History");

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    /*auto newContentHeight = ((winSize.height-m_bgSprite->getContentHeight())/2)+100;
    m_bgSprite->setContentHeight(m_bgSprite->getContentHeight()+newContentHeight);
    m_bgSprite->setPositionY(m_bgSprite->getPositionY()-newContentHeight/2);*/

    auto newSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    this->m_closeBtn->setNormalImage(newSprite);
    this->m_closeBtn->updateSprite();
    this->m_closeBtn->setPosition({(this->m_buttonMenu->getContentSize().width-winSize.width)/2+22.5f,this->m_buttonMenu->getContentSize().height-15.f});
    

    m_scrollLayer = geode::ScrollLayer::create({335.f,248.f});
    m_scrollLayer->setPosition({37.f,16.f});
    this->m_mainLayer->addChild(m_scrollLayer);
    
    auto border = CCScale9Sprite::create("geode.loader/inverseborder.png");
    border->setContentSize(m_scrollLayer->getContentSize());
    border->ignoreAnchorPointForPosition(true);
    border->setPosition({37.f,16.f});
    border->setAnchorPoint({0,0});
    this->m_mainLayer->addChild(border);

    auto reloadSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    reloadBtn = CCMenuItemSpriteExtra::create(reloadSpr,this,menu_selector(ReplyHistoryLayer::onReload));
    reloadBtn->setPosition({20.f,280.f});
    reloadSpr->setScale(0.5f);
    this->m_buttonMenu->addChild(reloadBtn);

    auto sortModeMenu = CCMenu::create();
    std::vector<std::pair<std::string,std::string>> sortingModes = {{"likes","GJ_likesIcon_001.png"},{"dislikes","GJ_dislikesIcon_001.png"},{"newest","GJ_sRecentIcon_001.png"},{"oldest","d_time01_001.png"}};
    for (auto sort : sortingModes) {
        auto sprite = CCSprite::create("GJ_button_01.png");
        if (sort.first == sortMode) sprite = CCSprite::create("GJ_button_02.png");
        sprite->setScale(0.5f);

        auto toggler = CCMenuItemExt::createSpriteExtra(sprite, [this,sort](CCMenuItemSpriteExtra* me) {
            this->sortMode = sort.first;
            this->m_page = 1;
            this->loadReplies(true);
            auto onSprite = CCSprite::create("GJ_button_02.png");
            auto offSprite = CCSprite::create("GJ_button_01.png");

            onSprite->setScale(0.5f);
            offSprite->setScale(0.5f);

            for (auto child : me->getParent()->getChildrenExt()) {
                typeinfo_cast<CCMenuItemSpriteExtra*>(child)->setSprite(offSprite);
            }
            me->setSprite(onSprite);
            me->getParent()->updateLayout();
            
            Mod::get()->setSavedValue<std::string>("sort_mode", sort.first);
        });

        auto sortIcon = CCSprite::createWithSpriteFrameName(sort.second.c_str());
        sortIcon->setScale(12/sortIcon->getContentWidth());
        sortIcon->setPosition({toggler->getContentWidth()/2,toggler->getContentHeight()/2});
        sortIcon->setZOrder(999);
        if (sort.first == "dislikes") sortIcon->setContentHeight(24.f); // robtop why the fuckkk is your texture fucked up like this
        toggler->addChild(sortIcon);

        sortModeMenu->addChild(toggler);
    }

    auto layout = AxisLayout::create(Axis::Column);
    layout->setAutoGrowAxis(1.f);
    layout->setGap(3.f);
    layout->setAxisReverse(true);
    sortModeMenu->setLayout(layout);
    sortModeMenu->updateLayout();

    sortModeMenu->setPosition({20.f,260.f});
    sortModeMenu->setAnchorPoint({0.5,1});

    this->m_mainLayer->addChild(sortModeMenu);

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
    this->m_nextBtn->setPosition({400.f,winSize.height/2.5f});
    this->m_buttonMenu->addChild(m_prevBtn);
    this->m_buttonMenu->addChild(m_nextBtn);

    pageLabel = CCLabelBMFont::create("Page ?/? (Total: ?)","chatFont.fnt");
    pageLabel->limitLabelWidth(200.f, 0.6f, 0.2f);
    pageLabel->setColor({0,0,0});
    pageLabel->setOpacity(90);
    pageLabel->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
    pageLabel->setAnchorPoint({1,0.5});
    pageLabel->setPosition({365.f,275.f});
    this->m_mainLayer->addChild(pageLabel);
    
    this->loadReplies(false);
    return true;
}
/*bool isLastInTree(Reply reply){
    bool ret = false;
    if (reply.parent){
        auto parentReplies = (*reply.parent).replies;
        if (parentReplies[parentReplies.size()-1].id != reply.id) return false;
        ret = isLastInTree(*reply.parent);
    } else return true;
    return ret;
}*/
/*float ReplyHistoryLayer::iterate(Reply reply,int replyLevel, Reply parentReply,int skip){
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
        if (last && reply.reply_count > 0 && replyLevel > 1) {
            auto replyCell = ReplyCell::create(this,reply,(this->_m_darker ? ReplyBackgroundColor::Darker : ReplyBackgroundColor::Regular),replyLevel,ReplySpriteType::MoreReplies,skip);
            m_scrollLayer->m_contentLayer->addChild(replyCell);
            total += replyCell->getContentHeight();
            this->_m_darker = !this->_m_darker;
            moreAdded = true;
        }
        if (last&&isLastInTree(reply_)) skip += 1;
        total += iterate(reply_,replyLevel+1,reply,skip);
        i++;
    }
    if (reply.reply_count > 0 && replyLevel > 1) {
        // really annoying edgecase
        bool noRepliesShown = reply.replies.size() == 0 && parentReply.replies[parentReply.replies.size()-1].id == reply.id;
        auto replyCell = ReplyCell::create(this,reply,(this->_m_darker ? ReplyBackgroundColor::Darker : ReplyBackgroundColor::Regular),replyLevel,ReplySpriteType::MoreReplies,skip,1+noRepliesShown);
        m_scrollLayer->m_contentLayer->addChild(replyCell);
        total += replyCell->getContentHeight();
        this->_m_darker = !this->_m_darker;
    }
    return total;
}*/

void ReplyHistoryLayer::populate(std::vector<Reply> const& replies,std::string const& message){
    if (m_page==1) m_prevBtn->setVisible(false);
    else m_prevBtn->setVisible(true);
    if (m_page==m_maxPages) m_nextBtn->setVisible(false);
    else m_nextBtn->setVisible(true);
    reloadBtn->setEnabled(true);

    /*g_replyCache[m_commentID].message = message;
    g_replyCache[m_commentID].max_pages = this->m_maxPages;
    g_replyCache[m_commentID].total_replies = this->m_totalReplies;
    g_replyCache[m_commentID].cached[this->m_page].replies = replies;
    g_replyCache[m_commentID].time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());*/
    float totalHeight = 0.f;
    m_scrollLayer->m_contentLayer->removeAllChildren();
    m_scrollLayer->m_contentLayer->setLayout(
        geode::ColumnLayout::create()
            ->setGap(0.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(geode::AxisAlignment::End)
            ->setCrossAxisLineAlignment(geode::AxisAlignment::End)
    );
    /*auto topCell = ReplyCell::create(this,topReply,ReplyBackgroundColor::Highlighted,0);
    m_scrollLayer->m_contentLayer->addChild(topCell);
    totalHeight += topCell->getContentSize().height;*/
    this->_m_darker = true;
    for (auto reply : replies) {
        auto replyCell = ReplyCell::create(nullptr,reply,(_m_darker ? ReplyBackgroundColor::Darker : ReplyBackgroundColor::Regular),0,ReplySpriteType::Line,0,0);
        totalHeight += replyCell->getContentHeight();
        m_scrollLayer->m_contentLayer->addChild(replyCell);
        this->_m_darker = !this->_m_darker;
    }
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

void ReplyHistoryLayer::loadReplies(bool force){
    /*auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
    auto cache = g_replyCache[this->m_commentID];
    auto diff = now-g_replyCache[this->m_commentID].time;
    if (cache.cached.count(this->m_page)&&diff<std::chrono::seconds(300)&&!force){
        if (now-cache.cached[this->m_page].time<std::chrono::seconds(300)){
            this->m_maxPages = cache.max_pages;
            this->m_totalReplies = cache.total_replies;
            this->populate(cache.cached[this->m_page].replies,cache.message);
            return;
        } else {
            geode::log::info("updating cache");
        }
    }*/

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

    req.header("mod-version",MOD_VERSION_HEADER);
    req.param("sort",sortMode);
    auto url = fmt::format("{}/reply_history/{}/{}",SERVER_URL,m_accountID,this->m_page);
    geode::log::info("{}",url);
    m_webListener.spawn(req.get(url),[this,loadingSpinner](web::WebResponse res){
        loadingSpinner->removeFromParent();
        if (res.ok()){
            auto json = res.json().unwrapOrDefault();
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
            auto json = res.json().unwrapOrDefault();
            if (json.contains("err")){
                auto error = json["err"]["text"].asString().unwrapOr("");
                this->populate({},error);
            } else {
                geode::log::error("Failed to load replies: {}",res.string().unwrapOr("Unknown"));
                this->populate({},"Something went wrong.");
            }
        }
    });
}

void ReplyHistoryLayer::onReload(CCObject* sender){
    //g_replyCache[this->m_commentID].cached = {};
    this->loadReplies(true);
}