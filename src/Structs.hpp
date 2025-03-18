#pragma once
#include "Geode/Enums.hpp"
#include <Geode/Geode.hpp>
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
        reply.author_id = value["author_id"].asInt().unwrapOr(0);
        reply.author_name = value["author_name"].asString().unwrapOr("");
        reply.timestamp = value["timestamp"].asInt().unwrapOr(0);
        reply.id = value["id"].asString().unwrapOr("");
        reply.likes = value["likes"].asInt().unwrapOr(0);
        reply.reply_count = value["like_count"].asInt().unwrapOr(0);
        if (value.contains("icon")){
            reply.icon = value["icon"].as<IconData>().unwrapOrDefault();
        }
        return Ok(reply);
    }
    static matjson::Value toJson(Reply const &value){
        auto obj = matjson::Value();
        obj["content"] = value.content;
        obj["author_id"] = value.author_id;
        obj["author_name"] = value.author_name;
        obj["timestamp"] = value.timestamp;
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
    reply.content = comment->m_commentString;
    reply.author_id = comment->m_accountID;
    reply.author_name = comment->m_userName;
    reply.timestamp = 0;
    reply.id = comment->m_commentID;
    reply.likes = comment->m_likeCount;
    reply.reply_count = replies;
    if (!comment->m_userScore) return reply;
    reply.icon.type = (int)comment->m_userScore->m_iconType;
    reply.icon.id = comment->m_userScore->m_iconID;
    reply.icon.primaryColor = comment->m_userScore->m_color1;
    reply.icon.secondaryColor = comment->m_userScore->m_color2;
    reply.icon.glowColor = comment->m_userScore->m_color3;
    reply.icon.glow = comment->m_userScore->m_glowEnabled;
    return reply;
}
