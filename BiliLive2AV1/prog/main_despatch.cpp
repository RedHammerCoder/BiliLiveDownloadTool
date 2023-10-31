#include <iostream>
#include <unistd.h>
#include <sstream>
#include <string>
#include "basic_class.h"
#include "fetch_live_status.h"
#include "m4s2mp4.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <sys/types.h>

LiveHomeStatus *LiveStatus;

void Exit()
{
    fprintf(stderr, " SubExec Exited  %d", getpid());
    // (LiveStatus->FetchM3u8Node)->~m3u8fetch();
    // LiveStatus->TransUnit->~m4s2mp4();
    // auto key = LiveStatus->key_id;
    // shmdt((void*)LiveStatus);
    exit(-1);
}
void sigKill(int sig)
{
    Exit();
}
int main(int argc, char **argv)
{
    // signal(SIGKILL , sigKill);
    // signal(SIGINT, sigKill);
    fprintf(stderr, "PID %d", getpid());
    fprintf(stderr, "Despatch exec err %s\n", argv[0]);
    SetDefaultPath(argv[1]);

    assert(argc == 2);
    pid_t pid = getpid();
    std::string share_mem(argv[0]);
    std::stringstream ss;
    ss << share_mem;
    int id = 0;
    ss >> id;
    LiveHomeStatus *LiveStatus;
    int ShmId = shmget((key_t)id, sizeof(LiveHomeStatus), 0666 | IPC_CREAT);
    if (ShmId == -1)
    {
        fprintf(stderr, "Exit in shmget %s", strerror(errno));
        exit(-1);
    }
    void *target = shmat(ShmId, 0, 0);
    if (target == (void *)-1)
    {
        fprintf(stderr, "shmat  Err  is %s", strerror(errno));
        exit(-1);
    }
    LiveStatus = (LiveHomeStatus *)target;
    LiveStatus->ProcShared = nullptr;
    fprintf(stderr, "RoomName \n");
    FreshLiveRoomStatus(LiveStatus);
    sleep(5);
    // m3u8fetchLoop.Start();

    // Default_ExecutorManager.Start();
fprintf(stderr, "Despatch in Loop\n");
    while (1)
    {
        fprintf(stderr, "Despatch in Loop\n");
        sleep(20);
    }
    Exit();
}
