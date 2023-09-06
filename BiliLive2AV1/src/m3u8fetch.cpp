#include "m3u8fetch.h"
// #include "fetch_live_status.h"
// #include <map>
// #include <string>
// #include <utility>
#include <string_view>
#include <sstream>

constexpr size_t unusedLen=strlen("#EXTM3U\n#EXT-X-VERSION:7\n#EXT-X-START:TIME-OFFSET=0\n");


m3u8fetch::m3u8fetch(LiveHomeStatus* parent):_Parent(parent)
{

}

inline size_t GetNextSymable(char* src, char dec);


void m3u8fetch::Parserm3u8(BLOCK block)
{
    auto & [chrptr , len ] = block;
    std::string_view m3u8template((char*)chrptr , len  );
    std::string value;
    std::stringstream ss;
    char* charptr=(char*)chrptr;
    assert(strncmp(charptr ,"#EXTM3U" , strlen("#EXTM3U")==0 ));
    charptr+=unusedLen;
}

void SymbleSplite::Parserbychar(char _chr)
{






    return;
}