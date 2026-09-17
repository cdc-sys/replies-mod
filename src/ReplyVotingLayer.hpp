#pragma once
#include "Geode/binding/LikeItemLayer.hpp"
#include "Geode/loader/Event.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/modify/LikeItemLayer.hpp>
#include "ReplyCell.hpp"

using namespace geode::prelude;
class ReplyVotingLayer : public geode::Popup {
protected:
    std::string id;
    TaskHolder<web::WebResponse> m_requestListener;
    ReplyCell* rc;
    void castVote(int type){
        auto req = web::WebRequest();
        this->retain();
        this->rc->updateLikes(this->rc->m_reply.likes+type);
        this->removeFromParent();
        auto url = fmt::format("{}/replies/{}/vote",SERVER_URL,this->id);
        req.header("Authorization", Mod::get()->getSavedValue<std::string>("token"));
        req.header("mod-version",MOD_VERSION_HEADER);
        req.param("type",type);
        this->m_requestListener.spawn(req.put(url),[this,type](web::WebResponse res){
            if (!res.ok()) {
                auto json = res.json().unwrapOrDefault();
                if (json.contains("err")){
                    auto errorText = json["err"]["text"].asString().unwrapOr("Unknown");
                    auto notif = geode::Notification::create(fmt::format("Voting failed: {}",errorText),NotificationIcon::Error);
                    notif->show();
                }
            } else {
                g_votedOn.push_back(this->id);
            }
            this->release();
        });
    }
    bool init() {
        if (!Popup::init(240.f, 160.f))
            return false;

        this->setTitle("Vote");

        auto menu = CCMenu::create();
        menu->setLayout(geode::SimpleAxisLayout::create(Axis::Row));

        auto likeBtn = CCMenuItemExt::createSpriteExtra(CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png"),[this](auto s){
            this->castVote(1);
        });

        auto dislikeBtn = CCMenuItemExt::createSpriteExtra(CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png"),[this](auto s){
            this->castVote(-1);
        });

        menu->addChild(likeBtn);
        menu->addChild(dislikeBtn);

        this->m_mainLayer->addChild(menu);

        return true;
    }

public:
    static ReplyVotingLayer* create(std::string& id, ReplyCell* rc) {
        auto ret = new ReplyVotingLayer();
        ret->id = id;
        ret->rc = rc;
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};