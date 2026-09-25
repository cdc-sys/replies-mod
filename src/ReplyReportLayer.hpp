#pragma once
#include "Geode/utils/web.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/modify/LikeItemLayer.hpp>
#include "ReplyCell.hpp"

using namespace geode::prelude;
class ReplyReportLayer : public geode::Popup {
protected:
    Reply reply;
    TaskHolder<web::WebResponse> m_requestListener;
    std::vector<std::string> m_reportOptions = {"Inappropriate Content","Hate Speech / Harassment","Spam","Scamming / Phishing","Threats","Other"};
    std::vector<CCMenuItemToggler*> m_checkboxes = {};
    int m_optionChecked = -1;

    geode::TextInput* m_otherInput;
    geode::TextInput* m_additionalNoteInput;

    void doReport(){
        if (m_optionChecked == -1) return;
        if (m_optionChecked == m_reportOptions.size()-1 && m_otherInput->getString().empty()) return;
        auto req = web::WebRequest();
        this->retain();
        this->removeFromParent();
        auto url = fmt::format("{}/reply/{}/report",SERVER_URL,this->reply.id);
        req.header("Authorization", Mod::get()->getSavedValue<std::string>("token"));
        req.header("mod-version",MOD_VERSION_HEADER);
        req.param("reason",m_reportOptions[m_optionChecked]);
        if (m_optionChecked == m_reportOptions.size()-1) {
            req.param("reason",m_otherInput->getString());
        }
        if (!m_additionalNoteInput->getString().empty()) {
            req.param("note",m_additionalNoteInput->getString());
        }
        this->m_requestListener.spawn(req.post(url),[this](web::WebResponse res){
            if (!res.ok()) {
                auto json = res.json().unwrapOrDefault();
                if (json.contains("err")){
                    auto errorText = json["err"]["text"].asString().unwrapOr("Unknown");
                    auto notif = geode::Notification::create(fmt::format("Report failed: {}",errorText),NotificationIcon::Error);
                    notif->show();
                }
            } else {
                auto notif = geode::Notification::create("Successfully reported!",NotificationIcon::Success);
                notif->show();
            }
            this->release();
        });
    }
    bool init() {
        if (!Popup::init(270.f, 270.f))
            return false;

        this->setTitle("Report Reply");

        auto menu = CCMenu::create();

        auto layout = geode::RowLayout::create();

        layout->setAxis(Axis::Column);
        layout->setAxisAlignment(AxisAlignment::End);
        //layout->setCrossAxisAlignment(AxisAlignment::Center);
        //layout->setCrossAxisLineAlignment(AxisAlignment::Center);
        layout->setGap(5.f);
        layout->setAxisReverse(true);
        layout->setAutoScale(true);

        menu->setLayout(layout);

        for (int i=0;i<m_reportOptions.size();i++) {
            auto option = m_reportOptions[i];

            auto optionContainer = CCMenu::create();
            auto optionLayout = geode::RowLayout::create();
            optionLayout->setAxis(Axis::Row);
            optionLayout->setAxisAlignment(AxisAlignment::Start);
            layout->setGap(5.f);
            layout->setAutoScale(true);

            auto optionOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
            auto optionOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");

            auto optionCheckbox = CCMenuItemExt::createToggler(optionOn, optionOff, [this,i](auto){
                this->m_optionChecked = i;
                for (auto checkbox : this->m_checkboxes) {
                    checkbox->toggle(false);
                }
            });
            auto optionLabel = geode::Label::create("bigFont.fnt");
            optionLabel->setText(option);
            optionLabel->setScale(.7f);
            optionLabel->setLimitLabelWidth(200.f,.7f,.3f);

            m_checkboxes.push_back(optionCheckbox);

            optionContainer->addChild(optionCheckbox);
            optionContainer->addChild(optionLabel);

            if (i == m_reportOptions.size()-1) {
                this->m_otherInput = geode::TextInput::create(200.f,"Type a reason here..","chatFont.fnt");
                this->m_otherInput->setTextAlign(TextInputAlign::Left);
                this->m_otherInput->setMaxCharCount(20);
                optionContainer->addChild(this->m_otherInput);
            }

            optionContainer->setLayout(optionLayout);
            optionContainer->updateLayout();

            menu->addChild(optionContainer);
        }

        menu->setPosition({this->m_mainLayer->getContentWidth()/2,this->m_mainLayer->getContentWidth()/1.5f});
        menu->setAnchorPoint({0.5,1});
        menu->setScale(0.425f);
        menu->setContentWidth(this->m_mainLayer->getContentWidth());
        
        menu->updateLayout();

        auto authorLabel = geode::Label::create("bigFont.fnt");
        auto contentLabel = geode::Label::create("chatFont.fnt");
        authorLabel->setText(reply.author_name);
        contentLabel->setText(reply.content);
        authorLabel->setPosition({this->m_mainLayer->getContentWidth()/2,this->m_mainLayer->getContentWidth()/1.25f});
        contentLabel->setPosition({this->m_mainLayer->getContentWidth()/2,this->m_mainLayer->getContentWidth()/1.4f});
        authorLabel->setScale(0.7f);
        contentLabel->setScale(0.7f);
        contentLabel->setLimitLabelWidth(200,0.7f,0.01f);

        this->m_mainLayer->addChild(authorLabel);
        this->m_mainLayer->addChild(contentLabel);

        this->m_additionalNoteInput = TextInput::create(250.f, "Enter the additional note here.. (Optional)","chatFont.fnt");
        this->m_additionalNoteInput->setPosition({this->m_mainLayer->getContentWidth()/2,this->m_mainLayer->getContentWidth()/5.f});
        this->m_additionalNoteInput->setMaxCharCount(200);

        auto button = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Report"), [this](auto){
            this->doReport();
        });
        button->setPosition({this->m_mainLayer->getContentWidth()/2,this->m_mainLayer->getContentWidth()/12.f});
        button->m_pNormalImage->setScale(0.6f);

        this->m_mainLayer->addChild(m_additionalNoteInput);
        this->m_buttonMenu->addChild(button);
        
        this->m_mainLayer->addChild(menu);

        return true;
    }

public:
    static ReplyReportLayer* create(Reply reply) {
        auto ret = new ReplyReportLayer();
        ret->reply = reply;
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};