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
		TaskHolder<web::WebResponse> m_webListener;
		bool replyButtonAdded = false;
		CCMenu* m_replyInfoMenu;
	};
	static void onModify(auto& self) {
		(void) self.setHookPriority("CommentCell::loadFromComment", -4000); // using -3999 int prio because im doing stuff before AND after a function call and an enum would make it unclear. also -2123456789 prio on top
	}
	void addInfoUI(int64_t count, bool has_mod){
		auto menu = this->m_fields->m_replyInfoMenu;
		auto layout = geode::RowLayout::create();
		layout->setGap(10.f);
		layout->setAxisAlignment(AxisAlignment::End);
		layout->setAutoScale(false);
		menu->setLayout(layout);
		menu->setAnchorPoint({1.f,0.5f});
		menu->setScale(0.375f);

		if (has_mod) {
			auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
			auto checkBtn = CCMenuItemExt::createSpriteExtra(check, [](auto) {
				createQuickPopup("Information","This <cg>checkmark</c> signifies that the <cy>author of this comment</c> has the <co>Replies</c> mod and will be able to see your reply","OK",nullptr,[](auto,auto){});
			});
			menu->addChild(checkBtn);
		}

		if (count > 0) {
			auto label = CCLabelBMFont::create(fmt::format("{} Repl{}",count,(count == 1 ? "y" : "ies")).c_str(),"bigFont.fnt");
			label->setScale(0.75f);
			menu->addChild(label);
		}

		menu->updateLayout();
	}
	void loadInfo(){
		auto rm = &RepliesManager::get();
		if (rm->replyInfoCache.contains(this->m_comment->m_commentID)) {
			auto cached = rm->replyInfoCache[this->m_comment->m_commentID];
			addInfoUI(cached.count, cached.has_mod);
			return;
		}
		auto req = web::WebRequest();
        req.header("mod-version",MOD_VERSION_HEADER);
        auto url = fmt::format("{}/reply/{}/info",SERVER_URL,this->m_comment->m_commentID);
		req.param("author_id",this->m_comment->m_accountID);
        this->m_fields->m_webListener.spawn(req.get(url),[this](web::WebResponse res){
			if (res.ok()) {
				auto rm = &RepliesManager::get();
				auto json = res.json().unwrapOrDefault();
				auto count = geode::utils::numFromString<int64_t>(json["reply_count"].asString().unwrapOr("0")).unwrapOr(0);
				bool has_mod = json["has_mod"].asBool().unwrapOrDefault();
				this->addInfoUI(count, has_mod);
				rm->replyInfoCache[this->m_comment->m_commentID] = ReplyInfo{
					.count = count,
					.has_mod = has_mod
				};
			}
		});
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
		replyLabel->setScale(.30f);
		auto replyButton = CCMenuItemExt::createSpriteExtra(replyLabel, [comment](CCMenuItemSpriteExtra* btn){
			auto replyLayer = ReplyLayer::create(comment);
			replyLayer->show();
		});
		auto replyMenu = CCMenu::create();
		auto replyInfoMenu = CCMenu::create();
		this->m_fields->m_replyInfoMenu = replyInfoMenu;

		if (dateLabel) {
			replyInfoMenu->setPosition(dateLabel->getPositionX()-dateLabel->getScaledContentWidth()-20.f,dateLabel->getPositionY());
			replyMenu->setPosition(dateLabel->getPositionX()-dateLabel->getScaledContentWidth()-10.f,dateLabel->getPositionY());
			if (eryMenu) {
				geode::log::info("test");
				replyInfoMenu->setPosition(eryMenu->getPositionX()-eryMenu->getScaledContentWidth()-15.f,eryMenu->getPositionY());
				replyMenu->setPosition(eryMenu->getPositionX()-eryMenu->getScaledContentWidth()-5.f,eryMenu->getPositionY());
			}
		}

		replyMenu->addChild(replyButton);
		this->m_mainLayer->addChild(replyMenu);
		this->m_mainLayer->addChild(replyInfoMenu);

		if (Mod::get()->getSettingValue<bool>("reply-counts")) this->loadInfo();
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
		if (ownProfile && RepliesManager::get().modPermissions >= ModerationPermissions::Moderator) this->getChildByIDRecursive("left-menu")->addChild(myFuckassModButton);
		if (ownProfile && !Mod::get()->getSavedValue<std::string>("token").empty()) {
			this->getChildByIDRecursive("left-menu")->addChild(this->m_fields->m_notifButton);
			this->loadNotifCount();
		}
		this->getChildByIDRecursive("left-menu")->updateLayout();

		

		return true;
	}
};