#pragma once
#include "Structs.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

class ReplyNotificationListLayer : public RepliesBasePaginatedLayer {
    CCMenuItemSpriteExtra* m_uploadBtn;
    CCMenuItemSpriteExtra* m_prevBtn;
    CCMenuItemSpriteExtra* m_nextBtn;
    CCLabelBMFont* pageLabel;
    CCMenuItemSpriteExtra* reloadBtn;
    geode::ScrollLayer* m_scrollLayer;
    TaskHolder<web::WebResponse> m_webListener;

    bool init();
    void populate(std::vector<RepliesNotification> const& replies,std::string const& message="");
    void onReload(CCObject* sender);

    public:

    int m_page=1;
    int m_maxPages=1;
    int m_totalReplies=0;
    
    Mode m_displayMode=Mode::CompactCells;
    bool _m_darker;

    void show() override; 
    void loadReplies(bool force) override;

    static ReplyNotificationListLayer* create();
};