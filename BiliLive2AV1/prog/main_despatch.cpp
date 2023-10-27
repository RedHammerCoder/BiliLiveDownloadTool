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

int main(int argc, char **argv)
{
    fprintf(stderr, "Despatch exec err %s\n", argv[0]);
    assert(argc == 1);
    pid_t pid = getpid();
    std::string share_mem(argv[0]);
    std::stringstream ss;
    ss << share_mem;
    int id = 0;
    ss >> id;
    fprintf(stderr, "Entry to despatch  %d \n", id);
    LiveHomeStatus *LiveStatus;
    int ShmId = shmget((key_t)id, sizeof(LiveHomeStatus), 0666 | IPC_CREAT);
    if (ShmId == -1)
    {
        exit(-1);
    }
    void *target = shmat(ShmId, 0, 0);

    if (target == (void *)-1)
    {
        fprintf(stderr, "shmat  Err  is %s", strerror(errno));
        exit(-1);
    }
    LiveStatus = (LiveHomeStatus *)target;
    // assert(LiveStatus->ProcShared != nullptr);
    LiveStatus->ProcShared = nullptr;
    fprintf(stderr , "RoomName %s",LiveStatus->RoomName.c_str() );

    return 0;
}
