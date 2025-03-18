#include <Geode/Geode.hpp>
#include "Structs.hpp"

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
    Reply m_reply;
    ReplyBackgroundColor m_bgColor;
    int m_replyLevel;
    bool init() override;
    public:
    ReplySpriteType m_spriteType;
    static ReplyCell* create(Reply reply,ReplyBackgroundColor bgColor=Regular, int replyLevel=1, ReplySpriteType spriteType=Line);
};