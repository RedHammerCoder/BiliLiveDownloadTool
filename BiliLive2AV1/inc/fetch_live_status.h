#ifndef _FETCH_LIVE_STATUS__
#define _FETCH_LIVE_STATUS__
#pragma once

#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <string>
// #include <workflow/HttpMessage.h>
// #include <workflow/HttpUtil.h>
// #include <workflow/WFTaskFactory.h>
// #include <workflow/WFFacilities.h>
#include <regex>
#include <deque>
#include <assert.h>
#include <utility>
#include "basic_class.h"
#include <vector>
#include <rapidjson/encodings.h>
#include <rapidjson/document.h>

#include <sys/types.h>
#include <dirent.h>
// #include "m4s2mp4.h"
// #include "m3u8fetch.h"



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
#if 0
class LivingRoomIndex;
class m3u8fetch;
struct LiveHomeStatus
{
    uint64_t live_time;
    char RoomId_chr[32] = {0};
    std::string RoomHostName; // used to fill host name  ; like "key725" "战鹰"，，parsed by json file
    uint64_t RoomId;
    bool Hidden;
    bool Lock;
    int live_status;
    bool encrypted;
    LivingRoomIndex *LivingRoomExt = nullptr;
    std::string GetM3u8Url();
    std::string GetM4sUrl(uint64_t m4s_id);
    std::string GetM4sContent(std::string header);
    std::string M4sUrl_mode;
    m3u8fetch *FetchM3u8Node;
};
// #endif
/**
 * @brief 用于保存多种线路信息
 * @name
 */
class LivingRoomIndex
{
public:
    std::string GeneratorUrl;
    std::string formate_name, Codec_name;
    std::string BaseUrl;
    std::string host;
    std::string ExtraUrl;
    // m3u8 Url = host+BaseUrl+extra
    // stream_ttl 是干嘛用的？有可能是用于更新m3u8 url
    /**
     * @brief 基于GeneratorUrl 更新多种url
     * formate_name Codec_name来源于选定的formate_name 以及 codec_name
     *
     * @return * void
     */
    // std::string Getm4sUrl();
};
#endif
// using Block = std::pair<void* >;
// using M4sMap = std::pair<std::string , std::pair<void*  , size_t > > ;
// struct M4SVideo{

// };
// extern std::vector<M4SVideo> m4slist;

/**
 * @brief 在新获取的http m3u8 新于当前类的m3u8时候 本类析构并且初始化当前类的M4s列表
 *
 *
 */
// class M3u8Index
// {
// public:
//     M3u8Index() = delete;
//     M3u8Index(std::string head);
//     std::string MediaSeq;
//     std::string HeaderUri;
//     void *UniBlock;
//     size_t len;
//     std::vector<std::string> M4SList;
// };

extern std::deque<LiveHomeStatus> liveroom_list;

#define REDIRECT_MAX 5
#define RETRY_MAX 2
/**
 * @brief
 * 输入直播间号 使用api查询并获取data ，data经过json解析以后填充live_home_status
 * @param Liveaddr 输入直播间号 使用api 经行查询
 * @return LiveHomeStatus 返回直播间状态
 */
void GetliveStatus(const char *Liveaddr);

extern std::deque<LiveHomeStatus> liveroom_list;
/**
 * @brief LivingRoomIndexAnalysis用于获取正在直播的livingroom的m3u8 文件
 */
void LivingRoomIndexAnalysis();
void Listening_liveroom_init();
void UpdateRoomListMsg();

int FetchHttpBody(const std::string uri ,const void** ptr , size_t *len);


size_t MergeChunkedBody(void *_body, size_t &_body_len);

template <int wait_time, typename EVENT>
class LoopExecEvent
{
    EVENT _task;
    int _timer;
    bool _execFlag;
    WFTimerTask *TASK;
    std::function<void(WFTimerTask*)>  _exec;


public:
    LoopExecEvent(EVENT task) :  _task(std::move(task)), TASK(nullptr), _execFlag(false)
    {
        // fprintf(stderr , "per %d ms do log init\n" , wait_time);
        _timer=wait_time;
        // TASK =  WFTaskFactory::create_timer_task(wait_time, exec);
        _exec = [&](WFTimerTask* task){ 
                    // fprintf(stderr , "per %d ms   EXEC \n" , wait_time);
            this->_task(); series_of(task)->push_back(WFTaskFactory::create_timer_task(_timer,_exec)); };
    }
    // void exec(WFTimerTask *task)
    // {
    //     // _task();
    //     series_of(task)->push_back(WFTaskFactory::create_timer_task(_timer,_exec));
    // }
    void start()
    {
        _execFlag = true;
        auto fg = WFTaskFactory::create_timer_task(_timer, _exec);
        fg->start();
    }
    // void stop() { _execFlag = false; }
    ~LoopExecEvent() = default;
};

#endif
