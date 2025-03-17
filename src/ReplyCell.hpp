#include <Geode/Geode.hpp>
#include "Structs.hpp"

using namespace geode::prelude;

enum ReplyBackgroundColor {
    Highlighted,
    Darker,
    Regular
};

class ReplyCell : public CCNode {
    Reply m_reply;
    ReplyBackgroundColor m_bgColor;
    int m_replyLevel;
    bool init() override;
    public:
    static ReplyCell* create(Reply reply,ReplyBackgroundColor bgColor=Regular, int replyLevel=1);
};