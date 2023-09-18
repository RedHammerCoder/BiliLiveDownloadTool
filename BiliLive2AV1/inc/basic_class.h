#pragma once
#include <string>
#include "KExecutor.h"
#include "SyncBarrier.h"
#include <deque>
#include "ErrorLog.h"
#include <workflow/WFTask.h>
#include <workflow/HttpMessage.h>
#include <workflow/HttpUtil.h>
#include <workflow/WFTaskFactory.h>
#include <workflow/WFFacilities.h>
#include <atomic>
#include <ctime>
#include <chrono>

// using namespace ParallelCtrl;

using BLOCK = std::pair<void *, size_t>;

class LivingRoomIndex;
class m3u8fetch;
class m4s2mp4;
class LiveHomeStatus;

class LiveHomeStatus
{
public:
    uint64_t live_time;
    std::string RoomName; // 用于文件夹命名
    char RoomId_chr[32] = {0};
    std::string RoomHostName; // used to fill host name  ; like "key725" "战鹰"，，parsed by json file
    uint64_t RoomId;
    bool Hidden;
    bool Lock;
    int live_status;
    bool encrypted;
    LivingRoomIndex *LivingRoomExt = nullptr;
    std::string GetM3u8Url();
    std::string GetM4sUrl(uint64_t m4s_id);
    std::string GetM4sContent(std::string header);
    std::string M4sUrl_mode;
    m3u8fetch *FetchM3u8Node = nullptr;
    m4s2mp4 *TransUnit = nullptr;
};

class LivingRoomIndex
{
public:
    std::string GeneratorUrl;
    std::string formate_name, Codec_name;
    std::string BaseUrl;
    std::string host;
    std::string ExtraUrl;
    // m3u8 Url = host+BaseUrl+extra
    // stream_ttl 是干嘛用的？有可能是用于更新m3u8 url
    /**
     * @brief 基于GeneratorUrl 更新多种url
     * formate_name Codec_name来源于选定的formate_name 以及 codec_name
     *
     * @return * void
     */
    // std::string Getm4sUrl();
};

class m3u8fetch : public KExecutor
{
public:
    // using BlockPair = decltype(*(m4slist.begin()));
    using BlockPair = std::pair<uint64_t, std::pair<void *, size_t>>;

private:
    /* data */
    std::mutex mtx_m4s;                                    // if want to modify m4slist , the first thing is get this lock;
    std::map<uint64_t, std::pair<void *, size_t>> m4slist; // uint64_t 保存id

    uint64_t min_m4s_nb; // 保存最小的m4s id from m4slist   所有小于该数值的文件都已经写入存储
    uint64_t Max_m4s_nb; // 保存最大的m4s id from m4slist;  确保获取的m3u8文件下序列号小于等于该数值
    void *EXT_X_MAP;
    char *EXT_X_MAP_NAME;
    size_t EXT_X_MAP_len;

    std::string Url_m3u8;
    LiveHomeStatus *_Parent; // 用于获取当前的状态信息
    WFTimerTask *FetchM3u8Task;
    const int Exec_time;
    WFHttpTask *_task;
    struct
    {
        // todo : 用于保存最新的m3u8文件  使用文件名以及文件长度来避免 避免重复解析
        uint64_t SeqId;
        size_t FileSize;
        std::string headFile;

    } CurrentM3u8file;
public:
    void resetUri()
    {
        if (Url_m3u8.size() == 0)
        {
            Url_m3u8 = std::move(this->_Parent->GetM3u8Url());
            // fprintf(stderr, "----------########  m3u8 add is %s\n", Url_m3u8.c_str());
            if (Url_m3u8.size() == 0)
            {
                fprintf(stderr, "get m3 u8 file err---------------------\n");
                return;
            }
        }
    }
    int SetFetchTask();
    void free_task()
    {
        // assert(this->_task != nullptr);
        auto ref = _task;
        _task = nullptr;
        if(ref==nullptr)return;
        // free(ref);
    }
    // _task = nullptr;
    std::string GetHeaderFileName() { return CurrentM3u8file.headFile; }
    // void Getm3u8file();
    void GetHeadfile();
    m3u8fetch(LiveHomeStatus *Parent);
    int try_start()
    {

        fprintf(stderr, "Try start\n");
        // assert(_Parent->live_status==1);
        fprintf(stderr, "m3u8 fetched\n");
        this->resetUri();
        fprintf(stderr, "--------begin Set Fetch Task\n");
        fflush(stderr);
        return 0;
        this->SetFetchTask();
        fprintf(stderr, "Set Fetch Task\n");
        // assert(this->_task != nullptr);
        // fprintf(stderr, "------------m3u8 http start-----------\n");
        // this->_task->start();
        return 0;
    }
    // int updatem3u8list();//@todo : 更新m3u8文件列表
    int Parserm3u8(char *, size_t); // 解析m3u8文件并且
    void RegisterExecutor();
    std::deque<BlockPair> PopFrontM4sList();

    ~m3u8fetch(){
        fprintf(stderr , "m3u8fetch destry \n");
        exit(-1);
    }
};

class m4s2mp4 : public KExecutor
{
private:
    /* data */
    std::atomic_bool Atmc_Startonce = false;
    std::mutex _m4s_list_mtx;
    std::string _m4s_dir;
    std::string _m4s_filename;
    BLOCK _m4s_head;             // 作为头文件写入存储文件
    std::deque<BLOCK> _m4s_list; // 可以有序写入ssd中间
    FILE *file = nullptr;
    LiveHomeStatus *LiveStatus;
    m3u8fetch *m3u8list;
    bool LiveisDown; // 直播关闭的时候为true 没有关的时候 false；
    bool Inited;
    void OpenFile();
    std::function<void(void)> _task;
    bool InitFlag = false;

public:
    m4s2mp4(m3u8fetch *_mu, LiveHomeStatus *LHS) : LiveStatus(LHS), m3u8list(_mu), KExecutor(&Default_ExecutorManager)
    { // TODO : 初始化开始路径
        assert(LiveStatus!=nullptr);
        SetDirName();
        SetFilename();
        
        // InitFile();
#if 0
        _task = [&](){
            this->GetM4sList();
            this->AppendMsgBlock();
            fprintf(stderr,"append block to disk\n");
        };
        KExecutor::SetTask(_task);
        KExecutor::UploadNode();
#endif
    };
    ~m4s2mp4()
    {
        fflush(file);
        fclose(file);
    }

    void SetFilename()
    {
        char buff[25]={0};
        auto now_time =  std::chrono::system_clock::now();
        struct tm time;
        auto tt = std::chrono::system_clock::to_time_t(now_time);
        localtime_r(&tt,&time);
        // 年月日时分
        sprintf(buff,"start_at_%04d_%02d_%02d_%02d_%02d.m4s",time.tm_year,time.tm_mon,time.tm_mday,time.tm_hour,time.tm_sec);
        _m4s_filename=std::string(buff);
    }
    void SetDirName() { _m4s_dir =  this->LiveStatus->RoomName;assert(LiveStatus->RoomName.size()!=0 );}
    void Start();
    void StartOnce();
    void AppendMsgBlock();
    void GetM4sList();
    void InitFile(); // todo : 初始化file并且将数据插入
    // void TransCodeWrite();//TODO : ffmpeg 解码并写入文件
};
