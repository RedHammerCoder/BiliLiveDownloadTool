
#pragma once
#include "fetch_live_status.h"
#include <map>
#include <string>
#include <utility>
#include <mutex>
#include "basic_class.h"
#include "KExecutor.h"
#include <string_view>
#include <deque>
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
// using BLOCK = std::pair<void *, size_t>;

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