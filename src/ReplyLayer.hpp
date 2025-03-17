#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include "Geode/ui/TextInput.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ReplyLayer : public geode::Popup<std::string const&> {
    std::string m_commentID;
    geode::TextInput* m_replyTextInput;
    CCMenuItemSpriteExtra* m_uploadBtn;
    bool setup(std::string const& commentID) override;
    public:
    void show() override; 
    void onClose(CCObject*sender) override;
    static ReplyLayer* create(std::string commentID);
};