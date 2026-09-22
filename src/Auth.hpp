#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include "LoadingOverlay.hpp"
#include "ReplyLayer.hpp"

using namespace geode::prelude;

class Auth : public cocos2d::CCObject {
    TaskHolder<web::WebResponse> m_webListener;
    async::TaskHolder<Result<std::string>> m_listener;
    ReplyLayer* m_rl;
    void step1();
    void step2(std::string& code);
    void handleError(web::WebResponse res);
    void handleError(std::string error);
    public:
    LoadingOverlay* m_loading;
    void start();
    void send_icons();
    static Auth* create(ReplyLayer* rl);
};