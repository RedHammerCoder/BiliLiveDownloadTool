#pragma once
#include <string>
#include "KExecutor.h"
#include <deque>


#include <workflow/WFTask.h>
#include <workflow/HttpMessage.h>
#include <workflow/HttpUtil.h>
#include <workflow/WFTaskFactory.h>
#include <workflow/WFFacilities.h>

using BLOCK = std::pair<void *, size_t>;

class LivingRoomIndex;
class m3u8fetch;
class m4s2mp4;

struct LiveHomeStatus
{
    uint64_t live_time;
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
    m3u8fetch *FetchM3u8Node;
    m4s2mp4* TransUnit;
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
    using BlockPair = std::pair<uint64_t,std::pair<void *, size_t>>;


private:
    /* data */
    std::mutex mtx_m4s;                                    // if want to modify m4slist , the first thing is get this lock;
    std::map<uint64_t, std::pair<void *, size_t>> m4slist; // uint64_t 保存id
    uint64_t min_m4s_nb;                                   // 保存最小的m4s id from m4slist   所有小于该数值的文件都已经写入存储
    uint64_t Max_m4s_nb;                                   // 保存最大的m4s id from m4slist;  确保获取的m3u8文件下序列号小于等于该数值
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
    int CreateFetchTask();
    void resetUri()
    {
        free(this->_task);
        _task = nullptr;
        try_start();
    }

public:
    std::string GetHeaderFileName(){return CurrentM3u8file.headFile;}
    // void Getm3u8file();
    void GetHeadfile();
    m3u8fetch(LiveHomeStatus *Parent);
    int try_start()
    {
        if (_Parent->live_status != 1)
            return -1;
        // fprintf(stderr, "m3u8 fetched\n");
        if (Url_m3u8.size() == 0)
        {
            Url_m3u8 = std::move(this->_Parent->GetM3u8Url());
            // fprintf(stderr, "----------########  m3u8 add is %s\n", Url_m3u8.c_str());
            if (Url_m3u8.size() == 0)
            {
                // fprintf(stderr , "get m3 u8 file err---------------------\n");
                return -1;
            }
        }
        if (_task == nullptr)
        {
            CreateFetchTask();
        }
        assert(this->_task != nullptr);
        if (_task != nullptr)
        {
            // fprintf(stderr, "------------m3u8 http start-----------\n");
            this->_task->start();
            return 0;
        }
    }
    // int updatem3u8list();//@todo : 更新m3u8文件列表
    int Parserm3u8(char *, size_t); // 解析m3u8文件并且
    void RegisterExecutor();
    std::deque<BlockPair> PopFrontM4sList();

    ~m3u8fetch() = default;
};

class m4s2mp4 : public KExecutor
{
private:
    /* data */
    std::mutex _m4s_list_mtx;
    std::string _m4s_dir;
    std::string _m4s_filename;
    BLOCK _m4s_head;             // 作为头文件写入存储文件
    std::deque<BLOCK> _m4s_list; // 可以有序写入ssd中间
    FILE *file;
    LiveHomeStatus *LiveStatus;
    m3u8fetch *m3u8list;
    bool LiveisDown; // 直播关闭的时候为true 没有关的时候 false；
    bool Inited;
    void OpenFile();
    std::function<void(void)> _task;

public:
    m4s2mp4(m3u8fetch *_mu, LiveHomeStatus *LHS) : LiveStatus(LHS), m3u8list(_mu), KExecutor(&Default_ExecutorManager)
    {//TODO : 初始化开始路径
        SetFilename("file.m4s");
        SetDirName("key725");
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
    ~m4s2mp4() {
        fflush(file);
        fclose(file);
    }

    void SetFilename(std::string name) { _m4s_filename = std::move(name); }
    void SetDirName(std::string Dir) { _m4s_dir = std::move(Dir); }
    void Start();
    void AppendMsgBlock();
    void GetM4sList();
    void InitFile(); // todo : 初始化file并且将数据插入
    // void TransCodeWrite();//TODO : ffmpeg 解码并写入文件
};



