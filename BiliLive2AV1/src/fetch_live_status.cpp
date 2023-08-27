#include "fetch_live_status.h"
#include <string.h>
#include <iostream>
using namespace rapidjson;

const std::string web_live_status("https://api.live.bilibili.com/room/v1/Room/room_init");


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
    fprintf(stderr,"@@ body size is %d\r\n",body_len);
    fwrite(body, 1, body_len, stderr);
    fflush(stderr);
    fprintf(stderr, "http req web URI  %s   %d \r\n", uri,Uri_len);


    /**
     * @todo parsering message from body with json formates
     * and 获取msg中间的信息来判断直播间数据 如果数据合适
     *
     */
    Document StatusJson;
    StatusJson.Parse((const char *)body, body_len);
    fprintf(stderr,"Parsered Done");
    if (!StatusJson.HasMember("msg"))
    {
        fprintf(stderr, "Json Parser Error");
        return;
    }
    assert(StatusJson["msg"].IsString());
    const char *msg(StatusJson["msg"].GetString());
    if (strncmp(msg, "ok", 2) != 0)
    {
        fprintf(stderr, "Live Room Status Error , Msg not OK");
        return;
    }
    assert(StatusJson["data"].IsObject());
    auto DataObj = StatusJson["data"].GetObject();                                                                        
}

static WFFacilities::WaitGroup wait_group(1);

LiveHomeStatus GetliveStatus(const char *Liveaddr)
{
    LiveHomeStatus Status;
    memset(&Status, 0, sizeof(LiveHomeStatus));
    char JsonBuff[256];
    memset(JsonBuff, 0, 256);
    std::string website = web_live_status + "?id=" + Liveaddr;
    std::cout<<"website add is "<<website<<std::endl;

    auto task = WFTaskFactory::create_http_task(website, REDIRECT_MAX, RETRY_MAX, fetch_live_status_callback);
    auto req=task->get_req();
    req->add_header_pair("Accept", "*/*");
    req->add_header_pair("User-Agent", "Mozilla/5.0");
    req->add_header_pair("Connection", "close");
    task->start();
    wait_group.wait();
    return Status;
}
