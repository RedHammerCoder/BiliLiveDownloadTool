
#pragma once
#include "fetch_live_status.h"
#include <map>
#include <string>
#include <utility>
#include <mutex>
#include <string_view>
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
using BLOCK = std::pair<void *, size_t>;

class SymbleSplite
{
private:
public:
    std::string_view _Src_Temp;
    SymbleSplite(std::string_view sym) : _Src_Temp(sym)
    {
        _CurrentLine = 0;
    }
    void Reset(std::string_view _template, char chr = '\n')
    {
        _CurrentLine = 0;
        result_list.clear();
        _Src_Temp = _template;
        splitbychar(chr);
    }
    void splitbychar(char _chr = '\n');
    // size_t _sym_ptr;
    size_t GetLine();
    size_t _CurrentLine;
    std::string_view *GetNextView();
    std::vector<std::string_view> result_list;
};

class m3u8fetch
{
private:
    /* data */
    std::mutex mtx_m4s;                                    // if want to modify m4slist , the first thing is get this lock;
    std::map<uint64_t, std::pair<void *, size_t>> m4slist; // uint64_t 保存id
    uint64_t min_m4s_nb;                                   // 保存最小的m4s id from m4slist   所有小于该数值的文件都已经写入存储
    uint64_t Max_m4s_nb;                                   // 保存最大的m4s id from m4slist;  确保获取的m3u8文件下序列号小于等于该数值
    void *EXT_X_MAP;
    char *EXT_X_MAP_NAME;
    LiveHomeStatus *_Parent; // 用于获取当前的状态信息
    size_t EXT_X_MAP_len;
    WFTimerTask* FetchM3u8Task;
    const int Exec_time;

    struct
    {
        // todo : 用于保存最新的m3u8文件  使用文件名以及文件长度来避免 避免重复解析
        uint64_t SeqId;
        size_t FileSize;
        std::string headFile;

    } CurrentM3u8file;
    void TaskM3u8Fetch();

public:
    // void Getm3u8file();
    m3u8fetch(LiveHomeStatus *Parent);
    // int updatem3u8list();//@todo : 更新m3u8文件列表
    int Parserm3u8(char *, size_t); // 解析m3u8文件并且
    ~m3u8fetch() = default;
};
