#ifndef _M3U8FETCH_
#define _M3U8FETCH_
#pragma once
#include "fetch_live_status.h"
#include <map>
#include <string>
#include <utility>
#include <mutex>

/**
 * @brief 标准m3u8文件
#EXTM3U  
#EXT-X-VERSION:7
#EXT-X-START:TIME-OFFSET=0
#EXT-X-MEDIA-SEQUENCE:47187620
#EXT-X-TARGETDURATION:1
#EXT-X-MAP:URI="h1688140345.m4s"
#EXTINF:1.00,fe180|d39640bb
47187620.m4s
#EXTINF:1.00,f9ad2|e0b564e9
47187621.m4s
 * 
 */
using BLOCK  = std::pair<void* , size_t > ;




class m3u8fetch
{
private:
    /* data */
    std::mutex mtx_m4s;// if want to modify m4slist , the first thing is get this lock;
    std::map<uint64_t ,std::pair<void* , size_t>> m4slist;//uint64_t 保存id
    uint64_t min_m4s_nb;//保存最小的m4s id from m4slist   所有小于该数值的文件都已经写入存储
    uint64_t Max_m4s_nb;//保存最大的m4s id from m4slist;  确保获取的m3u8文件下序列号小于等于该数值
    void* EXT_X_MAP;
    char* EXT_X_MAP_NAME;
    LiveHomeStatus* _Parent;//用于获取当前的状态信息
    size_t EXT_X_MAP_len;
public:
    m3u8fetch(LiveHomeStatus* Parent);
    int updatem3u8list();//@todo : 更新m3u8文件列表
    void Parserm3u8(BLOCK );
    


    ~m3u8fetch();
};




#endif