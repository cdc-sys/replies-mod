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
    LineCurl,
    MoreReplies
};

class ReplyCell : public CCNode {
    ReplyBackgroundColor m_bgColor;
    ReplyLayer* m_rl;
    int m_replyLevel;
    int m_skipLines;
    int m_skipLinesRight;
    CCSprite* likeSpr;
    CCLabelBMFont* likeLabel;
    TaskHolder<web::WebResponse> m_webListener;
    void onReply(CCObject* sender);
    void onVote(CCObject* sender);
    void onDelete(CCObject* sender);
    void doDelete();
    void fetchContent();
    bool init(bool fromFetch=false); 
    public:
    Reply m_reply;
    ReplySpriteType m_spriteType;
    void updateLikes(int likes);
    static ReplyCell* create(ReplyLayer* rl,Reply reply,ReplyBackgroundColor bgColor=Regular, int replyLevel=1, ReplySpriteType spriteType=Line,int skipLines=0,int skipLinesRight=0);
};
