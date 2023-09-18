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
class KExecutor;
using ExecNode = std::function<void(void)>;



// template <int Timer> 
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
    ExecNode _startNode;
    public :
        void _start()
    {
        while (execflag)
        {
            fprintf(stderr , "   -------------loop exec ----------------\n");
            std::lock_guard lgx(mtx_taskmap);
            if(TaskMap.size()==0){sleep(1);continue;}
            for(auto& node  : TaskMap )
            {
                if(node.second.expired()){
                    fprintf(stderr , "saved obj is deleted\n");
                    continue;}//保管对象已经删除
                //todo : 保管对象没有删除时候
                fprintf(stderr , "Node Ready TO exec\n");
                (*(node.second.lock()).get())();//执行node的代码
                fprintf(stderr , "Node execd\n");
            }
            sleep(4);
        }
    }
    ExecutorManager():execflag(true)
    {
        _startNode=[&](){this->_start();};
        tasksum.store(0);
    }
    void Start()
    {
        std::thread t(_startNode);
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
        if(thr.joinable())thr.join();
    }
} ;
extern ExecutorManager Default_ExecutorManager;
// extern ExecutorManager<3> m3u8fetchManager;

class KExecutor
{
private:
    // std::function<void(void)> _Task;
    std::shared_ptr<ExecNode> _Task; 
    int ID;

    ExecutorManager* _manager;

    public:
    KExecutor(ExecutorManager * manager):_manager(manager){}
    void SetTask(ExecNode task)
    {
        // _Task=std::move(task);
        _Task= std::make_shared<ExecNode>(std::move(task));
    }
    void UploadNode()
    {
        fprintf(stderr , "____________  upload node --------------\n");
        ID = _manager->uploadNode(_Task);

    }
    void SwapNode()
    {
    }
    virtual ~KExecutor(){}
};






