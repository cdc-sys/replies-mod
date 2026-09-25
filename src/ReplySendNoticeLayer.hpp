#pragma once
#include "Geode/utils/web.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/modify/LikeItemLayer.hpp>
#include "ReplyCell.hpp"

using namespace geode::prelude;
class ReplySendNoticeLayer : public geode::Popup {
protected:
    std::string m_user;

    TaskHolder<web::WebResponse> m_requestListener;
    int m_optionChecked = -1;

    geode::TextInput* m_userInput;
    geode::TextInput* m_textInput;

    void doSend(){
        if (m_userInput->getString().empty() || m_textInput->getString().empty()) return;
        auto req = web::WebRequest();
        this->retain();
        this->removeFromParent();
        auto url = fmt::format("{}/moderation/send_notice",SERVER_URL);
        req.header("Authorization", Mod::get()->getSavedValue<std::string>("token"));
        req.header("mod-version",MOD_VERSION_HEADER);
        req.param("text",m_textInput->getString());
        req.param("user",m_userInput->getString());
        this->m_requestListener.spawn(req.post(url),[this](web::WebResponse res){
            if (!res.ok()) {
                auto json = res.json().unwrapOrDefault();
                if (json.contains("err")){
                    auto errorText = json["err"]["text"].asString().unwrapOr("Unknown");
                    auto notif = geode::Notification::create(fmt::format("Sending failed: {}",errorText),NotificationIcon::Error);
                    notif->show();
                }
            } else {
                auto notif = geode::Notification::create("Successfully sent notice!",NotificationIcon::Success);
                notif->show();
            }
            this->release();
        });
    }
    bool init() {
        if (!Popup::init(270.f, 200.f))
            return false;

        this->setTitle("Send Notice");

        this->m_userInput = TextInput::create(250.f,"Enter the user ID/name here..","chatFont.fnt");

        this->m_userInput->setPosition({this->m_mainLayer->getContentWidth()/2,140.f});

        this->m_userInput->setString(m_user);

        this->m_textInput = TextInput::create(250.f,"Enter the notice text here..","chatFont.fnt");

        this->m_textInput->setPosition({this->m_mainLayer->getContentWidth()/2,this->m_mainLayer->getContentHeight()/2.f});

        auto button = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Send"), [this](auto){
            this->doSend();
        });
        button->setPosition({this->m_mainLayer->getContentWidth()/2,25.f});
        button->m_pNormalImage->setScale(0.6f);

        this->m_mainLayer->addChild(m_textInput);
        this->m_buttonMenu->addChild(button);
        
        this->m_mainLayer->addChild(m_userInput);

        return true;
    }

public:
    static ReplySendNoticeLayer* create() {
        auto ret = new ReplySendNoticeLayer();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
    static ReplySendNoticeLayer* create(const std::string& user) {
        auto ret = new ReplySendNoticeLayer();
        ret->m_user = user;
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};