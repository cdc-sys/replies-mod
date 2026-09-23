#pragma once
#include "Geode/Enums.hpp"
#include <Geode/Geode.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <matjson.hpp>

using namespace geode::prelude;
struct IconData {
    int id=1;
    int type=0;
    int primaryColor=1;
    int secondaryColor=1;
    int glowColor=1;
    bool glow=false;
};
struct Reply {
    std::string content;
    int64_t author_id;
    std::string author_name;
    int64_t timestamp;
    std::string id;
    int64_t likes;
    int64_t reply_count;
    IconData icon;
    std::string comment_id;

    // misc
    bool from_comment=false;
    bool account_comment=false;
    std::string comment_timestamp;
    std::vector<Reply> replies;
    bool last=false;
    
    bool needs_web_fetch=false;

    Reply* parent=nullptr;
};

template <>
struct matjson::Serialize<IconData>
{
    static Result<IconData> fromJson(matjson::Value const &value)
    {
        IconData icon = IconData();
        icon.id = value["id"].asInt().unwrapOr(0);
        icon.type = value["type"].asInt().unwrapOr(0);
        icon.primaryColor = value["primary_color"].asInt().unwrapOr(0);
        icon.secondaryColor = value["secondary_color"].asInt().unwrapOr(0);
        icon.glowColor = value["glow_color"].asInt().unwrapOr(0);
        icon.glow = value["glow"].asBool().unwrapOr(false);
        return Ok(icon);
    }
    static matjson::Value toJson(IconData const &value){
        auto obj = matjson::Value();
        obj["id"] = value.id;
        obj["type"] = value.type;
        obj["primary_color"] = value.primaryColor;
        obj["secondary_color"] = value.secondaryColor;
        obj["glow_color"] = value.glowColor;
        obj["glow"] = value.glow;
        return obj;
    }
};

template <>
struct matjson::Serialize<Reply>
{
    static Result<Reply> fromJson(matjson::Value const &value)
    {
        Reply reply = Reply();
        reply.content = value["content"].asString().unwrapOr("");
        reply.author_id = geode::utils::numFromString<int64_t>(value["author_id"].asString().unwrapOr("0")).unwrapOr(0);
        reply.author_name = value["author_name"].asString().unwrapOr("");
        reply.timestamp = geode::utils::numFromString<int64_t>(value["timestamp"].asString().unwrapOr("0")).unwrapOr(0);
        reply.comment_id = value["comment_id"].asString().unwrapOr("");
        reply.id = value["id"].asString().unwrapOr("");
        reply.likes = value["likes"].asInt().unwrapOr(0);
        reply.reply_count = value["reply_count"].asInt().unwrapOr(0);
        if (value.contains("icon")){
            reply.icon = value["icon"].as<IconData>().unwrapOrDefault();
        }
        reply.replies = value["replies"].as<std::vector<Reply>>().unwrapOrDefault();
        return Ok(reply);
    }
    static matjson::Value toJson(Reply const &value){
        auto obj = matjson::Value();
        obj["content"] = value.content;
        obj["author_id"] = value.author_id;
        obj["author_name"] = value.author_name;
        obj["timestamp"] = value.timestamp;
        obj["comment_id"] = value.comment_id;
        obj["id"] = value.id;
        obj["likes"] = value.likes;
        obj["reply_count"] = value.reply_count;
        if (value.icon.id != 0){
            obj["icon"] = matjson::Value(value.icon);
        }
        return obj;
    }
};

inline Reply replyFromComment(GJComment* comment,int replies=0){
    Reply reply = Reply();
    reply.from_comment = true;

    reply.content = comment->m_commentString;
    reply.author_id = comment->m_accountID;
    reply.author_name = comment->m_userName;
    reply.comment_timestamp = comment->m_uploadDate;
    reply.id = fmt::format("{}",comment->m_commentID);
    reply.likes = comment->m_likeCount;
    reply.reply_count = replies;

    if (!comment->m_userScore) {
        reply.account_comment = true;
        return reply;
    }

    reply.icon.type = (int)comment->m_userScore->m_iconType;
    reply.icon.id = comment->m_userScore->m_iconID;
    reply.icon.primaryColor = comment->m_userScore->m_color1;
    reply.icon.secondaryColor = comment->m_userScore->m_color2;
    reply.icon.glowColor = comment->m_userScore->m_color2;
    reply.icon.glow = comment->m_userScore->m_special == 2;

    return reply;
}

inline std::string toAgoString(int timestamp) {
    auto const fmtPlural = [](auto count, auto unit) {
        if (count == 1) {
            return fmt::format("{} {} ago", count, unit);
        }
        return fmt::format("{} {}s ago", count, unit);
    };
    auto value = std::chrono::seconds(timestamp);
    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
    auto len = std::chrono::duration_cast<std::chrono::seconds>(now - value).count();
    if (len <= 0){
        return fmt::format("0 seconds ago");
    }
    if (len < 60) {
        return fmtPlural(len, "second");
    }
    len = std::chrono::duration_cast<std::chrono::minutes>(now - value).count();
    if (len < 60) {
        return fmtPlural(len, "minute");
    }
    len = std::chrono::duration_cast<std::chrono::hours>(now - value).count();
    if (len < 24) {
        return fmtPlural(len, "hour");
    }
    len = std::chrono::duration_cast<std::chrono::days>(now - value).count();
    if (len < 31) {
        return fmtPlural(len, "day");
    }
    len = std::chrono::duration_cast<std::chrono::weeks>(now - value).count();
    if (len < 4) {
        return fmtPlural(len, "week");
    }
    len = std::chrono::duration_cast<std::chrono::months>(now - value).count();
    if (len < 12) {
        return fmtPlural(len, "month");
    }
    len = std::chrono::duration_cast<std::chrono::years>(now - value).count();
    if (len >= 1) {
        return fmtPlural(len, "year");
    }
    return fmt::format("this is the secret string");
}
// https://replies.cdc-sys.com
static constexpr const std::string_view SERVER_URL = "http://localhost:6650";
static const std::string MOD_VERSION_HEADER = Mod::get()->getVersion().toVString();

struct CacheEntry {
    std::vector<Reply> replies;
    std::chrono::seconds time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
};

struct ReplyCache {
    int page=1;
    int max_pages=1;
    int total_replies=0;
    std::string message;
    std::map<int,CacheEntry> cached;
    std::chrono::seconds time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
};

static std::map<std::string,ReplyCache> g_replyCache = {};
static std::vector<std::string> g_votedOn = {};
#define VECTOR_HAS_ITEM(vec, item) std::find(vec.begin(), vec.end(), item) != vec.end()

static bool g_syncedIcons = false;

enum class ModerationPermissions {
    None = 0,
    Moderator = 1,
    Administrator = 2,
    FullAccess = 3
};

enum class Mode {
    LargeCells,
    CompactCells
};
class RepliesBasePaginatedLayer : public geode::Popup {
    public:
    void loadReplies(bool force) {};
    Mode m_displayMode=Mode::CompactCells;
    int m_page=1;
    int m_maxPages=1;
    int m_totalReplies=0;
};

static ModerationPermissions g_permissions = (ModerationPermissions)Mod::get()->getSavedValue<int64_t>("moderation_permissions");