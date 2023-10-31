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
#include <condition_variable>
#include <mutex>

#include "ExecTask.h"
#include "UniFetch.h"

// using namespace ParallelCtrl;

using BLOCK = std::pair<void *, size_t>;

class LivingRoomIndex;
class m3u8fetch;
class m4s2mp4;
class LiveHomeStatus;

class LiveHomeStatus
{

public:                     // h1698325202
    char m4shead[16] = {0}; // 添加对m4s检查
    pid_t SubPid;
    uint64_t live_time;
    // std::string RoomName; // 用于文件夹命名
    char RoomName[256] = {0};
    char RoomId_chr[32] = {0};
    // std::string RoomHostName; // used to fill host name  ; like "key725" "战鹰"，，parsed by json file
    char RoomHostName[32] = {0};
    uint64_t RoomId;
    bool Hidden;
    bool Lock;
    int live_status = 0;
    int live_status_old = 0;
    int64_t key_id = 0;
    bool encrypted;
    LivingRoomIndex *LivingRoomExt = nullptr;
    std::string GetM3u8Url();
    std::string GetM4sUrl(uint64_t m4s_id);
    std::string GetM4sContent(std::string header);
    // std::string M4sUrl_mode;
    m3u8fetch *FetchM3u8Node = nullptr;
    m4s2mp4 *TransUnit = nullptr;
    LiveHomeStatus *ProcShared = nullptr;
    int shmid = 0;
    /**
     * @brief 直播间开启或者关闭
     */
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
    std::mutex ConnMtx; // used to sync the
    std::condition_variable m4sTrigger;
};

class m3u8fetch
{
public:
    // using BlockPair = decltype(*(m4slist.begin()));
    using BlockPair = std::pair<uint64_t, std::pair<void *, size_t>>;

private:
    /* data */
    std::mutex mtx_m4s;                                    // if want to modify m4slist , the first thing is get this lock;
    std::map<uint64_t, std::pair<void *, size_t>> m4slist; // uint64_t 保存id

    uint64_t min_m4s_nb;     // 保存最小的m4s id from m4slist   所有小于该数值的文件都已经写入存储
    uint64_t Max_m4s_nb = 0; // 保存最大的m4s id from m4slist;  确保获取的m3u8文件下序列号小于等于该数值
    void *EXT_X_MAP;
    char *EXT_X_MAP_NAME;
    size_t EXT_X_MAP_len;
    // 每秒一次更新采集数据
    ExecTask<4> Exec;
    std::string Url_m3u8;
    LiveHomeStatus *_Parent; // 用于获取当前的状态信息
    WFTimerTask *FetchM3u8Task;
    WFHttpTask *_task;
    struct
    {
        // todo : 用于保存最新的m3u8文件  使用文件名以及文件长度来避免 避免重复解析
        uint64_t SeqId = 0;
        size_t FileSize = 0;
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
#if 0
    int SetFetchTask();
#endif
    void StartExec()
    {
        this->Exec.Start();
    }
#if 0
    void free_task()
    {
        // assert(this->_task != nullptr);
        auto ref = _task;
        _task = nullptr;
        if (ref == nullptr)
            return;
        // free(ref);
    }
#endif
    // _task = nullptr;
    std::string GetHeaderFileName() { return CurrentM3u8file.headFile; }
    // void Getm3u8file();
    void GetHeadfile();
    m3u8fetch(LiveHomeStatus *Parent);
    // int updatem3u8list();//@todo : 更新m3u8文件列表
    int Parserm3u8(char *, size_t); // 解析m3u8文件并且
    void RegisterExecutor();
    std::deque<BlockPair> PopFrontM4sList();

    ~m3u8fetch()
    {
        fprintf(stderr, "m3u8fetch destry \n");
        exit(-1);
    }
};

class m4s2mp4
{
public:
    LiveHomeStatus *LiveStatus;
    std::atomic_bool Atmc_Startonce ;
private:
    /* data */
    std::mutex _m4s_list_mtx;
    std::string _m4s_dir;
    std::string _m4s_filename;
    BLOCK _m4s_head;             // 作为头文件写入存储文件
    std::deque<BLOCK> _m4s_list; // 可以有序写入ssd中间
    FILE *file = nullptr;
    m3u8fetch *m3u8list;
    bool LiveisDown; // 直播关闭的时候为true 没有关的时候 false；
    bool Inited;
    void OpenFile();
    std::function<void(void)> _task;
    volatile bool InitFlag;
    ExecTask<2> Exec;

public:
    m4s2mp4(m3u8fetch *_mu, LiveHomeStatus *LHS) : LiveStatus(LHS), m3u8list(_mu), InitFlag(false)
    { // TODO : 初始化开始路径
        Atmc_Startonce.store(false);
        fprintf(stderr, "m4s2mp4 Start to init \n");
        assert(LiveStatus != nullptr);
        SetDirName();
        SetFilename();
        fprintf(stderr, "m4s2mp4 Start to init \n");

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
        fprintf(stderr , " m4s2mp4 deleted \n");
        fflush(file);
        fclose(file);
    }

    void SetFilename()
    {
        char buff[25] = {0};
        auto now_time = std::chrono::system_clock::now();
        struct tm time;
        auto tt = std::chrono::system_clock::to_time_t(now_time);
        localtime_r(&tt, &time);
        // 年月日时分
        sprintf(buff, "start_at_%04d_%02d_%02d_%02d_%02d.m4s", time.tm_year, time.tm_mon, time.tm_mday, time.tm_hour, time.tm_sec);
        _m4s_filename = std::string(buff);
    }
    void SetDirName()
    {
        // _m4s_dir = this->LiveStatus->RoomName;
        _m4s_dir = std::string(this->LiveStatus->RoomName);
        assert(_m4s_dir.size() != 0);
    }
    void Start();
    void StartOnce();
    void AppendMsgBlock();
    void GetM4sList();
    void InitFile(); // todo : 初始化file并且将数据插入
    // void TransCodeWrite();//TODO : ffmpeg 解码并写入文件
};


struct Notifyer {
std::mutex mtx;
std::condition_variable cv;
};

extern struct Notifyer notifyer;

// class UniFetch
// {

// private:
//     LiveHomeStatus *Room;

// private:
//     void m3u8init();
//     void m4sinit();

// public:
//     std::mutex ConnMtx; // used to sync the
//     std::condition_variable m4sTrigger;

// public:
//     /// @brief construct function  used to init m3u8fetch and m4s Downloader
//     /// @param LiveRoom
//     UniFetch(LiveHomeStatus *LiveRoom);
// };