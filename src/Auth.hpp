#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include "LoadingOverlay.hpp"

using namespace geode::prelude;

class Auth : public cocos2d::CCObject {
    EventListener<web::WebTask> m_webListener;
    web::WebTask m_webTask;
    void step1();
    void step2(const char* code);
    void handleError(web::WebResponse* res);
    public:
    LoadingOverlay* m_loading;
    void step3();
    void start();
    static Auth* create();
};