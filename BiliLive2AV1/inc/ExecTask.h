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
using Task = std::function<void(void)>;
template <uint64_t sleeptime>
class ExecTask
{
public:
    Task _task;
    bool RunningFlag = true;
    std::thread _thread;
    int Status = 0;

public:
    ExecTask(Task &&t) : _task(t)
    {
        // _task(std::move(t));
    }
    ExecTask() {}
    void SetTask(Task &&t)
    {
        _task=std::move(t);
    }
    void Start()
    {
        auto TaskPack = [=]()
        {
            while (this->RunningFlag)
            {
                fprintf(stderr, "thread Exec --------\n");
                this->_task();
                // assert(Status == 0);
                sleep(sleeptime);
            }
        };
        _thread = std::move(std::thread(TaskPack));
    }
    ~ExecTask()
    {
        if (_thread.joinable())
        {
            RunningFlag = false;
            _thread.join();
        }
    }
};

#endif
