#include "m3u8fetch.h"
#include <string_view>
#include <string.h>
#include <sstream>
#include <thread>
#include <iostream>
#include <memory>

constexpr size_t unusedLen = strlen("#EXTM3U\n#EXT-X-VERSION:7\n#EXT-X-START:TIME-OFFSET=0\n");
constexpr int TargDurLen = strlen("#EXT-X-TARGETDURATION");
constexpr int MapUrILen = strlen("#EXT-X-MAP:URI");
constexpr int EXTINF = strlen("#EXTINF:");

m3u8fetch::m3u8fetch(LiveHomeStatus *parent) : _Parent(parent), Exec_time(1000), KExecutor(&Default_ExecutorManager)
{
    // FetchM3u8Task = WFTaskFactory::create_http_task()
    fprintf(stderr, "  ### ### ###   m3u8 analysisd");
    this->SetFetchTask();
    assert(_task != nullptr);
    RegisterExecutor();
}

void m3u8fetch::GetHeadfile()
{
    /**
     * @brief 下载headfile 并且存入EXT_X_MAP 长度保存到 EXT_X_MAP_len
     *
     *
     */
}

void m3u8fetch::RegisterExecutor()
{
    fprintf(stderr, "m3u8fetch auto task start\n");
    this->resetUri();
    auto tsk = [=]()
    {
        // TODO 获取m3u8文件
        fprintf(stderr, "m3u8 http task start\n");
        if (this->_task != nullptr)
        {
            this->free_task();
            
        }
        this->SetFetchTask();
        this->_task->start();
    };
    KExecutor::SetTask(tsk);
    UploadNode();
    // KExecutor::S
    // UploadNode(tsk);
}

int m3u8fetch::Parserm3u8(char *ptr, size_t len)
{
    std::string line;
    std::stringstream ss;
    char *charptr = ptr;
    // printf("BUFF is \n%s  \n", charptr);
    fflush(stdout);
    // fprintf(stderr, charptr);
    assert(strncmp(charptr, "#EXTM3U", strlen("#EXTM3U")) == 0);
    charptr = charptr + unusedLen;
    ss << std::string(charptr);
    // printf("BUFF is \n%s  \n",charptr);
    ss >> line;
    uint64_t SeqId = 0;
    sscanf(line.c_str(), "#EXT-X-MEDIA-SEQUENCE:%lld", &SeqId);
    std::cout << "\n seq  id is " << SeqId << std::endl;

#ifdef Debug
    // goto UpdateM4slist;
#endif
    if (SeqId < CurrentM3u8file.SeqId)
        return -1;
    if (SeqId == CurrentM3u8file.SeqId)
    { // 检查文件长度来判断是否需要解析更新map
        if (len > CurrentM3u8file.FileSize)
        {
            CurrentM3u8file.FileSize = len;
            goto UpdateM4slist;
        }
        else
        {
            return 0;
        }
    }
    if (SeqId > CurrentM3u8file.SeqId)
    {
        CurrentM3u8file.SeqId = SeqId;
        CurrentM3u8file.FileSize = len;
        goto UpdateM4slist;
    }
    /**
     * @todo 循环处理  查找所有m4s文件并获取int数值
     *
     */
UpdateM4slist:

    uint64_t m4sId = 0;
    while (ss >> line)
    {
        std::string_view sbstr(line);
        // std::cout<<"line is "<<sbstr<<std::endl;

        if (sbstr.substr(0, EXTINF) == std::string_view("#EXTINF:").substr(0, EXTINF))
        {
            // std::cout<<"\nfind a # EXT_INF"<<std::endl;
            ss >> line;
            sscanf(line.c_str(), "%lld.m4s", &m4sId);
            // fprintf(stderr, "m4s is %lld", m4sId);
            std::cout << "m4s is" << m4sId << ".m4s" << std::endl;
            BLOCK empty;
            empty.first = nullptr;
            empty.second = 0;
            // auto & [ref , inserOk]= m4slist.try_emplace(m4sId, std::make_pair<void* , size_t>(nullptr,0));
            std::lock_guard LG(mtx_m4s);
            m4slist.try_emplace(m4sId, std::move(empty));
            // if(*flag)
            continue;
        }

        if (sbstr.substr(0, TargDurLen) == (std::string_view("#EXT-X-TARGETDURATION").substr(0, TargDurLen)))
        {
            continue;
        }
        //
        // std::cout << "uri is " << sbstr.substr(8) << std::endl;
        if (sbstr.substr(0, MapUrILen) == (std::string_view("#EXT-X-MAP:URI")).substr(0, MapUrILen))
        {
            // continue;
            // std::cout << "#######find a ext map uri" << std::endl;
            // ss>>line;
            char buffs[40] = {0};
            sscanf(line.c_str(), "#EXT-X-MAP:URI=\"%s", buffs);
            if (buffs[strlen(buffs) - 1] == '\"')
            {
                buffs[strlen(buffs) - 1] = 0;
            }
            std::string headfile(buffs);
            CurrentM3u8file.headFile = std::move(headfile);
            printf("BUFFis%s\n", buffs);
            continue;
        }
    }
#ifdef Debug
    std::cout << "Current M3u8 parserd " << CurrentM3u8file.headFile << " end" << std::endl;
#endif
    return 0;
}

#if 0
void m3u8fetch::TaskM3u8Fetch()
{
    // this->FetchM3u8Task =  WFTaskFactory::create_timer_task(Exec_time ,[]() );
    if (this->_Parent->live_status != 1)
        return;
    std::string M3u8Url = this->_Parent->GetM3u8Url();
}
#endif

size_t SymbleSplite::GetLine()
{
    return result_list.size();
}

std::string_view *SymbleSplite::GetNextView()
{
    if (_CurrentLine < GetLine())
    {
        return &result_list[_CurrentLine++];
    }
    else
    {
        return nullptr;
    }
}
void SymbleSplite::splitbychar(char _chr)
{
    // 分支处理
    size_t _sym_ptr = 0;
    size_t _ptr_tail = 0;
    result_list.clear();
    const char *src = _Src_Temp.data();
    // TODO 解决分隔符连续的情况
    for (char ver : _Src_Temp)
    {
        if (ver == _chr)
        {
            result_list.push_back(std::string_view(src + _sym_ptr, _ptr_tail));
            std::cout << std::string(src + _sym_ptr, _ptr_tail) << std::endl;
            _sym_ptr = _ptr_tail + 1;
        }
        _ptr_tail++;
    }
    // TODO ： 解决不存在分隔符以及最后一个字符是分隔符的情况；
    result_list.push_back(std::string_view(src + _sym_ptr, _ptr_tail));
    return;
}

/**
 * @brief 用于获取并解析m3u8文件
 *
 * @return int
 */
int m3u8fetch::SetFetchTask()
{
    fprintf(stderr, "SetFetchTask\n");
    _task = WFTaskFactory::create_http_task(Url_m3u8, 5, 2, [=](WFHttpTask *task)
                                            {
                                                    // fprintf(stderr, "----------start to slove m3u8file-----------\n");

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
                                                    // fprintf(stderr, "----------start to slove m3u8file---jj--------\n");
                                                    fflush(stderr);
                                                    const void *body = nullptr;
                                                    size_t body_len = 0;
                                                    resp->get_parsed_body(&body, &body_len);
                                                    void *Content = malloc(body_len);
                                                    assert(body_len != 0);
                                                    mempcpy(Content, body, body_len);
                                                    Parserm3u8((char*)Content , body_len);
                                                    free(Content);
                                                    fprintf(stderr , "stop at parser m3u8 URL");
                                                    this->_Parent->TransUnit->StartOnce();
                                                    /**
                                                     * @todo 获取json信息以后将信息
                                                     *
                                                     */
                                                    // auto gotask_callback = [=](WFGoTask* gotask){
                                                    //     fprintf(stderr , "go task call back start \n");
                                                    //     this->_Parent->TransUnit->StartOnce();
                                                    // };
                                                    #if 0
                                                    auto *gotask = WFTaskFactory::create_go_task("parser_m3u8", [=]()
                                                                                                 {
                                                        Parserm3u8((char*)Content , body_len);
                                                        free(Content); 
                                                        fprintf(stderr , "go task call back start \n");
                                                        this->_Parent->TransUnit->StartOnce();
                                                        
                                                        });
                                                    #endif
                                                        // gotask->set_callback(gotask_callback);
                                                    });
    // TODO: create go task and append to httptask;
    protocol::HttpRequest *req = _task->get_req();
    req->add_header_pair("Accept", "*/*");
    req->add_header_pair("User-Agent", "Wget/1.14 (linux-gnu)");
    req->add_header_pair("Connection", "close");
}

std::deque<m3u8fetch::BlockPair> m3u8fetch::PopFrontM4sList()
{
#ifdef Debug
// fprintf(stderr , "start to init m4slist");
#endif
    std::deque<BlockPair> retvalue;
    std::lock_guard list_lock(mtx_m4s);
    for (auto &&ref : m4slist)
    {
        retvalue.emplace_back(std::make_pair(ref.first, ref.second));
    }
    m4slist.clear();
    return std::move(retvalue);
}