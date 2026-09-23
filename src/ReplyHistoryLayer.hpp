#pragma once
#include "Structs.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

class ReplyHistoryLayer : public RepliesBasePaginatedLayer {
    int m_accountID;
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

    bool init();
    void populate(std::vector<Reply> const& replies,std::string const& message="");
    void onReload(CCObject* sender);

    public:

    int m_page=1;
    int m_maxPages=1;
    int m_totalReplies=0;
    
    Mode m_displayMode=Mode::CompactCells;
    bool _m_darker;

    void show() override; 
    void loadReplies(bool force);

    static ReplyHistoryLayer* create(int accountID);
};