#pragma once

#include <workflow/Executor.h>
#include <functional>
#include <utility>
#include <map>
#include <thread>
#include <memory>
#include <atomic>
#include <unistd.h>
#include <assert.h>
#include <mutex>
class Executor;

using ExecNode = std::function<void(void)>;
class ExecutorManager
{
    /**
     * @todo 添加时间轮算法  避免不通任务执行不同
     * 
     */
    // size_t tasksum;
    std::atomic_int64_t tasksum;
    std::mutex mtx_taskmap;
    std::thread thr;
    bool execflag;
    std::map<int64_t , std::weak_ptr<ExecNode> > TaskMap;
    void _start()
    {
        while (execflag)
        {
            std::lock_guard lgx(mtx_taskmap);
            if(TaskMap.size()==1){sleep(1);continue;}
            for(auto& node  : TaskMap )
            {
                if(node.second.expired())continue;//保管对象已经删除
                //todo : 保管对象没有删除时候
                (*(node.second.lock()).get())();//执行node的代码
            }
            sleep(4);
        }
        
    }
    public :
    ExecutorManager():execflag(true)
    {
        tasksum.store(0);
    }
    void Start()
    {
        std::thread t(_start);
        thr=std::move(t);
    }
    int  uploadNode(std::weak_ptr<ExecNode> Task)
    {//TODO: 获取一个id并将id以及对应的weak_ptr 插入 TaskMap中间
    int64_t _id = tasksum++;
    TaskMap.insert(std::pair<int64_t , std::weak_ptr<ExecNode>>(_id,std::move(Task))  );
    return _id;
    }
    ~ExecutorManager()
    {
        execflag=false;
        assert(thr.joinable());
        thr.join();
    }
};

class Executor
{
private:
    // std::function<void(void)> _Task;
    std::shared_ptr<ExecNode> _Task; 
    int ID;
    ExecutorManager* _manager;

    public:
    Executor(ExecutorManager * manager):_manager(manager){}
    void SetTask(ExecNode task)
    {
        // _Task=std::move(task);
        _Task= std::make_shared<ExecNode>(std::move(task));
    }
    void UploadNode()
    {
        ID = _manager->uploadNode(_Task);

    }
    virtual ~Executor(){}
};






