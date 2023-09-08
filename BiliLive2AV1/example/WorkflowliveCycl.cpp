
#include <workflow/WFTaskFactory.h>
#include <syscall.h>
#include <unistd.h>
/**
 * @brief 用于测试多个类在series_of 中间的生命周期
 * 
 */
#include <iostream>
#include <string>
using namespace std;


class test{
    public:
    string _str;
    test(std::string str):_str(str)  {
        cout<<"entry to"<<ends<<_str<<endl;
    }

    ~test()
    {
        cout<<"exit from "<<_str<<endl;
    }
};




int main()
{
    auto gotask =  WFTaskFactory::create_go_task("entry" , [](){ test adj("adj"); });
    gotask->set_callback([](WFGoTask* gotask)
    {
        cout<<"entry to call back"<<endl;

    });
    gotask->start();
    sleep(2);

}