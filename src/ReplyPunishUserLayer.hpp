#pragma once
#include "Geode/utils/web.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/modify/LikeItemLayer.hpp>
#include "ReplyCell.hpp"

using namespace geode::prelude;
class ReplyPunishUserLayer : public geode::Popup {
protected:
    std::string m_type;
    std::string m_user;
    std::string m_reason;

    TaskHolder<web::WebResponse> m_requestListener;
    int m_optionChecked = -1;

    geode::TextInput* m_userInput;
    geode::TextInput* m_lengthInput;
    geode::TextInput* m_reasonInput;

    bool removePunishment;

    void doRemovePunish(){
        if (m_userInput->getString().empty()) return;
        auto req = web::WebRequest();
        this->retain();
        this->removeFromParent();
        auto url = fmt::format("{}/moderation/clear_punishments",SERVER_URL);
        req.header("Authorization", Mod::get()->getSavedValue<std::string>("token"));
        req.header("mod-version",MOD_VERSION_HEADER);
        req.param("type",m_type);
        req.param("user",m_userInput->getString());
        this->m_requestListener.spawn(req.post(url),[this](web::WebResponse res){
            if (!res.ok()) {
                auto json = res.json().unwrapOrDefault();
                if (json.contains("err")){
                    auto errorText = json["err"]["text"].asString().unwrapOr("Unknown");
                    auto notif = geode::Notification::create(fmt::format("Clear failed: {}",errorText),NotificationIcon::Error);
                    notif->show();
                }
            } else {
                auto notif = geode::Notification::create("Successfully cleared punishments!",NotificationIcon::Success);
                notif->show();
            }
            this->release();
        });
    }

    void doPunish(){
        if (m_userInput->getString().empty()) return;
        if (m_lengthInput->getString().empty() && m_reasonInput->getString().empty()) return;
        auto req = web::WebRequest();
        this->retain();
        this->removeFromParent();
        auto url = fmt::format("{}/moderation/punishment",SERVER_URL);
        req.header("Authorization", Mod::get()->getSavedValue<std::string>("token"));
        req.header("mod-version",MOD_VERSION_HEADER);
        req.param("type",m_type);
        req.param("user",m_userInput->getString());
        if (m_lengthInput->getString().empty()) {
            req.param("length","-1");
        } else {
            req.param("length",m_lengthInput->getString());
        }
        if (m_reasonInput->getString().empty()) {
            req.param("reason","No reason provided.");
        } else {
            req.param("reason",m_reasonInput->getString());
        }
        this->m_requestListener.spawn(req.post(url),[this](web::WebResponse res){
            if (!res.ok()) {
                auto json = res.json().unwrapOrDefault();
                if (json.contains("err")){
                    auto errorText = json["err"]["text"].asString().unwrapOr("Unknown");
                    auto notif = geode::Notification::create(fmt::format("Punishment failed: {}",errorText),NotificationIcon::Error);
                    notif->show();
                }
            } else {
                auto notif = geode::Notification::create("Successfully punished!",NotificationIcon::Success);
                notif->show();
            }
            this->release();
        });
    }
    bool init() {
        if (!Popup::init(270.f, 200.f))
            return false;

        this->setTitle(fmt::format("New {}",m_type));

        this->m_userInput = TextInput::create(250.f,"Enter the user ID/name here..","chatFont.fnt");

        this->m_userInput->setPosition({this->m_mainLayer->getContentWidth()/2,140.f});

        this->m_userInput->setString(m_user);

        this->m_lengthInput = TextInput::create(250.f,"Enter the length here.. (e.g - 2d 5h)","chatFont.fnt");

        this->m_lengthInput->setPosition({this->m_mainLayer->getContentWidth()/2,this->m_mainLayer->getContentHeight()/2.f});

        this->m_reasonInput = TextInput::create(250.f, "Enter the reason here.. (Optional)","chatFont.fnt");
        this->m_reasonInput->setPosition({this->m_mainLayer->getContentWidth()/2,60.f});
        this->m_reasonInput->setMaxCharCount(200);
        this->m_reasonInput->setString(m_reason);

        auto button = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Punish"), [this](auto){
            if (removePunishment) {
                this->doRemovePunish();
            } else {
                this->doPunish();
            }
        });
        button->setPosition({this->m_mainLayer->getContentWidth()/2,25.f});
        button->m_pNormalImage->setScale(0.6f);

        auto toggle = CCMenuItemExt::createToggler(CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"), CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"), [this](CCMenuItemToggler* self){
            this->removePunishment = !self->isToggled();
            this->m_lengthInput->setVisible(!this->removePunishment);
            this->m_reasonInput->setVisible(!this->removePunishment);
            
            if (this->removePunishment) {
                this->setTitle(fmt::format("Clear {}s",m_type));
            } else {
                this->setTitle(fmt::format("New {}",m_type));
            }
        });
        toggle->setPosition({240,175});

        this->m_buttonMenu->addChild(toggle);

        this->m_mainLayer->addChild(m_reasonInput);
        this->m_buttonMenu->addChild(button);
        
        this->m_mainLayer->addChild(m_lengthInput);
        
        this->m_mainLayer->addChild(m_userInput);

        return true;
    }

public:
    static ReplyPunishUserLayer* create(const std::string& type) {
        auto ret = new ReplyPunishUserLayer();
        ret->m_type = type;
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
    static ReplyPunishUserLayer* create(const std::string& type, const std::string& user, const std::string& reason) {
        auto ret = new ReplyPunishUserLayer();
        ret->m_type = type;
        ret->m_user = user;
        ret->m_reason = reason;
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};