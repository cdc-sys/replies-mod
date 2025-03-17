#include "Geode/cocos/menu_nodes/CCMenuItem.h"
#include <Geode/Geode.hpp>
#include "ReplyLayer.hpp"

using namespace geode::prelude;

#include <Geode/modify/CommentCell.hpp>
class $modify(MyCommentCell, CommentCell) {
	void loadFromComment(GJComment* comment){
		CommentCell::loadFromComment(comment);
		bool smallCommentsMode = this->m_height == 36;
		auto replyLabel = CCSprite::createWithSpriteFrameName("GJ_undoBtn_001.png");
		auto replyButton = CCMenuItemExt::createSpriteExtra(replyLabel, [comment](CCMenuItemSpriteExtra* btn){
			auto replyLayer = ReplyLayer::create(fmt::format("{}",comment->m_commentID));
			replyLayer->show();
		});
		auto replyMenu = CCMenu::create(0);
		if (!smallCommentsMode){
			replyMenu->setPosition(320,30);
			replyLabel->setScale(.45f);
		}
		else{
			replyMenu->setPosition(297,8);
			replyLabel->setScale(.30f);
		}
		replyMenu->addChild(replyButton);
		this->addChild(replyMenu);
	}
};