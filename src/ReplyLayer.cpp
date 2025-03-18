#include "ReplyLayer.hpp"
#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/cocos/actions/CCActionInterval.h"
#include "Geode/cocos/cocoa/CCObject.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include "Geode/ui/Layout.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/cocos.hpp"
#include "ReplyCell.hpp"
#include "Geode/utils/web.hpp"
#include "Structs.hpp"

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

    m_scrollLayer = geode::ScrollLayer::create({335.f,200.f});
    m_scrollLayer->setPosition({20.f,55.f});
    this->m_mainLayer->addChild(m_scrollLayer);
    
    auto border = CCScale9Sprite::create("geode.loader/inverseborder.png");
    border->setContentSize(m_scrollLayer->getContentSize());
    border->ignoreAnchorPointForPosition(true);
    border->setPosition({20.f,55.f});
    border->setAnchorPoint({0,0});
    this->m_mainLayer->addChild(border);
    
    this->loadReplies();
    return true;
}

void ReplyLayer::populate(std::vector<Reply> const& replies){
    float totalHeight = 0.f;
    m_scrollLayer->m_contentLayer->setLayout(
        geode::ColumnLayout::create()
            ->setGap(0.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(geode::AxisAlignment::End)
            ->setCrossAxisLineAlignment(geode::AxisAlignment::End)
    );
    auto topCell = ReplyCell::create(replyFromComment(m_comment,m_totalReplies),ReplyBackgroundColor::Highlighted,0);
    m_scrollLayer->m_contentLayer->addChild(topCell);
    totalHeight += topCell->getContentSize().height;
    bool darker = true;
    int i = 0;
    bool _prevNested = false;
    for (auto reply : replies){
        ReplySpriteType spriteType = (i+1 == replies.size() ? ReplySpriteType::Curl : ReplySpriteType::Line);
        if (reply.replies.size() > 0){
            spriteType = ReplySpriteType::LineCurl;
        }
        if (_prevNested){
            spriteType = ReplySpriteType::LineCurl;
            _prevNested = false;
        }
        ReplyCell* cell = ReplyCell::create(reply,(darker ? ReplyBackgroundColor::Darker : ReplyBackgroundColor::Regular),1,spriteType);
        m_scrollLayer->m_contentLayer->addChild(cell);
        totalHeight += cell->getContentSize().height;
        darker = !darker;
        int i2 = 0;
        for (auto reply_ : reply.replies){
            spriteType = (i2+1 == reply.replies.size() ? ReplySpriteType::Curl : ReplySpriteType::Line);
            auto replyCell = ReplyCell::create(reply_,(darker ? ReplyBackgroundColor::Darker : ReplyBackgroundColor::Regular),2,spriteType);
            m_scrollLayer->m_contentLayer->addChild(replyCell);
            totalHeight += replyCell->getContentSize().height;
            darker = !darker;
            i2++;
            _prevNested=true;
        }
        i++;
    }
    if (totalHeight < m_scrollLayer->getContentHeight()){
        totalHeight = m_scrollLayer->getContentHeight();
    }
    m_scrollLayer->m_contentLayer->setContentSize({335.f,totalHeight});
    m_scrollLayer->m_contentLayer->updateLayout();
    m_scrollLayer->moveToTop();
}

void ReplyLayer::onUpload(CCObject* sender){
    if (m_replyTextInput->getString().length() > 0){
        auto req = web::WebRequest();
        m_webListener.bind([this](web::WebTask::Event* e){
            if (auto res = e->getValue()){
                if (res->ok()){
                    this->m_page = this->m_maxPages;
                    this->m_scrollLayer->m_contentLayer->removeAllChildren();
                    this->loadReplies();
                    m_replyTextInput->setString("");
                } else {
                    auto json = res->json().unwrapOrDefault();
                    if (json.contains("err")){
                        auto error = json["err"]["text"].asString().unwrapOr("");
                        geode::log::error("Failed to post: {}",error);
                    } else {
                        geode::log::error("Failed to load replies: {}",res->string().unwrapOr("Unknown"));
                    }
                }
            }
        });
        req.header("Authorization", "a");
        auto url = fmt::format("http://localhost:6650/replies/{}",m_commentID);
        req.param("c",m_replyTextInput->getString());
        m_webListener.setFilter(req.post(url));
    }
}

void ReplyLayer::loadReplies(){
    auto req = web::WebRequest();
    auto loadingSpinner = LoadingCircle::create();
    loadingSpinner->setParentLayer(this->m_mainLayer);
    loadingSpinner->setContentSize(this->m_mainLayer->getContentSize());
    loadingSpinner->show();
    m_webListener.bind([this,loadingSpinner](web::WebTask::Event* e){
        if (auto res = e->getValue()){
            loadingSpinner->fadeAndRemove();
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
                    if (error == "Invalid page." && this->m_page == 1){
                        geode::log::error("No replies for this comment :(");
                        this->populate({});
                    }
                } else {
                    geode::log::error("Failed to load replies: {}",res->string().unwrapOr("Unknown"));
                }
            }
        }
    });
    auto url = fmt::format("http://localhost:6650/replies/{}/{}",m_commentID,this->m_page);
    geode::log::info("{}",url);
    m_webListener.setFilter(req.get(url));
}