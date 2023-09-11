#include "fetch_live_status.h"
#include <sys/unistd.h>

void appendtimer( int& len)
{

    fprintf(stderr,"write file  %d\n",len++);
}
int main()
{
    // FILE * fd =  fopen("timerlog.txt","w+");
    int len =0;
    auto exec = [&]()
    {
        appendtimer(len);
    };
    // LoopExecEvent<300000 , decltype( exec)>   execnode(exec);
    // execnode.start(); 
    Listening_liveroom_init();
    // GetliveStatus("4089850");
    UpdateRoomListMsg();
        sleep(3);
    LivingRoomIndexAnalysis();
    
    sleep(4);
    fprintf(stderr , "\nPROGRAMDONE\n");
    // fclose(fd);

    return 0;
}