#include "Geode/cocos/menu_nodes/CCMenuItem.h"
#include <Geode/Geode.hpp>
#include "ReplyLayer.hpp"

using namespace geode::prelude;

#include <Geode/modify/CommentCell.hpp>
class $modify(MyCommentCell, CommentCell) {
	struct Fields {
		bool replyButtonAdded = false;
	};
	static void onModify(auto& self) {
		(void) self.setHookPriority("CommentCell::loadFromComment", -4000); // using -3999 int prio because im doing stuff before AND after a function call and an enum would make it unclear. also -2123456789 prio on top
	}
	void loadFromComment(GJComment* comment){
		CommentCell::loadFromComment(comment);
		if (m_fields->replyButtonAdded) return;
		m_fields->replyButtonAdded = true;

		bool smallCommentsMode = this->m_height == 36;
		auto eryMenu = this->getChildByIDRecursive("raydeeux.variouscommenttweaks/the-menu-with-buttons-that-im-too-lazy-to-move-into-main-menu-sorry");
		auto dateLabel = this->m_mainLayer->getChildByID("date-label");

		if (!dateLabel) return;

		auto replyLabel = CCSprite::createWithSpriteFrameName("GJ_undoBtn_001.png");
		auto replyButton = CCMenuItemExt::createSpriteExtra(replyLabel, [comment](CCMenuItemSpriteExtra* btn){
			auto replyLayer = ReplyLayer::create(comment);
			replyLayer->show();
		});
		auto replyMenu = CCMenu::create();

		if (!smallCommentsMode){
			replyMenu->setPosition(320,30);
			replyLabel->setScale(.45f);
		}
		else{
			if (dateLabel) {
				replyMenu->setPosition(dateLabel->getPositionX()-dateLabel->getScaledContentWidth()-10.f,8);
				if (eryMenu) {
					geode::log::info("test");
					replyMenu->setPosition(eryMenu->getPositionX()-eryMenu->getScaledContentWidth()-5.f,8);
				}
				replyLabel->setScale(.30f);
			}
		}

		replyMenu->addChild(replyButton);
		this->m_mainLayer->addChild(replyMenu);
	}
};