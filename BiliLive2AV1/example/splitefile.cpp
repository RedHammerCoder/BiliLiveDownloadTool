#include "m3u8fetch.h"
#include "fetch_live_status.h"
#include <string_view>
#include <iostream>
#include <sstream>
const char filename []="./Test/ac.m3u8";
char BUFF [1024];

int main()
{
    auto  file = fopen(filename , "r");
    memset((void*)BUFF,0,1024);
    size_t len =  fread((void*)BUFF , 1024 , 1 ,file);
    m3u8fetch mft(nullptr);
    mft.Parserm3u8(BUFF  , len);




    return 0;
}