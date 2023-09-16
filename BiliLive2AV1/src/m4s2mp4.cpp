#include "m4s2mp4.h"
#include "m3u8fetch.h"
#include <assert.h>
#include <atomic>
#include <sys/stat.h>
#include <sys/types.h>
#include <workflow/WFFacilities.h>
#include <iostream>

std::string Default_Path;
void SetDefaultPath(std::string Path)
{
    Default_Path = std::move(Path);
}

void m4s2mp4::Start()
{
    fprintf(stderr, "#################------------- m4smp4 start\n");

    _task = [=]()
    {
        fprintf(stderr, "###----m4s2mp4 ERROR\n");
        fflush(stderr);
        this->GetM4sList();
        this->AppendMsgBlock();
        fprintf(stderr, "-------##########---------########  append block to disk\n");
    };

    if (InitFlag == false)
    {
        InitFile();
        KExecutor::SetTask(_task);
        KExecutor::UploadNode();
    }

    fprintf(stderr, "ready to update node\n");
}

void m4s2mp4::StartOnce()
{
    bool flg = Atmc_Startonce.exchange(true);
    if (flg == true){
        fprintf(stderr , "exit from startonce\n");
        return;
    };
        
    Start();
}

/**
 * @brief 用于创建文件夹或检查文件夹已经存在
 *
 * @param path
 * @return int retvalue ==0 mean 有文件夹或者文件夹已经创建 其他数值代表异常
 */
int TryCreateDir(std::string path)
{
recheck:
    int ret = 0;
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr)
    {
        // TODO: create dir
        mkdir(path.c_str(), S_IRWXU);
        goto recheck;
    }
    else
    {
        return 0;
    }
}

void m4s2mp4::InitFile()
{
    // if (file == nullptr)
    assert(InitFlag == false);
    assert(file == nullptr);
    assert(_m4s_filename.size() != 0);
    // mkdir(_m4s_dir.c_str(),S_IRWXU);
    std::string Path = Default_Path + '/' + _m4s_dir;
    int flag = TryCreateDir(Path);
    if (flag != 0)
    {
        fprintf(stderr, "create dir error \n");
        return;
    }
    Path += '/' + _m4s_filename;
    fprintf(stderr, "file path is %s \n ", Path.c_str());
    file = fopen(Path.c_str(), "w+");
    if (fileno(file) == -1)
    {
        fprintf(stderr, "mp4 file open err\n");
        exit(-1);
    }
    const void *ptr = nullptr;
    size_t ptr_len = 0;
    std::string m4sheader = this->m3u8list->GetHeaderFileName();
    // fprintf(stderr ,)
    assert(m4sheader.size() != 0);
    std::string header_url = this->LiveStatus->GetM4sContent(m4sheader);
    int state = FetchHttpBody(header_url, &ptr, &ptr_len);
    if (state != WFT_STATE_SUCCESS)
    {
        fprintf(stderr, "Get header file error \n");
        return;
    }
    size_t siz = fwrite(ptr, ptr_len, 1, file);
    fprintf(stderr, "---------ALL WRITE %lld byte   ptr len is %lld\n", siz, ptr_len);
    fflush(file);
    free((void*)ptr);
    InitFlag = true;
}

void m4s2mp4::GetM4sList()
{
    fprintf(stderr , "entry to Getm4slist\n");
    assert(this->m3u8list != nullptr);
    std::lock_guard _m4s_list_mtx_lock(_m4s_list_mtx);
    // std::lock_guard _m3u8list_lock(m3u8list.)
    std::deque<m3u8fetch::BlockPair> m4slist;
    m4slist = this->m3u8list->PopFrontM4sList();

    std::atomic_int64_t RemainTask_nb=0; // 用于记录正在执行的http req数目

    WFFacilities::WaitGroup wg(1);
    for (auto &ref : m4slist)
    {
        /**
         * @todo : 遍历m4slist 并且依据id发出http req  将返回后的block 写入Block
         *
         */

        RemainTask_nb.fetch_add(1);
        auto &Kvalue = ref.second;
        uint64_t m4sId = ref.first;

        std::string URL = LiveStatus->GetM4sUrl(m4sId);
        std::cout<<URL<<std::endl;
        // fprintf(stderr , "M4s URL : %\n",URL.c_str());
        auto http_callback = [&wg, &RemainTask_nb, &Kvalue](WFHttpTask *task)
        {
            fprintf(stderr, "start to fetch m4sdoc\n");
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
            void *body = nullptr;
            size_t body_len = 0;
            resp->get_parsed_body((const void **)&body, &body_len);
            fprintf(stderr , "Block Size is %zu \n",body_len);

            if (ThunckFlag == true)
            {
                size_t len = MergeChunkedBody(body, body_len);
            }
            void *act_body = nullptr;
            act_body = malloc(body_len);
            memcpy(act_body, body, body_len);
            Kvalue.first = act_body;
            Kvalue.second = body_len;
            // 获取http数据后唤醒主线程
            uint64_t val = RemainTask_nb.fetch_sub(1);

            // if (val == 1)
            // {
            //     fprintf(stderr, "#### fetch m4s Video loop done  \n");
            //     wg.done();
            // }
        };
        WFTaskFactory::create_http_task(URL, 5, 2, http_callback)->start();
        

    }
    // wg.wait();
    sleep(5);
    // TODO: 将获取的m4s list 插入 _m4s_list
    for (auto &ref : m4slist)
    {
        auto& [ptr , len] = ref.second;
        if(ptr==nullptr && 0==len)
        {
            std::string URL = LiveStatus->GetM4sUrl(ref.first);
            int state=-1;
            do
            {
                state=FetchHttpBody(URL,(const void**)&ptr,&len);
                fprintf(stderr , "RE fetch worklist ID : %zu , len :%zu \n",ref.first,len);
                /* code */
            } while (state!=WFT_STATE_SUCCESS);
            

        }
        _m4s_list.push_back(std::move(ref.second));
    }
}

void m4s2mp4::AppendMsgBlock()
{
    assert(InitFlag == true);
    BLOCK blk;
    std::lock_guard mtx(_m4s_list_mtx);
    while (true)
    {
        if (_m4s_list.size() == 0)
            break;
        blk = _m4s_list.front();
        _m4s_list.pop_front();
        auto &[ptr, len] = blk;
        // fprintf(stderr , "Block len : %d",blk);
        fwrite(ptr, len, 1, file);
        fflush(file);
        free(ptr);
    }
}