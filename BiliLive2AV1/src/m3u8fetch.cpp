#include "m3u8fetch.h"
#include <string_view>
#include <string.h>
#include <sstream>

constexpr size_t unusedLen=strlen("#EXTM3U\n#EXT-X-VERSION:7\n#EXT-X-START:TIME-OFFSET=0\n");

// m3u8fetch::m3u8fetch(LiveHomeStatus* parent):_Parent(parent)
// {

// }

// inline size_t GetNextSymable(char* src, char dec);


// void m3u8fetch::Parserm3u8(BLOCK block)
// {
//     auto & [chrptr , len ] = block;
//     std::string_view m3u8template((char*)chrptr , len  );
//     std::string value;
//     std::stringstream ss;
//     char* charptr=(char*)chrptr;
//     assert(strncmp(charptr ,"#EXTM3U" , strlen("#EXTM3U")==0 ));
//     charptr+=unusedLen;
// }

size_t SymbleSplite::GetLine()
{
    return result_list.size();
}


std::string_view* SymbleSplite::GetNextView()
{
    if(_CurrentLine< GetLine() )
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
    size_t _sym_ptr=0;
    size_t _ptr_tail=0;
    result_list.clear();
    const char * src =  _Src_Temp.data();
    //TODO 解决分隔符连续的情况
    for(char ver : _Src_Temp)
    {
        if(ver==_chr)
        {
            result_list.push_back(std::string_view(src+_sym_ptr,_ptr_tail));
            _sym_ptr=_ptr_tail+1;
        }
        _ptr_tail++;

    }
    //TODO ： 解决不存在分隔符以及最后一个字符是分隔符的情况；
    result_list.push_back(std::string_view(src+_sym_ptr , _ptr_tail));
    return;
}