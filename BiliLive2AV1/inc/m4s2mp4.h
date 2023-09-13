/**
 * @file m4s2mp4.h
 * @author yaoyuxin (you@domain.com)
 * @brief get file from m4s list and write package to file
 * @version 0.1
 * @date 2023-09-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <string>
#include "m3u8fetch.h"
#include <deque>
#include <mutex>
// using namespace std;
extern std::string Default_Path;


void SetDefaultPath(std::string);


class m4s2mp4:public KExecutor
{
private:
    /* data */
    std::mutex _m4s_list_mtx;
    std::string  _m4s_dir;
    std::string  _m4s_filename;
    BLOCK _m4s_head;//作为头文件写入存储文件
    std::deque<BLOCK> _m4s_list;//可以有序写入ssd中间
    FILE* file;
    LiveHomeStatus * LiveStatus;
    m3u8fetch* m3u8list;
    bool LiveisDown;//直播关闭的时候为true 没有关的时候 false；
    bool Inited;
    void OpenFile();

public:
    m4s2mp4(m3u8fetch* _mu , LiveHomeStatus* LHS  ):LiveStatus(LHS), m3u8list(_mu),KExecutor(&Default_ExecutorManager){};
    ~m4s2mp4(){}

    void SetFilename(std::string name){_m4s_filename= std::move(name);}
    void SetDirName(std::string Dir){_m4s_dir=std::move(Dir);}
    void AppendMsgBlock();
    void GetM4sList();
    void InitFile();//todo : 初始化file并且将数据插入
    // void TransCodeWrite();//TODO : ffmpeg 解码并写入文件

};