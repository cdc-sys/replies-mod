#include "Auth.hpp"
#include "Geode/binding/GJAccountManager.hpp"
#include "Geode/utils/web.hpp"

class MyUploadDelegate : public UploadMessageDelegate {
    Auth* m_auth;
    void uploadMessageFailed(int p0) override {
        m_auth->m_loading->fadeOut();
        FLAlertLayer::create("Oops!", "Sending message failed due to an unknown error, please try again later.", "OK")->show();
    }
    void uploadMessageFinished(int p0) override {
        m_auth->m_loading->changeStatus("Authenticating [3/3]");
        m_auth->step3();
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

void Auth::handleError(web::WebResponse* res){
    auto json = res->json().unwrapOrDefault();
    if (json.contains("err")){
        auto errorText = json["err"]["text"].asString().unwrapOr("Unknown");
        auto alert = FLAlertLayer::create("Uh Oh!",fmt::format("<cr>Auth failed: {}</c>",errorText).c_str(),"OK");
        alert->show();
        m_loading->fadeOut();
    } else {
        auto alert = FLAlertLayer::create("Uh Oh!",fmt::format("<cr>Auth failed: {}</c>",res->string().unwrapOr("Unknown")).c_str(),"OK");
        alert->show();
        m_loading->fadeOut();
    }
    this->release();
}

void Auth::step3(){
    auto req = web::WebRequest();
    m_webListener.bind([this](web::WebTask::Event* e){
        if (auto res = e->getValue()){
            if (res->ok()) {
                // success
                auto json = res->json().unwrapOrDefault();
                if (json.contains("token")){
                    Mod::get()->setSavedValue("token", json["token"].asString().unwrap());
                }
                this->release();
            } else {
                this->handleError(res);
            }
        }
    });
    m_webTask = req.post("http://localhost:6650/auth/validate");
    m_webListener.setFilter(m_webTask);
}

void Auth::step2(const char* code){
    auto GLM = GameLevelManager::get();
	GLM->m_uploadMessageDelegate = MyUploadDelegate::create(this);
	GLM->uploadUserMessage(28167925,code,"This is an authentication message for the \"Replies\" mod.\nIf you see this, please delete it!");
}

void Auth::step1(){
    auto req = web::WebRequest();
    m_webListener.bind([this](web::WebTask::Event* e){
        if (auto res = e->getValue()){
            if (res->ok()) {
                // success
                auto json = res->json().unwrapOrDefault();
                if (json.contains("code")){
                    m_loading->changeStatus("Authenticating [2/3]");
                    step2(json["code"].asString().unwrap().c_str());
                }
            } else {
                this->handleError(res);
            }
        }
    });
    req.param("id",GJAccountManager::get()->m_accountID);
    m_webTask = req.get("http://localhost:6650/auth/get_code");
    m_webListener.setFilter(m_webTask);
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

Auth* Auth::create(){
    Auth* ret = new Auth();
    if (ret){
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}