#pragma once
#include <Geode/Geode.hpp>
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include <Geode/utils/web.hpp>
#include "Geode/utils/web.hpp"
#include "Structs.hpp"

using namespace geode::prelude;

class NotificationCell : public CCNode {
    ReplyBackgroundColor m_bgColor;
    RepliesBasePaginatedLayer* m_rl;
    TaskHolder<web::WebResponse> m_webListener;
    bool init(); 
    public:
    RepliesNotification m_notif;
    static NotificationCell* create(RepliesBasePaginatedLayer* rl,RepliesNotification notif,ReplyBackgroundColor bgColor=ReplyBackgroundColor::Regular);
};
