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

