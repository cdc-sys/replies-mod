#pragma once
#include "Geode/utils/web.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/modify/LikeItemLayer.hpp>
#include "ReplyCell.hpp"
#include "Structs.hpp"

using namespace geode::prelude;
class ReplySetRoleLayer : public geode::Popup {
protected:
    std::string m_user;

    TaskHolder<web::WebResponse> m_requestListener;
    int m_optionChecked = -1;

    geode::TextInput* m_userInput;

    int m_minRole = 0;
    int m_maxRole = (int)RepliesManager::get().modPermissions;

    int m_role = m_minRole;
    
    std::vector<std::string> m_roleNames = {
        "User",
        "Moderator",
        "Administrator",
        "Full Access"
    };

    CCLabelBMFont* m_roleSelectTitle;
    CCLabelBMFont* m_roleSelectRole;
    CCMenuItemSpriteExtra* m_roleSelectLeftArrow;
    CCMenuItemSpriteExtra* m_roleSelectRightArrow;

    void updateRoleSelectUI(){
        this->m_roleSelectRole->setString(m_roleNames[m_role].c_str());
        this->m_roleSelectRole->limitLabelWidth(100.f, 0.8f, 0.01f);
        this->m_roleSelectLeftArrow->setVisible(m_role > m_minRole);
        this->m_roleSelectRightArrow->setVisible(m_role < m_maxRole);
    }
    void addRoleSelectUI(){
        this->m_roleSelectTitle = CCLabelBMFont::create("Select Role:","goldFont.fnt");

        this->m_roleSelectRole = CCLabelBMFont::create(m_roleNames[m_role].c_str(),"bigFont.fnt");

        auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
        auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
        prevSprite->setScale(0.7f);
        nextSprite->setScale(0.7f);
        nextSprite->setFlipX(true);
        this->m_roleSelectLeftArrow = CCMenuItemExt::createSpriteExtra(prevSprite, [this](auto btn){
            this->m_role--;
            this->updateRoleSelectUI();
        });
        this->m_roleSelectRightArrow = CCMenuItemExt::createSpriteExtra(nextSprite, [this](auto btn){
            this->m_role++;
            this->updateRoleSelectUI();
        });

        this->m_roleSelectTitle->setPosition({this->m_mainLayer->getContentWidth()/2,110.f});
        this->m_roleSelectTitle->setScale(0.6f);

        this->m_roleSelectRole->setPosition({this->m_mainLayer->getContentWidth()/2,90.f});

        this->m_roleSelectLeftArrow->setPosition({this->m_mainLayer->getContentWidth()/2-75.f,90.f});
        this->m_roleSelectRightArrow->setPosition({this->m_mainLayer->getContentWidth()/2+75.f,90.f});

        this->m_mainLayer->addChild(this->m_roleSelectTitle);
        this->m_mainLayer->addChild(this->m_roleSelectRole);

        this->m_buttonMenu->addChild(this->m_roleSelectLeftArrow);
        this->m_buttonMenu->addChild(this->m_roleSelectRightArrow);

        this->updateRoleSelectUI();
    }
    void doSend(){
        if (m_userInput->getString().empty()) return;
        auto req = web::WebRequest();
        this->retain();
        this->removeFromParent();
        auto url = fmt::format("{}/moderation/set_role",SERVER_URL);
        req.header("Authorization", Mod::get()->getSavedValue<std::string>("token"));
        req.header("mod-version",MOD_VERSION_HEADER);
        req.param("role",std::to_string(m_role));
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

        this->setTitle("Set Role");

        this->m_userInput = TextInput::create(250.f,"Enter the user ID/name here..","chatFont.fnt");

        this->m_userInput->setPosition({this->m_mainLayer->getContentWidth()/2,140.f});

        this->m_userInput->setString(m_user);

        auto button = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Send"), [this](auto){
            this->doSend();
        });
        button->setPosition({this->m_mainLayer->getContentWidth()/2,25.f});
        button->m_pNormalImage->setScale(0.6f);

        this->m_buttonMenu->addChild(button);
        
        this->m_mainLayer->addChild(m_userInput);

        this->addRoleSelectUI();

        return true;
    }

public:
    static ReplySetRoleLayer* create() {
        auto ret = new ReplySetRoleLayer();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};