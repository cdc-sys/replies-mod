#include "ReplyNotificationListLayer.hpp"
#include "NotificationCell.hpp"
#include "ReplyPunishUserLayer.hpp"

ReplyNotificationListLayer* ReplyNotificationListLayer::create() {
    auto ret = new ReplyNotificationListLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void ReplyNotificationListLayer::show(){
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

bool ReplyNotificationListLayer::init(){
    if (!Popup::init(375,300)) return false; 

    this->setTitle("Notifications");

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto newSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    this->m_closeBtn->setNormalImage(newSprite);
    this->m_closeBtn->updateSprite();
    this->m_closeBtn->setPosition({(this->m_buttonMenu->getContentSize().width-winSize.width)/2+22.5f,this->m_buttonMenu->getContentSize().height-15.f});
    

    m_scrollLayer = geode::ScrollLayer::create({335.f,248.f});
    m_scrollLayer->setPosition({20.f,16.f});
    this->m_mainLayer->addChild(m_scrollLayer);
    
    auto border = CCScale9Sprite::create("geode.loader/inverseborder.png");
    border->setContentSize(m_scrollLayer->getContentSize());
    border->ignoreAnchorPointForPosition(true);
    border->setPosition({20.f,16.f});
    border->setAnchorPoint({0,0});
    this->m_mainLayer->addChild(border);

    auto reloadSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    reloadBtn = CCMenuItemSpriteExtra::create(reloadSpr,this,menu_selector(ReplyNotificationListLayer::onReload));
    reloadBtn->setPosition({20.f,280.f});
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
    pageLabel->setPosition({365.f,275.f});
    this->m_mainLayer->addChild(pageLabel);
    
    this->loadReplies(false);
    return true;
}

void ReplyNotificationListLayer::populate(std::vector<RepliesNotification> const& notifs,std::string const& message){
    if (m_page==1) m_prevBtn->setVisible(false);
    else m_prevBtn->setVisible(true);
    if (m_page==m_maxPages) m_nextBtn->setVisible(false);
    else m_nextBtn->setVisible(true);
    reloadBtn->setEnabled(true);

    float totalHeight = 0.f;
    m_scrollLayer->m_contentLayer->removeAllChildren();
    m_scrollLayer->m_contentLayer->setLayout(
        geode::ColumnLayout::create()
            ->setGap(0.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(geode::AxisAlignment::End)
            ->setCrossAxisLineAlignment(geode::AxisAlignment::End)
    );

    this->_m_darker = true;
    for (auto notif : notifs) {
        auto replyCell = NotificationCell::create(this,notif,(_m_darker ? ReplyBackgroundColor::Darker : ReplyBackgroundColor::Regular));
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

void ReplyNotificationListLayer::loadReplies(bool force){
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
    req.header("Authorization",Mod::get()->getSavedValue<std::string>("token"));
    auto url = fmt::format("{}/notifications/{}",SERVER_URL,this->m_page);
    geode::log::info("{}",url);
    m_webListener.spawn(req.get(url),[this,loadingSpinner](web::WebResponse res){
        loadingSpinner->removeFromParent();
        if (res.ok()){
            auto json = res.json().unwrapOrDefault();
            if (json.contains("notifications")){
                this->m_maxPages = json["total_pages"].asInt().unwrapOr(1);
                this->m_totalReplies = json["total"].asInt().unwrapOr(0);
                auto notifs = json["notifications"].asArray().unwrap();
                auto processedNotifs = std::vector<RepliesNotification>();
                for (auto _notif : notifs){
                    auto notif = _notif.as<RepliesNotification>();
                    processedNotifs.push_back(notif.unwrap());
                }
                this->populate(processedNotifs);
            }
        } else {
            auto json = res.json().unwrapOrDefault();
            if (json.contains("err")){
                auto error = json["err"]["text"].asString().unwrapOr("");
                this->populate({},error);
            } else {
                geode::log::error("Failed to load notifications: {}",res.string().unwrapOr("Unknown"));
                this->populate({},"Something went wrong.");
            }
        }
    });
}

void ReplyNotificationListLayer::onReload(CCObject* sender){
    this->loadReplies(true);
}