#pragma once
#include <Geode/Geode.hpp>
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include <Geode/utils/web.hpp>
#include "Geode/utils/web.hpp"
#include "Structs.hpp"

using namespace geode::prelude;

class PunishmentCell : public CCNode {
    ReplyBackgroundColor m_bgColor;
    RepliesBasePaginatedLayer* m_rl;
    TaskHolder<web::WebResponse> m_webListener;
    bool init(); 
    public:
    Punishment m_punishment;
    static PunishmentCell* create(RepliesBasePaginatedLayer* rl,Punishment punishment,ReplyBackgroundColor bgColor=ReplyBackgroundColor::Regular);
};
