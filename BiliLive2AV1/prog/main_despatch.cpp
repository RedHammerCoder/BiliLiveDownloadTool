#include <iostream>
#include <unistd.h>
#include <sstream>
#include <string>
#include "basic_class.h"
#include "fetch_live_status.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <sys/types.h>

LiveHomeStatus *LiveStatus;

void sigKill(int sig)
{
    exit(-1);
}



int main(int argc, char **argv)
{
    signal(SIGKILL , sigKill);
    fprintf(stderr , "PID %d",getpid());
    auto FD= fopen("./1.txt","w+");
    fprintf(FD, "Despatch exec err %s\n", argv[0]);

    
    assert(argc == 1);
    pid_t pid = getpid();
    std::string share_mem(argv[0]);
    std::stringstream ss;
    ss << share_mem;
    int id = 0;
    ss >> id;
    fprintf(FD, "Entry to despatch  %d \n", id);
    LiveHomeStatus *LiveStatus;
    int ShmId = shmget((key_t)id, sizeof(LiveHomeStatus), 0666 | IPC_CREAT);
    if (ShmId == -1)
    {
        fprintf(FD, "Exit in shmget %s", strerror(errno));
        exit(-1);
    }
    void *target = shmat(ShmId, 0, 0);

    if (target == (void *)-1)
    {
        fprintf(FD, "shmat  Err  is %s", strerror(errno));
        exit(-1);
    }
    LiveStatus = (LiveHomeStatus *)target;
    // assert(LiveStatus->ProcShared != nullptr);
    LiveStatus->ProcShared = nullptr;
    fprintf(FD , "RoomName %s",LiveStatus->RoomName.c_str() );
    while (1)
    {
        fprintf(FD, "Exec in Loop\n");
        sleep(1);
    }
    fclose(FD);
    return 0;
}
