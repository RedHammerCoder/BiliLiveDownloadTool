#include "fetch_live_status.h"
#include <sys/unistd.h>
#include <sys/signal.h>
#include "KExecutor.h"




bool SigctrlC=false;

WFFacilities::WaitGroup wg(1);

void sigCtrlC(int sig)
{
    SigctrlC=true;
    fprintf(stderr , "sig ctrl c called\n");
    wg.done();

}


int main()
{
    // FILE * fd =  fopen("timerlog.txt","w+");
    int len =0;
    Listening_liveroom_init();
    // GetliveStatus("4089850");
    UpdateRoomListMsg();
    sleep(3);
    LivingRoomIndexAnalysisNew();
    
    // sleep(40);
    fprintf(stderr , "\nPROGRAMDONE\n");
    // fclose(fd);

    signal(SIGINT,sigCtrlC);
    wg.wait();


    return 0;
}