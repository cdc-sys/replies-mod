#pragma once
#include "Geode/binding/LikeItemLayer.hpp"
#include "Geode/loader/Event.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/modify/LikeItemLayer.hpp>
#include "ReplyCell.hpp"

using namespace geode::prelude;

class $modify(MyLikeItemLayer,LikeItemLayer){
    struct Fields {
        std::string id;
        EventListener<web::WebTask> m_requestListener;
        ReplyCell* rc;
    };
    void castVote(int type){
        auto req = web::WebRequest();
        this->retain();
        this->m_fields->rc->updateLikes(this->m_fields->rc->m_reply.likes+type);
        this->removeFromParent();
        this->m_fields->m_requestListener.bind([this,type](web::WebTask::Event* e){
            if (auto res = e->getValue()) {
                if (!res->ok()) {
                    auto json = res->json().unwrapOrDefault();
                    if (json.contains("err")){
                        auto errorText = json["err"]["text"].asString().unwrapOr("Unknown");
                        auto notif = geode::Notification::create(fmt::format("Voting failed: {}",errorText),NotificationIcon::Error);
                        notif->show();
                    }
                }
                this->release();
            }
        });
        auto url = fmt::format("http://localhost:6650/replies/{}/vote",this->m_fields->id);
        req.header("Authorization", "a");
        req.param("type",type);
        this->m_fields->m_requestListener.setFilter(req.put(url));
    }
    void onLike(CCObject* sender){
        if (!this->m_fields->id.empty()){
            this->castVote(1);
        } else LikeItemLayer::onLike(sender);
    }
    void onDislike(CCObject* sender){
        if (!this->m_fields->id.empty()){
            this->castVote(-1);
        } else LikeItemLayer::onDislike(sender);
    }
    static MyLikeItemLayer* createWrapper(std::string id,ReplyCell* rc){
        auto ret = new MyLikeItemLayer();
        ret->m_fields->id = id;
        ret->m_fields->rc = rc;
        if (ret&&ret->init(LikeItemType::Unknown,0,0)){
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};