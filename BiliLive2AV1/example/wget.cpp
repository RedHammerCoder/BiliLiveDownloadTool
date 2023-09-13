#include "fetch_live_status.h"
#include <string>
#include <iostream>
std::string addr("https://live.bilibili.com/904823?live_from=85001&spm_id_from=444.41.live_users.item.click");
std::string filepath("./Test/wget.html");
int main(int argc ,char *argv[])
{
    if(argc!=2)return -1;
    const void * dest;
    size_t len;
    int state = FetchHttpBody(addr,&dest,&len);
    std::cout<<"exit  from inter"<<std::endl;
    FILE* fd =  fopen(filepath.c_str(),"w+");
    fwrite(dest,len,1,fd);
    fclose(fd);
    std::cout<<"end"<<std::endl;
    return 0;
}
// https://www.suacg.com/moehome-3-44957.html