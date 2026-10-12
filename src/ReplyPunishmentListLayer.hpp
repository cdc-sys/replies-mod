#pragma once
#include "Structs.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

class ReplyPunishmentListLayer : public RepliesBasePaginatedLayer {
    CCMenuItemSpriteExtra* m_uploadBtn;
    CCMenuItemSpriteExtra* m_prevBtn;
    CCMenuItemSpriteExtra* m_nextBtn;
    CCLabelBMFont* pageLabel;
    CCMenuItemSpriteExtra* reloadBtn;
    geode::ScrollLayer* m_scrollLayer;
    TaskHolder<web::WebResponse> m_webListener;

    bool init();
    void populate(std::vector<Punishment> const& punishments,std::string const& message="");
    void onReload(CCObject* sender);

    public:

    bool m_showClosed;

    int m_page=1;
    int m_maxPages=1;
    int m_totalReplies=0;
    
    Mode m_displayMode=Mode::CompactCells;
    bool _m_darker;

    void show() override; 
    void loadReplies(bool force) override;

    static ReplyPunishmentListLayer* create();
};