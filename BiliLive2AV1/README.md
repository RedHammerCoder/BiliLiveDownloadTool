# BiliLive2AV1 开发LOG
## desc : 将bilibili直播下载下来转换为av1编码
## TODOLIST
### https
1. 建立维护https接口 实现使用https 与bilibili建立链接关系
2. workflow 库 简化轮子操作
3. 记录分析bilibili api 的参数以及对应影响 

### regex
1. 正则表达式 
 - 引入正则表达式 实现对http信息的处理
2.  json解析需要引入相关库文件
3. 

### process
1. 获取bilibili直播间号码 并且获取直播间信息  DONE
2. 使用对应cookie 实现登录网站并且获取最高画质信息  
    - 不需要实现 下载bilibili直播不需要登陆
3. 使用最高画质下载对应的u3m8 DONE **画质选项设置为1000**
4. 使用u3m8 组建对应网址， 使用https实现视频的下载
    - 使用workflow 实现的create_http_task来实现http下载  ?\*\*=\*\** 需要手动实现
5. 将下载下来的的视频片段写入本地并且使用ffmpeg转码为整体视频
6. 实现time trigger  实现定时检查直播间是否开播

## COOKIE
#### https://github.com/withsalt/BilibiliLiveTools/tree/master
#### https://api.bilibili.com/x/web-interface/nav
- 打开之后，会看到一大串Json，不用管内容。然后点击右侧的nav（序号3），将图中序号4中cookie中的值拷贝出来，粘贴到程序目录下面的cookie.txt文件即可。
- 点击进入视频会收到 passport.bilibili.com

-  https://passport.bilibili.com/x/passport-login/web/cookie/info?csrf=aa0213079ddd7e9ff5fb24987a75368d

## json path
- array  /data/playurl_info/playurl/stream/  
- get obj from array then for loop 
- where protocal_name == "fmp4" then
- current_qn==10000 and codec_name == "avc" 
- base_url = "/live-bvc/374127/live_11332884_9386100_bluray/index.m3u8?"
-  obj  url_info is array {host extra stream_ttl}


## M3u8 REQUEST
- host url  
https://cn-jssz-cm-02-13.bilivideo.com   

- base url  
/live-bvc/932497/live_20544502_54782404/index.m3u8?   

- extra msg  
expires=1694509696&len=0&oi=979675362&pt=h5&qn=10000&trid=1007309c1814faaa4fd08e88e6c0d36276bb&sigparams=cdn,expires,len,oi,pt,qn,trid&cdn=cn-gotcha01&sign=6d8a43b968aa6b2730a006e6ca3679a7&sk=0468e42074e5ce87705794bafa5056a1&flvsk=03519ba8544fcc8a04836503e61b74d4&p2p_type=0&sl=1&free_type=0&mid=0&sid=cn-jssz-cm-02-13&chash=1&bmt=1&sche=ban&bvchls=1&score=3&pp=rtmp&source=onetier&trace=120d&site=33fcdd21ef4d7d28827505bf9a0c02d9&order=1


## M4s REQUEST
- host url   
https://cn-jssz-cm-02-13.bilivideo.com   

- base url  
/live-bvc/899070/live_20544502_54782404_1500/53539542.m4s?   

- extra msg  
expires=1694509597&len=0&oi=979675362&pt=web&qn=0&trid=1007a8272a0f15a54795a784840cdfd13be9&sigparams=cdn,expires,len,oi,pt,qn,trid&cdn=cn-gotcha01&sign=6f30cb3481e9724f5be2e5404f22b441&sk=e028b496f74740320894a15b4352907b&flvsk=8ae0bc2920e1216c95114a4c891b5ab3&p2p_type=1&sl=1&free_type=0&mid=181473946&sid=cn-jssz-cm-02-13&chash=1&bmt=1&sche=ban&bvchls=1&score=3&pp=rtmp&source=onetier&trace=120d&site=4143600621e22b184390036b425361e5&order=1




https://d1--cn-gotcha208.bilivideo.com/live-bvc/568137/live_322892_3999292_1500/53547635.m4s?expires=1694517689&len=0&oi=979675362&pt=web&qn=0&trid=1007c2d42c6c69d74f03a8cf789f14c8a3f3&sigparams=cdn,expires,len,oi,pt,qn,trid&cdn=cn-gotcha208&sign=eb43db6484c7536eb61a8ffc766983c1&sk=657866af8fc16d8b2b84dfdde3e981438dff44b02a5b137933388fa4fca4c196&p2p_type=1&sl=3&free_type=0&mid=181473946&pp=rtmp&source=onetier&trace=4&site=d24c4467b5a4cbe5236a70b458712993&order=1