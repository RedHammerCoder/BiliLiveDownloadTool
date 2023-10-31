// #include "m3u8fetch.h"
#include "basic_class.h"
#include <string_view>
#include <string.h>
#include <sstream>
#include <thread>
#include <iostream>
#include <memory>
#include "ErrorLog.h"
#include "fetch_live_status.h"

const char *ZeroStr = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
constexpr size_t unusedLen = strlen("#EXTM3U\n#EXT-X-VERSION:7\n#EXT-X-START:TIME-OFFSET=0\n");
constexpr int TargDurLen = strlen("#EXT-X-TARGETDURATION");
constexpr int MapUrILen = strlen("#EXT-X-MAP:URI");
constexpr int EXTINF = strlen("#EXTINF:");

m3u8fetch::m3u8fetch(LiveHomeStatus *parent) : _Parent(parent)
{
    fprintf(stderr, "  ### ### ###   m3u8 analysisd");
    _task = nullptr;
    RegisterExecutor();
}

void m3u8fetch::GetHeadfile()
{
}

void m3u8fetch::RegisterExecutor()
{
    fprintf(stderr, "m3u8fetch auto task start\n");
    auto tsk = [&]()
    {
        // TODO 获取m3u8文件
        this->resetUri();
        fprintf(stderr, "m3u8 http task start\n");
        if (this->Url_m3u8.size() == 0)
        {
            return;
        }
        const void *ptr = nullptr;
        size_t ptr_len = 0;
        int state = -1;
        do
        {
            fprintf(stderr, "do fetching url \n");
            state = FetchHttpBody(this->Url_m3u8, &ptr, &ptr_len);
        } while (state != WFT_STATE_SUCCESS);
        int stat = this->Parserm3u8((char *)ptr, ptr_len);
#if 1
        // assert(this->CurrentM3u8file.headFile.size() == strlen(ZeroStr));
        if (strncmp(this->_Parent->m4shead, ZeroStr, strlen(this->_Parent->m4shead)) == 0)
        {
            memcpy(this->_Parent->m4shead, this->CurrentM3u8file.headFile.c_str(), this->CurrentM3u8file.headFile.size());
        }
        else
        {
            if (this->CurrentM3u8file.headFile != std::string(this->_Parent->m4shead))
            {
                raise(SIGINT);
            }
        }
#endif
        fprintf(stderr, "Parserm3u8ing \n");
        if (stat != 0)
        {
            return;
        }
        assert(this->CurrentM3u8file.headFile.size()!=0);
        if(this->_Parent->TransUnit->Atmc_Startonce.load()==false)
        {
                    this->_Parent->TransUnit->StartOnce();
                    sleep(2);
        }
        // this->_Parent->LivingRoomExt->m4sTrigger.notify_all();
        notifyer.cv.notify_all();
        fprintf(stderr, "notify_one\n");

    };

    this->Exec.SetTask(std::move(tsk));
}

int m3u8fetch::Parserm3u8(char *ptr, size_t len)
{
    fprintf(stderr, "Parserm3u8 Start  \n");
    if (ptr == nullptr && len == 0)
    {
        fprintf(stderr, "Parserm3u8 Error  \n");
        fprintf(ERRLOG.Handle, "Parserm3u8 body len is zero \n");
        fflush(ERRLOG.Handle);
        return -2;
    }
    std::string line;
    std::stringstream ss;
    char *charptr = ptr;
    // printf("BUFF is \n%s  \n", charptr);
    fflush(stdout);
    // fprintf(stderr, charptr);
    if (strncmp(charptr, "#EXTM3U", strlen("#EXTM3U")) != 0)
    {
        fprintf(stderr, "u3m8 Error  \n");
        // fwrite((void *)ptr, len, 1, ERRLOG.Handle);
        // fflush(ERRLOG.Handle);
        return -2;
    }

    charptr = charptr + unusedLen;
    ss << std::string(charptr);
    // printf("BUFF is \n%s  \n",charptr);
    ss >> line;
    uint64_t SeqId = 0;
    sscanf(line.c_str(), "#EXT-X-MEDIA-SEQUENCE:%lld", &SeqId);
    std::cout << "\n seq  id is " << SeqId << std::endl;
    // if(SeqId>this->Max_m4s_nb)
    // {
    //     fprintf(ERRLOG.Handle,"m3u8 fetch too slow at %s video room once\n",this->_Parent->RoomName.c_str());
    // }

#ifdef Debug
    // goto UpdateM4slist;
#endif
    if (SeqId > this->Max_m4s_nb)
    {
        // fprintf(ERRLOG.Handle,"产生一个缺页 \n");
        // fflush(ERRLOG.Handle);
    }
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
    return 0;
    /**
     * @todo 循环处理  查找所有m4s文件并获取int数值
     *
     */
UpdateM4slist:
    std::lock_guard LG(mtx_m4s);
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

            if (m4sId <= this->Max_m4s_nb)
            {
                continue;
            }
            this->Max_m4s_nb = (this->Max_m4s_nb > m4sId ? this->Max_m4s_nb : m4sId);
            // fprintf(stderr, "m4s is %lld", m4sId);
            std::cout << "m4s is" << m4sId << ".m4s" << std::endl;
            BLOCK empty;
            empty.first = nullptr;
            empty.second = 0;
            // auto & [ref , inserOk]= m4slist.try_emplace(m4sId, std::make_pair<void* , size_t>(nullptr,0));
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

#endif

#if 0

/**
 * @brief 用于获取并解析m3u8文件
 *
 * @return int
 */
int m3u8fetch::SetFetchTask()
{
    this->free_task();
    fprintf(stderr, "SetFetchTask\n");
    if (Url_m3u8.size() == 0)
        return -1;

    auto Ktask = WFTaskFactory::create_http_task(Url_m3u8, 5, 2, [&](WFHttpTask *task)
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
                                                     if(body_len==0)
                                                     {
                                                        return ;
                                                     }
                                                     void *Content = malloc(body_len);
                                                     assert(body_len != 0);
                                                     mempcpy(Content, body, body_len);
                                                     int kid = Parserm3u8((char *)Content, body_len);
                                                     free(Content);
                                                     if (kid == -1)
                                                         return;
                                                     fprintf(stderr, "stop at parser m3u8 URL");
                                                     this->_Parent->TransUnit->StartOnce();
                                                     /**
                                                      * @todo 获取json信息以后将信息
                                                      *
                                                      */

                                                     // gotask->set_callback(gotask_callback);
                                                 });
    // TODO: create go task and append to httptask;
    fprintf(stderr, "Finish create http Task \n");

    protocol::HttpRequest *req = Ktask->get_req();
    // fprintf(stderr , "Finish create http Task \n");
    fflush(stderr);
    req->add_header_pair("Accept", "*/*");
    req->add_header_pair("User-Agent", "Wget/1.14 (linux-gnu)");
    req->add_header_pair("Connection", "close");
    _task = Ktask;

}
#endif

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