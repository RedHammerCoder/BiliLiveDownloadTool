#include "fetch_live_status.h"
#include <sys/unistd.h>
#include "KExecutor.h"
void appendtimer( int& len)
{

    fprintf(stderr,"write file  %d\n",len++);
}
int main()
{
    // FILE * fd =  fopen("timerlog.txt","w+");
    int len =0;
    Default_ExecutorManager.Start();
    Listening_liveroom_init();
    // GetliveStatus("4089850");
    UpdateRoomListMsg();
        sleep(3);
    LivingRoomIndexAnalysis();
    
    sleep(40);
    fprintf(stderr , "\nPROGRAMDONE\n");
    // fclose(fd);

    return 0;
}