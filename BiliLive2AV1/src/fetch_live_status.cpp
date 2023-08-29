#include "fetch_live_status.h"
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace rapidjson;
std::deque<LiveHomeStatus> liveroom_list;
const std::string web_live_status("https://api.live.bilibili.com/room/v1/Room/room_init");
char * HOME = getenv("HOME");
std::string default_profile_json =  std::string(HOME)+"/.BiliLiveDown.json";

std::vector<M4SVideo> m4slist;
void fetch_live_status_callback(WFHttpTask *task)
{
    protocol::HttpRequest *req = task->get_req();
    protocol::HttpResponse *resp = task->get_resp();
    int state = task->get_state();
    int error = task->get_error();

    switch (state)
    {
    case WFT_STATE_SYS_ERROR:
        fprintf(stderr, "system error: %s\n", strerror(error));
        break;
    case WFT_STATE_DNS_ERROR:
        fprintf(stderr, "DNS error: %s\n", gai_strerror(error));
        break;
    case WFT_STATE_SSL_ERROR:
        fprintf(stderr, "SSL error: %d\n", error);
        break;
    case WFT_STATE_TASK_ERROR:
        fprintf(stderr, "Task error: %d\n", error);
        break;
    case WFT_STATE_SUCCESS:
        break;
    }

    if (state != WFT_STATE_SUCCESS)
    {
        fprintf(stderr, "Failed. Press Ctrl-C to exit.\n");
        return;
    }
    size_t body_len = 0;
    const void *body = nullptr;
    std::string name;
    std::string value;
    int statuscode = strncmp(resp->get_status_code(), "200", 3);
    int reason_phrase = strncmp(resp->get_reason_phrase(), "OK", 2);
    if (statuscode != 0 || reason_phrase != 0)
    {
        fprintf(stderr, "%s\r\n", "status code OR reason_phrase ERROR");
        return;
    }
    auto uri = req->get_request_uri();
    fprintf(stderr, "entry to req_resp\r\n");
    size_t Uri_len = strlen(uri);
    resp->get_parsed_body(&body, &body_len);
    fprintf(stderr, "@@ body size is %ld\r\n", body_len);
    fwrite(body, 1, body_len, stderr);
    fflush(stderr);
    fprintf(stderr, "http req web URI  %s   %d \r\n", uri, Uri_len);

    /**
     * @todo parsering message from body with json formates
     * and 获取msg中间的信息来判断直播间数据 如果数据合适
     *
     */
    Document StatusJson;
    StatusJson.Parse((const char *)body, body_len);
    fprintf(stderr, "Parsered Done");
    if (!StatusJson.HasMember("msg"))
    {
        fprintf(stderr, "Json Parser Error");
        return;
    }
    assert(StatusJson["msg"].IsString());
    const char *msg(StatusJson["msg"].GetString());
    fprintf(stderr, "Msg Status is  %s", msg);
    if (strncmp(msg, "ok", 2) != 0)
    {
        fprintf(stderr, "Live Room Status Error , Msg not OK");
        return;
    }
    assert(StatusJson["data"].IsObject());
    auto DataObj = StatusJson["data"].GetObject();
    uint64_t RoomId = DataObj["room_id"].GetUint64();
    int64_t Live_time = DataObj["live_time"].GetInt64();
    int LiveStatus = DataObj["live_status"].GetInt();
    bool is_locked = DataObj["is_locked"].GetBool();
    if (is_locked == true)
        return;
    for (auto &i : liveroom_list)
    {
        if (i.RoomId == RoomId)
        {
            i.live_time = Live_time;
            i.live_status = LiveStatus;
        }
        return;
    }
}

static WFFacilities::WaitGroup wait_group(1);

void Listening_liveroom_init()
{
    // auto dir = opendir("~/BLD");
    fprintf(stderr, default_profile_json.c_str());
    auto file = fopen(default_profile_json.c_str(), "r");
    if(file==NULL)
    {
        fprintf(stderr , "read profile error\r\n");
        exit(-1);
        return;

    }
    struct stat file_stat;
    memset(&file_stat , 0 , sizeof(  file_stat));
    int _fd = fileno(file);
    if(_fd==-1){
        fprintf(stderr,"Open profile Error");
        exit(-1);
    }
    fstat(_fd,&file_stat);
    size_t Fsiz =  (size_t)file_stat.st_size;
    fprintf(stderr , "profile size is %ld\n\r",Fsiz);

    void* block = malloc(Fsiz);
    int readtimes = 10;
    while(Fsiz!=0 and  readtimes>0)
    {
        readtimes--;
        size_t FullReadSiz =  fread(block,Fsiz,1,file);
        fflush(file);
        
        fprintf(stderr," readfile and read size is %ld\r\n",FullReadSiz);
        if(FullReadSiz==1)break;
        Fsiz-=FullReadSiz;
    }
    fprintf(stderr,"LivingRoomMessage Readed to  Mem\r\n");
    fprintf(stderr,(char*)block);
    
     
}

void GetliveStatus(const char *Liveaddr)
{

    std::string website = web_live_status + "?id=" + Liveaddr;
    std::cout << "website add is " << website << std::endl;

    auto task = WFTaskFactory::create_http_task(website, REDIRECT_MAX, RETRY_MAX, fetch_live_status_callback);
    auto req = task->get_req();
    req->add_header_pair("Accept", "*/*");
    req->add_header_pair("User-Agent", "Mozilla/5.0");
    req->add_header_pair("Connection", "close");
    task->start();
    sleep(5);
    return;
}

const std::string RoomUrlInfo = "https://api.live.bilibili.com/xlive/web-room/v2/index/getRoomPlayInfo";

// void live_room_website_call_back(WFHttpTask *task);

void LivingRoomIndexAnalysis()
{
    for (auto &i : liveroom_list)
    {
        if (i.live_status == true)
        {
            if (i.LivingRoomExt == nullptr)
            {
                i.LivingRoomExt = new LivingRoomIndex();
            }
            std::string website;
            website.resize(RoomUrlInfo.size() + 50);
            char buff[50];
            memset(buff, 0, 50);
            sprintf(buff, "?room_id=%lld&protocol=0,1&format=0,1,2&codec=0,1&qn=10000&platform=h5&ptype=8", i.RoomId);
            website = RoomUrlInfo + buff;
            auto Task = WFTaskFactory::create_http_task(website, 5, 2, [=](WFHttpTask *task)
                                                        {
    protocol::HttpRequest *req = task->get_req();
    protocol::HttpResponse *resp = task->get_resp();
    int state = task->get_state();
    int error = task->get_error();

    switch (state)
    {
    case WFT_STATE_SYS_ERROR:
        fprintf(stderr, "system error: %s\n", strerror(error));
        break;
    case WFT_STATE_DNS_ERROR:
        fprintf(stderr, "DNS error: %s\n", gai_strerror(error));
        break;
    case WFT_STATE_SSL_ERROR:
        fprintf(stderr, "SSL error: %d\n", error);
        break;
    case WFT_STATE_TASK_ERROR:
        fprintf(stderr, "Task error: %d\n", error);
        break;
    case WFT_STATE_SUCCESS:
        break;
    }

    if (state != WFT_STATE_SUCCESS)
    {
        fprintf(stderr, "Failed. Press Ctrl-C to exit.\n");
        return;
    }
                                                            
                                                            uint64_t RoomId = i.RoomId;
                                                            for(auto &  i : liveroom_list)
                                                            {
                                                                if(i.RoomId==RoomId)
                                                                {
                                                                    auto webref = i.LivingRoomExt;
                                                                    const void* body;
                                                                    size_t body_len;
                                                                    bool flg= resp->get_parsed_body(&body,&body_len);
                                                                    if(flg==false)return;
                                                                    Document webdesc;
                                                                    webdesc.Parse((const char*)body,body_len);
                                                                    fprintf(stderr,(const char*)body);

                                                                    return;
                                                                }
                                                            } });
            auto req = Task->get_req();
            req->add_header_pair("Accept", "*/*");
            req->add_header_pair("User-Agent", "Mozilla/5.0");
            req->add_header_pair("Connection", "close");
            Task->start();
        }
    }
}