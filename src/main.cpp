#include "Geode/cocos/menu_nodes/CCMenuItem.h"
#include <Geode/Geode.hpp>
#include "ReplyLayer.hpp"
#include "ReplyHistoryLayer.hpp"
#include "ReplyReportListLayer.hpp"
#include "ReplyNotificationListLayer.hpp"

using namespace geode::prelude;

#include <Geode/modify/ProfilePage.hpp>
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

class $modify(MyProfilePage,ProfilePage) {
	struct Fields {
		TaskHolder<web::WebResponse> m_webListener;
		CCMenuItemSpriteExtra* m_notifButton;
	};

	void loadNotifCount(){
		auto req = web::WebRequest();
        req.header("Authorization", Mod::get()->getSavedValue<std::string>("token"));
        req.header("mod-version",MOD_VERSION_HEADER);
        auto url = fmt::format("{}/notification/count",SERVER_URL);
        this->m_fields->m_webListener.spawn(req.get(url),[this](web::WebResponse res){
			if (res.ok()) {
				auto json = res.json().unwrapOrDefault();
				auto count = geode::utils::numFromString<int>(json["count"].asString().unwrapOr("0")).unwrapOr(0);
				if (count > 0) {
					auto node = CCNode::create();
					auto circle = CCSprite::create("circle.png");
					circle->setScale(1.5f);
					circle->setColor({255,0,0});
					auto text = fmt::format("{}",count);
					if (count > 9) text = "9+";
					auto label = CCLabelBMFont::create(text.c_str(),"bigFont.fnt");
					label->setScale(0.3f);
					node->addChild(circle);
					node->addChild(label);
					node->setZOrder(999);
					node->setPosition(this->m_fields->m_notifButton->getContentSize()*0.8f);
					this->m_fields->m_notifButton->addChild(node);
				}
			}
		});
	}

	bool init(int accountID, bool ownProfile) {
		if (!ProfilePage::init(accountID, ownProfile)) return false;
		
		auto myFuckassButtonSpr = CCSprite::createWithSpriteFrameName("GJ_undoBtn_001.png");
		myFuckassButtonSpr->setScale(30.f/myFuckassButtonSpr->getContentWidth());
		auto myFuckassButton = CCMenuItemExt::createSpriteExtra(myFuckassButtonSpr, [accountID](auto){
			ReplyHistoryLayer::create(accountID)->show();
		});

		auto myFuckassModButtonSpr = CCSprite::createWithSpriteFrameName("GJ_reportBtn_001.png");
		myFuckassModButtonSpr->setScale(30.f/myFuckassModButtonSpr->getContentWidth());
		auto myFuckassModButton = CCMenuItemExt::createSpriteExtra(myFuckassModButtonSpr, [accountID](auto){
			ReplyReportListLayer::create()->show();
		});

		auto myFuckassNotificationButtonSpr = CCSprite::createWithSpriteFrameName("GJ_starBtn_001.png");
		myFuckassNotificationButtonSpr->setScale(30.f/myFuckassNotificationButtonSpr->getContentWidth());
		this->m_fields->m_notifButton = CCMenuItemExt::createSpriteExtra(myFuckassNotificationButtonSpr, [accountID](auto){
			ReplyNotificationListLayer::create()->show();
		});

		this->getChildByIDRecursive("left-menu")->addChild(myFuckassButton);
		if (ownProfile && g_permissions >= ModerationPermissions::Moderator) this->getChildByIDRecursive("left-menu")->addChild(myFuckassModButton);
		if (ownProfile) this->getChildByIDRecursive("left-menu")->addChild(this->m_fields->m_notifButton);
		this->getChildByIDRecursive("left-menu")->updateLayout();

		this->loadNotifCount();

		return true;
	}
};