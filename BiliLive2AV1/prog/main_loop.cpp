#include <sys/ipc.h>
#include <sys/shm.h>
#include "basic_class.h"
#include "fetch_live_status.h"
#include "string"
#include "sstream"
#include <sys/signal.h>
#include <sys/types.h>
#include <new>
#include <atomic>

// 使用kill pid 0 来判断子进程是否存在
//  liveroom_list  std::deque<livehomestatus>
std::atomic_int64_t random_id;

int main(int argc, char **argv)
{
    random_id = getpid() + 1;
    fprintf(stderr, "start to exec\n");
    // 初始化liveroomlist
    Listening_liveroom_init();
    while (true)
    {
        for (auto &ref : liveroom_list)
        {
            if (ref.key_id == 0)
            {
                int key_id = random_id.fetch_add(1);
                ref.key_id = key_id;
                int shm_identifier = shmget((key_t)key_id, sizeof(LiveHomeStatus), 0666 | IPC_CREAT | IPC_EXCL);
                if (shm_identifier == -1)
                {
                    fprintf(stderr, "error on Create shared Memary \n");
                    exit(-1);
                }
                void *addr = shmat(shm_identifier, 0, 0);
                assert(addr != nullptr);
                fprintf(stderr, "\n key_id is %d\n", shm_identifier);
                if (addr == (void *)-1)
                {
                    fprintf(stderr, "shmat attach mem Error  %s\n", strerror(errno));
                    switch (errno)
                    {
                    case EACCES:
                        fprintf(stderr, "EACCES\n");
                        break;
                    case EIDRM:
                        fprintf(stderr, "EIDRM\n");
                        break;
                    case EINVAL:
                        fprintf(stderr, "EINVAL\n");
                        break;
                    default:
                        fprintf(stderr, "OTHER\n");
                        break;
                    }
                    exit(-1);
                }
                auto k = new (addr) LiveHomeStatus();
                k->live_status=0;
                ref.ProcShared = (LiveHomeStatus *)addr;
                *k = ref;
            }
            // 已经完成初始化
            // TODO: 开始获取直播间信息
            UpdateRoomMsg(ref);

            if (ref.live_status == 1 && ref.ProcShared->live_status == 0)
            {
                // 子进程负责设置ref.live_status_old为1

                pid_t pt = fork();
                fprintf(stderr , "fork called \n");
                if (pt < 0)
                {
                    exit(-1);
                }
                if (pt == 0)
                {
                    // it is new process;
                    sleep(3);
                    // TODO: create shared memary;
                    int64_t shared_memary_key = ref.key_id;
                    std::stringstream ss;
                    ss << shared_memary_key;
                    auto &&key_id = ss.str();
                    fprintf(stderr, "get shared mem key %s \n", key_id.c_str());
                    char buff[16] = {0};
                    sprintf(buff, key_id.c_str());
                    int id = execl("./main_despatch", buff);
                }
                if (pt > 0)
                { // it is parsent process
                    ref.SubPid = pt;
                    ref.ProcShared->SubPid=pt;
                }
                ref.ProcShared->live_status = 1;
            }

            if (ref.live_status == 0 && ref.ProcShared->live_status == 1)
            {
                // TODO: kill sub proc;
                fprintf(stderr, "start to kill sub Proc \n");
                ref.ProcShared->live_status = 0;
            }
            kill(ref.SubPid, SIGINT);
            *(ref.ProcShared) = ref;
        }
        sleep(20); // 睡眠20s
    }

    return 0;
}