#include "fetch_live_status.h"
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <string_view>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <string.h>
#include <rapidjson/pointer.h>
using namespace rapidjson;
std::deque<LiveHomeStatus> liveroom_list;
const std::string web_live_status("https://api.live.bilibili.com/room/v1/Room/room_init");
char *HOME = getenv("HOME");
std::string default_profile_json = std::string(HOME) + "/.BiliLiveDown.json";

std::vector<M4SVideo> m4slist;

size_t
MergeChunkedBody(void *_body, size_t &_body_len)
{
    fprintf(stderr, "Entry to MergeChunck");
    size_t ret_siz = 0;
    char *body_chr = (char *)_body;
    // AllocFlag = realloc(dest,MemSize);
    int length = 0;
    int TutalLength = 0;
    auto File = fopen("body.json", "w+");
    do
    {
        length=0;

        int ptr = 0;
        assert(body_chr[0]!='\n');
        do
        {
            char c = body_chr[ptr];
            if (c == '\n')
            {
                ptr++;
                break;
            }
            ptr++;

        } while (ptr < _body_len - TutalLength);
        std::string sv(body_chr, ptr - 2);
        fprintf(stderr, "\r\n #### Number of length %d  and ptr is %d   ###\r\n", sv.length(),ptr);
        sscanf(sv.c_str(), "%X", &length);
        fprintf(stderr, "\r\n #### Length is %d ###\r\n",length);

        if (length == 0)
            break;
        fprintf(stderr, "\nmove lenth is %d\r\n", length);
        memmove((void *)((char*)_body+TutalLength), (const void *)body_chr + ptr, length);
        fwrite(body_chr, length, 1, File);
        fflush(File);

        // assert(body_chr[0] == '{');
        body_chr += (length + ptr+2);
        // body_chr+=ptr+length;
        TutalLength += length;

    } while (length != 0);
    fclose(File);
    _body_len = TutalLength;

    return _body_len;
}

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
    // fprintf(stderr, "Parsered Done");
    if (!StatusJson.HasMember("msg"))
    {
        // fprintf(stderr, "Json Parser Error");
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
    if (file == NULL)
    {
        fprintf(stderr, "read profile error\r\n");
        exit(-1);
        return;
    }
    struct stat file_stat;
    memset(&file_stat, 0, sizeof(file_stat));
    int _fd = fileno(file);
    if (_fd == -1)
    {
        fprintf(stderr, "Open profile Error");
        exit(-1);
    }
    fstat(_fd, &file_stat);
    size_t Fsiz = (size_t)file_stat.st_size;
    fprintf(stderr, "profile size is %ld\n\r", Fsiz);

    void *block = malloc(Fsiz);
    int readtimes = 10;
    while (Fsiz != 0 and readtimes > 0)
    {
        readtimes--;
        size_t FullReadSiz = fread(block, Fsiz, 1, file);
        fflush(file);

        fprintf(stderr, " readfile and read size is %ld\r\n", FullReadSiz);
        if (FullReadSiz == 1)
            break;
        Fsiz -= FullReadSiz;
    }
    fprintf(stderr, "LivingRoomMessage Readed to  Mem\r\n");
    fprintf(stderr, (char *)block);
    /**
     * @todo 实现对Json的解析 填充liveroom_list
     */
    Document Parsed_json;
    //   fprintf(stderr,"ENTRY to Parser\r\n");
    fflush(stderr);
    Parsed_json.Parse((char *)block, Fsiz);
    // fprintf(stderr,"ENTRY to Parser\r\n");
    if (!Parsed_json.HasMember("liveRoom"))
    {
        fprintf(stderr, "line in %c  has error,profile json parse\r\n", __LINE__);
        exit(-1);
    }
    // assert(Parsed_json.HasMember("liveRoom"));
    auto roomlists = Parsed_json["liveRoom"].GetArray();
    // fprintf(stderr,"#ENTRY to Parser\r\n");

    for (auto &Room : roomlists)
    {
        assert(Room.HasMember("roomid") && Room.IsObject());
        fprintf(stderr, "##ENTRY to Parser\r\n");

        uint64_t Int_Roomid = Room["Introomid"].GetInt64();
        const char *chr_roomid = Room["roomid"].GetString();
        LiveHomeStatus rid;
        rid.RoomId = Int_Roomid;
        memcpy(rid.RoomId_chr, chr_roomid, strlen(chr_roomid));
        liveroom_list.push_back(std::move(rid));
    }
    fprintf(stderr, "Tutoal liveroom conunt is %d\r\n", liveroom_list.size());
    // fprintf(stderr,"End Of Get Live Room \r\n");
    free(block);
}

void GetliveStatus(const char *Liveaddr)
{

    std::string website = web_live_status + "?id=" + Liveaddr;
    // std::cout << "website add is " << website << std::endl;

    auto task = WFTaskFactory::create_http_task(website, REDIRECT_MAX, RETRY_MAX, fetch_live_status_callback);
    // auto task = WFTaskFactory::create_http_task()
    auto req = task->get_req();
    req->add_header_pair("Accept", "*/*");
    req->add_header_pair("User-Agent", "Mozilla/5.0");
    req->add_header_pair("Connection", "close");
    task->start();
    return;
}

void UpdateRoomListMsg()
{

    for (auto &RoomMsg : liveroom_list)
    {
        auto task_callback = [&](WFHttpTask *task)
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
            // fprintf(stderr, "Parsered Done");
            if (!StatusJson.HasMember("msg"))
            {
                // fprintf(stderr, "Json Parser Error");
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
            int short_id = DataObj["short_id"].GetInt();
            int64_t Live_time = DataObj["live_time"].GetInt64();
            int LiveStatus = DataObj["live_status"].GetInt();
            bool is_locked = DataObj["is_locked"].GetBool();
            if (is_locked == true)
                return;
            if (short_id != 0)
                RoomId = short_id;
            assert(RoomMsg.RoomId == RoomId);
            RoomMsg.live_status = LiveStatus;
            if (LiveStatus == 1)
            {
                fprintf(stderr, "Current Room %c is Running\r\n", RoomMsg.RoomId_chr);
            }
            // for (auto &i : liveroom_list)
            // {
            //     if (i.RoomId == RoomId)
            //     {
            //         i.live_time = Live_time;
            //         i.live_status = LiveStatus;
            //     }
            //     return;
            // }
        };
        std::string website = web_live_status + "?id=" + RoomMsg.RoomId_chr;
        std::cout << "website add is " << website << std::endl;

        auto task = WFTaskFactory::create_http_task(website, REDIRECT_MAX, RETRY_MAX, task_callback);
        // auto task = WFTaskFactory::create_http_task()
        auto req = task->get_req();
        req->add_header_pair("Accept", "*/*");
        req->add_header_pair("User-Agent", "Mozilla/5.0");
        req->add_header_pair("Connection", "close");
        task->start();
    }

    return;
}

const std::string RoomUrlInfo = "https://api.live.bilibili.com/xlive/web-room/v2/index/getRoomPlayInfo";

// void live_room_website_call_back(WFHttpTask *task);

void LivingRoomIndexAnalysis()
{
    fprintf(stderr, "\r\nEntry to Room ANA\r\n");
    for (auto &i : liveroom_list)
    {
        if (i.live_status == 1)
        {
            if (i.LivingRoomExt == nullptr)
            {
                i.LivingRoomExt = new LivingRoomIndex();
            }
            std::string website;
            website.resize(RoomUrlInfo.size() + 256);
            char buff[256];
            memset(buff, 0, 256);
            sprintf(buff, "?room_id=%lld&protocol=0,1&format=0,1,2&codec=0,1&qn=10000&platform=h5&ptype=8", i.RoomId);
            website = RoomUrlInfo + buff;

            /**
             * @brief register callback function
             * 实现自动更新livingroom数据
             *
             */

            auto Task = WFTaskFactory::create_http_task(website, 5, 2, [&](WFHttpTask *task)
{
    if(i.live_status==1){fprintf(stderr,"\r\nRoom %s is Running\r\n");}
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
    // uint64_t RoomId = i.RoomId;
        
        // fprintf(stderr,"Match TO CURRET ROOM\r\n");
        auto webref = i.LivingRoomExt;
        const void* body;
        const void* rawbody;
        size_t rawbody_len;
        size_t body_len;
        resp->get_raw_body(&rawbody,&rawbody_len);
        bool flg= resp->get_parsed_body( &body,&body_len);


        auto rawbodyft  =fopen("rawbody.txt","w+");
        fwrite(rawbody, 1, rawbody_len, rawbodyft);
        fflush(rawbodyft);
        fclose(rawbodyft);

        std::string name , value;
        protocol::HttpHeaderCursor resp_cursor(resp);
        bool ThunckFlag=false;
        while (resp_cursor.next(name, value))
        {
            if(strncmp("Transfer-Encoding" , name.c_str() , strlen("Transfer-Encoding"))==0
            && strncmp("chunked",value.c_str(),strlen("chunked"))==0)
            {
                ThunckFlag=true;
            }
        }
        fprintf(stderr, "\r\n#############################\r\n");
        int ThunckLen=0;
        if(ThunckFlag==true)
        {
            ThunckFlag= MergeChunkedBody((void*)body , body_len);
        }

        if(flg==false)return;
        auto FT  =fopen("fetch_file.txt","w+");
                // fprintf(stderr,body+5);
        fwrite(body, body_len, 1, FT );
        fflush(FT);
        fclose(FT);

        fprintf(stderr,"\n###PRINT BODY END  body len is %lld\n",body_len);
        Document webdesc;
        webdesc.Parse((const char*)body,body_len);
        // fprintf(stderr,"\n###PARSER DONE  \r\n  ");
        
        assert(webdesc.IsObject() );
        // fprintf(stderr , "\nWebDesc is OBJ\n");
        // fprintf(stderr,"\r\n web dict is  %d \r\n" ,Web_Dict.GetType());
        /**
         * @todo 解析webdesc的网站内容并且填充liveroomlist
         * 
         */
        auto  LiveStream = rapidjson::Pointer("/data/playurl_info/playurl/stream").Get(webdesc);
        assert(LiveStream->IsArray());
        fprintf(stderr,"\nENTRY TO Pointer\n");

        for(auto  & Stream : LiveStream->GetArray() )
        {
            assert(Stream.HasMember("protocol_name"));
            fprintf(stderr,"\nASSERT PROTO NAME\n");

            std::string ProtoName =  Stream["protocol_name"].GetString();
            if(strncmp(ProtoName.c_str(),"http_hls",strlen("http_hls"))!=0)continue;
            for(auto & format : Stream["format"].GetArray())
            {
                fprintf(stderr,"\nENTRY Format name\n");
                // assert(Stream)
                auto formatName =  format["format_name"].GetString();

                fprintf(stderr,"\n  formate name is %s\n",formatName);

                if(strncmp(formatName  ,"fmp4",strlen("fmp4"))!=0)continue;
                fprintf(stderr,"\nCODEC\n");
                
                for(auto & codeEle : format["codec"].GetArray())
                {
                    fprintf(stderr,"\nENTRY TO CODEC\n");

                    auto codecname = codeEle["codec_name"].GetString();
                    if(strncmp(codecname,"avc",strlen("avc"))!=0)continue;
                    std::string UrlBase = codeEle["base_url"].GetString();
                    fprintf(stderr,"\n######    URLBASE is #####%s###\n",UrlBase.c_str());


                }


                

            }


        }
        // i.LivingRoomExt
        return; 
        });
            auto req = Task->get_req();
            req->add_header_pair("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36 Edg/116.0.1938.62");
            req->add_header_pair("Connection", "close");
            Task->start();
        }
    }
    sleep(2);
}