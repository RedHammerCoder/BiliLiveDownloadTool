#include "m4s2mp4.h"
#include "m3u8fetch.h"
#include <assert.h>
#include <atomic>
#include <workflow/WFFacilities.h>

std::string Default_Path;
void SetDefaultPath(std::string Path)
{
    Default_Path=std::move(Path);
}



void m4s2mp4::InitFile()
{
    if(file==nullptr);
    assert(_m4s_filename.size()!=0);
    std::string Path = Default_Path+'/'+_m4s_dir+'/'+_m4s_filename;
    file = fopen(Path.c_str(),"w+");
    const void* ptr=nullptr;
    size_t ptr_len = 0;
    std::string m4sheader = this->m3u8list->GetHeaderFileName();
    std::string header_url = this->LiveStatus->GetM4sContent(m4sheader);
    int state =  FetchHttpBody(header_url,&ptr,&ptr_len);
    if(state!=WFT_STATE_SUCCESS)
    {
        fprintf(stderr , "Get header file error \n");
        return;
    }
    fwrite(ptr,ptr_len,1,file);
}

void m4s2mp4::GetM4sList()
{
    assert(this->m3u8list != nullptr);
    std::lock_guard _m4s_list_mtx_lock(_m4s_list_mtx);
    // std::lock_guard _m3u8list_lock(m3u8list.)
    std::deque<m3u8fetch::BlockPair> m4slist;
    m4slist = this->m3u8list->PopFrontM4sList();

    std::atomic_int64_t RemainTask_nb; // 用于记录正在执行的http req数目

    WFFacilities::WaitGroup wg(1);
    for (auto &ref : m4slist)
    {
        /**
         * @todo : 遍历m4slist 并且依据id发出http req  将返回后的block 写入Block
         *
         */
        
        RemainTask_nb.fetch_add(1);
        auto& Kvalue= ref.second;
        uint64_t m4sId = ref.first;
        
        std::string URL = LiveStatus->GetM4sUrl(m4sId);
        auto http_callback = [&wg, &RemainTask_nb,&Kvalue](WFHttpTask *task)
        {
            fprintf(stderr, "start to fetch m4sdoc");
            auto req = task->get_req();
            auto resp = task->get_resp();
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
                fprintf(stderr, "Task state error \n");
                return;
            }
            bool ThunckFlag = false;
            std::string name, value;
            protocol::HttpHeaderCursor resp_cursor(resp);
            while (resp_cursor.next(name, value))
            {
                if (strncmp("Transfer-Encoding", name.c_str(), strlen("Transfer-Encoding")) == 0 && strncmp("chunked", value.c_str(), strlen("chunked")) == 0)
                {
                    ThunckFlag = true;
                }
            }
             void * body=nullptr ; size_t body_len=0;
            resp->get_parsed_body((const void **)&body , &body_len);
            if(ThunckFlag==true)
            {
                size_t len = MergeChunkedBody(body , body_len);
            }
            void* act_body=nullptr;
            act_body=malloc(body_len);
            memcpy(act_body,body,body_len);
            Kvalue.first =act_body;
            Kvalue.second =body_len;
            //获取http数据后唤醒主线程
            uint64_t val = RemainTask_nb.fetch_sub(1);
            if (val == 1)
            {
                fprintf(stderr, "#### fetch m4s Video loop done  \n");
                wg.done();
            }
        };
        WFTaskFactory::create_http_task(URL, 5, 2, http_callback);
    }
    wg.wait();
    //TODO: 将获取的m4s list 插入 _m4s_list
    for(auto & ref  : m4slist)
    {
        _m4s_list.push_back(std::move(ref.second));
    }
}



void m4s2mp4::AppendMsgBlock()
{
    assert(Inited==true);
    BLOCK blk;
    std::lock_guard mtx(_m4s_list_mtx);
    while (true)
    {
        if(_m4s_list.size()==0)break;
        blk= _m4s_list.front();
        _m4s_list.pop_front();
        auto & [ptr, len] = blk;
        fwrite(ptr,len,1,file);
    }
    
}