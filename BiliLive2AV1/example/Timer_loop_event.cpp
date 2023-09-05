#include <workflow/WFTaskFactory.h>
#include <iostream>
using namespace std;
uint64_t sum=0;



void timer_task(WFTimerTask * task)
{
    std::cout<<"WFtimer called  "<<sum++<<std::endl;
    WFTimerTask * subtask = WFTaskFactory::create_timer_task(10000 , timer_task);
    series_of(task)->push_back(subtask);


}

int main()
{
    auto task = WFTaskFactory::create_timer_task(10000,timer_task);
    task->start();
    getchar();


    return 0;
}