#ifndef _FETCH_LIVE_STATUS__
#define _FETCH_LIVE_STATUS__
#pragma once


#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <string>
#include <workflow/HttpMessage.h>
#include <workflow/HttpUtil.h>
#include <workflow/WFTaskFactory.h>
#include <workflow/WFFacilities.h>
#include <regex>
#include <deque>
#include <assert.h>

#include <rapidjson/encodings.h>
#include <rapidjson/document.h>


/**
 * @brief 
 * {
    "code": 0,
    "msg": "ok",
    "message": "ok",
    "data": {
        "room_id": 904823,
        "short_id": 0,
        "uid": 11332884,
        "need_p2p": 0,
        "is_hidden": false,
        "is_locked": false,
        "is_portrait": false,
        "live_status": 1,
        "hidden_till": 0,
        "lock_till": 0,
        "encrypted": false,
        "pwd_verified": false,
        "live_time": 1692882666,
        "room_shield": 1,
        "is_sp": 0,
        "special_type": 0
    }
}
 * @arg /data/live_status 为1的时候开播 为0的时候没有开播
需要获取 live_time is_locked room_id 
 */


struct LiveHomeStatus {
    uint64_t live_time;
    char room_id [16];
    bool Hidden;
    bool Lock;
    bool live_status;
    bool encrypted;
};

// std::deque<LiveHomeStatus> liveroom_list;


#define REDIRECT_MAX    5
#define RETRY_MAX       2
/**
 * @brief 
 * 输入直播间号 使用api查询并获取data ，data经过json解析以后填充live_home_status
 * @param Liveaddr 输入直播间号 使用api 经行查询
 * @return LiveHomeStatus 返回直播间状态
 */
LiveHomeStatus  GetliveStatus(const char*  Liveaddr);



void Listening_liveroom_init();
#endif




