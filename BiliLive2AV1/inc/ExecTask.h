#ifndef _ExecTask_
#define _ExecTask_
#include <functional>
#include <thread>
#include <assert.h>
#include <unistd.h>

/**
 * @brief ExecTask 用于建立一个线程并循环执行线程内部的数据
 *
 */
using Task = std::function<int(void)>;
template <uint64_t sleeptime>
class ExecTask
{
private:
    Task _task;
    bool RunningFlag = true;
    std::thread _thread;
    int Status=0;
public:
    ExecTask(Task &&t):_task(t)
    {
        // _task(std::move(t));
    }
    void Start()
    {
        auto TaskPack = [&RunningFlag, &_task ,&Status]()
        {
            while (RunningFlag)
            {
                fprintf(stderr, "thread Exec --------\n");
                Status = _task();
                assert(Status == 0);
                sleep(sleeptime);
            }
        };
        _thread = std::move(std::thread(TaskPack));
    }
    ~ExecTask()
    {
        if(_thread.joinable())
        {
            RunningFlag=false;
            _thread.join();
        }
    }
};

#endif
