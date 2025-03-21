#pragma once
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include "Geode/ui/ScrollLayer.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/web.hpp"
#include "Structs.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ReplyLayer : public geode::Popup<std::string const&> {
    std::string m_commentID;
    geode::TextInput* m_replyTextInput;
    CCMenuItemSpriteExtra* m_uploadBtn;
    CCLabelBMFont* authenticateLabel;
    CCMenuItemSpriteExtra* authenticateBtn;
    geode::ScrollLayer* m_scrollLayer;
    geode::EventListener<web::WebTask> m_webListener;
    int m_page=1;
    int m_maxPages=1;
    int m_totalReplies=0;
    GJComment* m_comment;
    Reply m_reply;
    bool setup(std::string const& commentID) override;
    void populate(std::vector<Reply> const& replies,std::string const& message="");
    void onUpload(CCObject* sender);
    void onAuthenticate(CCObject* sender);
    float iterate(Reply reply,int replyLevel,Reply parentReply={},int skip=0);
    public:
    bool _m_darker;
    void show() override; 
    void loadReplies();
    void addReplyUI();
    void onClose(CCObject*sender) override;
    static ReplyLayer* create(GJComment* comment);
    static ReplyLayer* create(Reply reply);
};