#pragma once
#include "Structs.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;
class ReplyLayer : public RepliesBasePaginatedLayer {
    std::string m_commentID;
    geode::TextInput* m_replyTextInput;
    CCMenuItemSpriteExtra* m_uploadBtn;
    CCMenuItemSpriteExtra* m_prevBtn;
    CCMenuItemSpriteExtra* m_nextBtn;
    CCLabelBMFont* authenticateLabel;
    CCLabelBMFont* pageLabel;
    CCMenuItemSpriteExtra* authenticateBtn;
    CCMenuItemSpriteExtra* reloadBtn;
    geode::ScrollLayer* m_scrollLayer;
    TaskHolder<web::WebResponse> m_webListener;

    GJComment* m_comment;
    Reply m_reply;

    std::string sortMode="likes";

    bool init(std::string const& commentID);
    void populate(std::vector<Reply> const& replies,std::string const& message="");
    float iterate(Reply reply,int replyLevel,Reply parentReply={},int skip=0);
    void onUpload(CCObject* sender);
    void onReload(CCObject* sender);
    void onAuthenticate(CCObject* sender);
    void onUploadFailed(int code);

    public:

    int m_page=1;
    int m_maxPages=1;
    int m_totalReplies=0;
    
    Mode m_displayMode=Mode::CompactCells;
    bool _m_darker;

    void show() override; 
    void loadReplies(bool force);
    void addReplyUI();
    void onClose(CCObject*sender) override;

    static ReplyLayer* create(GJComment* comment);
    static ReplyLayer* create(Reply reply);
};