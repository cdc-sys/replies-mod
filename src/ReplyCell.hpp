#pragma once
#include <Geode/Geode.hpp>
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include <Geode/utils/web.hpp>
#include "Geode/utils/web.hpp"
#include "Structs.hpp"
#include "ReplyLayer.hpp"

using namespace geode::prelude;

enum ReplyBackgroundColor {
    Highlighted,
    Darker,
    Regular
};

enum ReplySpriteType {
    Line,
    Curl,
    LineCurl
};

class ReplyCell : public CCNode {
    ReplyBackgroundColor m_bgColor;
    ReplyLayer* m_rl;
    int m_replyLevel;
    int m_skipLines;
    CCSprite* likeSpr;
    CCLabelBMFont* likeLabel;
    EventListener<web::WebTask> m_webListener;
    void onReply(CCObject* sender);
    void onVote(CCObject* sender);
    void onDelete(CCObject* sender);
    void doDelete();
    bool init() override; 
    public:
    Reply m_reply;
    ReplySpriteType m_spriteType;
    void updateLikes(int likes);
    static ReplyCell* create(ReplyLayer* rl,Reply reply,ReplyBackgroundColor bgColor=Regular, int replyLevel=1, ReplySpriteType spriteType=Line,int skipLines=0);
};
