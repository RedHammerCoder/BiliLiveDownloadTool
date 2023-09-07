#include "m3u8fetch.h"
#include <string_view>
#include <string.h>
#include <sstream>

#include <iostream>

constexpr size_t unusedLen = strlen("#EXTM3U\n#EXT-X-VERSION:7\n#EXT-X-START:TIME-OFFSET=0\n");
constexpr int TargDurLen = strlen("#EXT-X-TARGETDURATION");
constexpr int MapUrILen = strlen("#EXT-X-MAP:URI");
constexpr int EXTINF = strlen("#EXTINF:");

m3u8fetch::m3u8fetch(LiveHomeStatus *parent) : _Parent(parent) {}

int m3u8fetch::Parserm3u8(char *ptr, size_t len)
{
    std::string line;
    std::stringstream ss;
    char *charptr = ptr;
    printf("BUFF is \n%s  \n", charptr);
    fflush(stdout);
    assert(strncmp(charptr, "#EXTM3U", strlen("#EXTM3U")) == 0);
    charptr = charptr + unusedLen;
    ss << std::string(charptr);
    // printf("BUFF is \n%s  \n",charptr);
    ss >> line;
    uint64_t SeqId = 0;
    sscanf(line.c_str(), "#EXT-X-MEDIA-SEQUENCE:%lld", &SeqId);
    #if 1
    std::cout << "\n seq  id is " << SeqId << std::endl;
    
    goto UpdateM4slist;
    #endif
    if (SeqId < CurrentM3u8file.SeqId)
        return -1;
    if (SeqId == CurrentM3u8file.SeqId)
    { // 检查文件长度来判断是否需要解析更新map
        if (len > CurrentM3u8file.FileSize)
            CurrentM3u8file.FileSize = len;
    }
    if (SeqId > CurrentM3u8file.SeqId)
    {
        CurrentM3u8file.SeqId=SeqId;
        CurrentM3u8file.FileSize=len;
        goto UpdateM4slist;
    }
    /**
     * @todo 循环处理  查找所有m4s文件并获取int数值
     * 
     */
UpdateM4slist:
    uint64_t m4sId=0;
    while(ss>>line)
    {
        std::string_view sbstr(line);
        std::cout<<"line is "<<sbstr<<std::endl;

        if(sbstr.substr(0,EXTINF)==std::string_view("#EXTINF:").substr(0,EXTINF))
        {

            std::cout<<"\nfind a # EXT_INF"<<std::endl;
                        ss>>line;
            sscanf(line.c_str(),"%lld.m4s",&m4sId);
            std::cout<<"m4s is"<<m4sId<<".m4s"<<std::endl;
            BLOCK empty;
            empty.first=nullptr;
            empty.second=0;
            // auto & [ref , inserOk]= m4slist.try_emplace(m4sId, std::make_pair<void* , size_t>(nullptr,0));
            m4slist.try_emplace(m4sId, std::move(empty));
            continue;

        }

        if(sbstr.substr(TargDurLen)==(std::string_view("#EXT-X-TARGETDURATION").substr(TargDurLen)))
        {
            std::cout<<"########find a #EXT-X-TARGETDURATION"<<std::endl;
            // continue;

        }
        // 
        std::cout<<"uri is "<<sbstr.substr(8)<<std::endl;
        if(sbstr.substr(MapUrILen)==(std::string_view("#EXT-X-MAP:URI")).substr(MapUrILen))
        {
            std::cout<<"#######find a ext map uri"<<std::endl;
            // ss>>line;
            // sscanf(line.c_str(),"%lld.m4s",&m4sId);
            // std::cout<<"m4s is"<<m4sId<<".m4s"<<std::endl;
            // continue;

        }

    }
    return 0;
}

size_t SymbleSplite::GetLine()
{
    return result_list.size();
}

std::string_view *SymbleSplite::GetNextView()
{
    if (_CurrentLine < GetLine())
    {
        return &result_list[_CurrentLine++];
    }
    else
    {
        return nullptr;
    }
}
void SymbleSplite::splitbychar(char _chr)
{
    // 分支处理
    size_t _sym_ptr = 0;
    size_t _ptr_tail = 0;
    result_list.clear();
    const char *src = _Src_Temp.data();
    // TODO 解决分隔符连续的情况
    for (char ver : _Src_Temp)
    {
        if (ver == _chr)
        {
            result_list.push_back(std::string_view(src + _sym_ptr, _ptr_tail));
            std::cout << std::string(src + _sym_ptr, _ptr_tail) << std::endl;
            _sym_ptr = _ptr_tail + 1;
        }
        _ptr_tail++;
    }
    // TODO ： 解决不存在分隔符以及最后一个字符是分隔符的情况；
    result_list.push_back(std::string_view(src + _sym_ptr, _ptr_tail));
    return;
}