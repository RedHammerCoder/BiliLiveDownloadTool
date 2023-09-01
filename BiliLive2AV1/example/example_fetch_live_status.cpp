#include "fetch_live_status.h"
#include <sys/unistd.h>

int main()
{
    Listening_liveroom_init();
    // GetliveStatus("4089850");
    UpdateRoomListMsg();
        sleep(3);
    LivingRoomIndexAnalysis();
    fprintf(stderr , "\nPROGRAMDONE\n");

    return 0;
}