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









    // printf(BUFF);
    // SymbleSplite spi(std::string_view(BUFF , len));
    // spi.splitbychar();
    // spi.splitbychar();

    // getline()

    // printf("splite len is %d \n", spi.GetLine());
    // for(auto & ref : spi.result_list)
    // {

        // std::string_view sv = ref.substr(3);
        // std::cout<<ref<<std::endl;
         
    // }

    








    return 0;
}