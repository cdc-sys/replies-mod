#include "Auth.hpp"
#include "Geode/binding/GJAccountManager.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/web.hpp"
#include "Geode/modify/MenuLayer.hpp"
#include <argon/argon.hpp>

class MyUploadDelegate : public CommentUploadDelegate {
    Auth* m_auth;
    void commentUploadFailed(int p0, CommentError err) override {
        m_auth->m_loading->fadeOut();
        FLAlertLayer::create("Oops!", "Sending message failed due to an unknown error, please try again later.", "OK")->show();
    }
    void commentUploadFinished(int p0) override {
        m_auth->m_loading->changeStatus("Authenticating [3/3]");
        m_auth->step3();
        geode::log::info("{}",p0);
    }
    public:
    static MyUploadDelegate* create(Auth* auth) {
        MyUploadDelegate* ret = new MyUploadDelegate();
        ret->m_auth = auth;
        if (ret) {
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

void Auth::handleError(web::WebResponse res){
    auto json = res.json().unwrapOrDefault();
    if (json.contains("err")){
        auto errorText = json["err"]["text"].asString().unwrapOr("Unknown");
        auto alert = FLAlertLayer::create("Uh Oh!",fmt::format("<cr>Auth failed: {}</c>",errorText).c_str(),"OK");
        alert->show();
        m_loading->fadeOut();
    } else {
        auto alert = FLAlertLayer::create("Uh Oh!",fmt::format("<cr>Auth failed: {}</c>",res.string().unwrapOr("Unknown")).c_str(),"OK");
        alert->show();
        m_loading->fadeOut();
    }
    this->release();
}
int whichIcon() {
    GameManager* gm = GameManager::get();
    switch (gm->m_playerIconType) {
        case IconType::Ship:
            return gm->m_playerShip.value();
        case IconType::Ball:
            return gm->m_playerBall.value();
        case IconType::Ufo:
            return gm->m_playerBird.value();
        case IconType::Wave:
            return gm->m_playerDart.value();
        case IconType::Robot:
            return gm->m_playerRobot.value();
        case IconType::Spider:
            return gm->m_playerSpider.value();
        case IconType::Swing:
            return gm->m_playerSwing.value();
        case IconType::Jetpack:
            return gm->m_playerJetpack.value();
        default:
            return gm->m_playerFrame.value();
    }
}
void Auth::send_icons(){
    auto req = web::WebRequest();
    auto GM = GameManager::get();
    req.param("id",whichIcon());
    req.param("type",(int)GM->m_playerIconType);
    req.param("c1",GM->getPlayerColor());
    req.param("c2",GM->getPlayerColor2());
    req.param("c3",GM->getPlayerGlowColor());
    req.param("glow",(GM->getPlayerGlow() ? "true" : "false"));
    req.header("Authorization", Mod::get()->getSavedValue<std::string>("token"));
    req.header("mod-version",MOD_VERSION_HEADER);
    auto url = fmt::format("{}/update_icons",SERVER_URL);
    m_webListener.spawn(req.post(url),[this](web::WebResponse res){
        if (res.ok()) {
            this->release();
        } else {
            //this->handleError(res);
            this->release();
        }
    });
}

void Auth::step2(std::string& code){
    auto req = web::WebRequest();
    req.param("id",GJAccountManager::get()->m_accountID);
    req.param("user_id",GameManager::get()->m_playerUserID.value());
    req.param("username",GJAccountManager::get()->m_username);
    req.param("token",code);
    req.header("mod-version",MOD_VERSION_HEADER);
    auto url = fmt::format("{}/auth/validate",SERVER_URL);
    m_webListener.spawn(req.post(url),[this](web::WebResponse res){
        if (res.ok()) {
            // success
            auto json = res.json().unwrapOrDefault();
            if (json.contains("token")){
                Mod::get()->setSavedValue("token", json["token"].asString().unwrap());
                this->m_loading->fadeOut();
                this->m_rl->addReplyUI();
            }
            if (json["send_icons"].asBool().unwrapOr(false)){
                this->send_icons();
            } else this->release();
        } else {
            this->handleError(res);
        }
    });
}

void Auth::step1(){
    m_listener.spawn(
            argon::startAuth(),
            [this](Result<std::string> result) {
                if (result.isOk()) {
                    auto token = std::move(result).unwrap();
                    this->step2(token);
                } else {
                    log::warn("Failed to authenticate: {}", result.unwrapErr());
                }
            }
        );
}

void Auth::start(){
    // step 1. GET request https://{}/auth/get_code
    // step 2. send message
    // step 3. POST request https://{}/auth/validate
    this->retain();
    m_loading = LoadingOverlay::create();
    m_loading->changeStatus("Authenticating [1/3]");
    m_loading->show();
    step1();
}

Auth* Auth::create(ReplyLayer* rl){
    Auth* ret = new Auth();
    ret->m_rl = rl;
    if (ret){
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

class $modify(MyMenuLayer,MenuLayer){
    struct Fields{
        TaskHolder<web::WebResponse> m_webListener;
    };
    bool init(){
        if (!MenuLayer::init()) return false;

        if (g_syncedIcons) return true;
        if (Mod::get()->getSavedValue<std::string>("token").empty()) return true;

        g_syncedIcons = true;
        auto req = web::WebRequest();
        req.header("Authorization",Mod::get()->getSavedValue<std::string>("token"));
        req.header("mod-version",MOD_VERSION_HEADER);
        auto url = fmt::format("{}/auth/test",SERVER_URL);
        this->m_fields->m_webListener.spawn(req.post(url),[this](web::WebResponse res){
            if (!res.ok()){
                geode::log::error("{}",res.string().unwrap());
                auto notif = geode::Notification::create("[Replies] Unauthorized.",NotificationIcon::Error);
                notif->show();
                Mod::get()->setSavedValue<std::string>("token","");
            } else {
                // kinda evil but hey i already made the func
                auto auth = Auth::create(nullptr);
                auth->retain();
                auth->send_icons();
                auto json = res.json().unwrapOrDefault();
                auto modPerms = json["mod_permission_level"].asInt().unwrapOr(0);
                g_permissions = (ModerationPermissions)modPerms;
                Mod::get()->setSavedValue<int64_t>("moderation_permissions", modPerms);
                geode::log::info("mod perm level: {} ENUM: {}",modPerms,(int)g_permissions);
            }
        });
        return true;
    }
};