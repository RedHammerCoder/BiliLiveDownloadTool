# 软件架构

## m3u8 基本结构
'''txt   
#EXTM3U
#EXT-X-VERSION:7
#EXT-X-START:TIME-OFFSET=0
#EXT-X-MEDIA-SEQUENCE:52249430
#EXT-X-TARGETDURATION:2
#EXT-X-MAP:URI="h1693191788.m4s"
#EXTINF:1.03,1e455|f4589602
52249430.m4s
#EXTINF:1.00,14776|7c78b26c
52249431.m4s
#EXTINF:0.96,118a7|21e33012
52249432.m4s
#EXTINF:1.03,1a29d|f84192fe
52249433.m4s
#EXTINF:1.00,1522d|715bdd87
52249434.m4s
#EXTINF:0.96,17f69|cf761ad5
52249435.m4s
#EXTINF:1.03,201e1|2fe1d4b0
52249436.m4s

'''
##  软件结构体架构分析
1. LiveHomeStatus
    - 用于保存对应直播间的状态信息 比如是否在直播 是否封禁  以及指向下一个阶段
    - LiveHomeStatus 使用GetliveStatus进行状态更新
2. LivingRoomIndex
    - 用于保存http获取的线路信息 主要用于有筛查http_hls and fmp4 avc 10000qn
    - 使用后获取baseurl urlinfo  以及其他相关信息
3. m3u8IndexList
    - LivingRoomIndex 获取之后会获取一个网址信息 每次执行http获取对应网址之后的文件可以获得一个m3u8 文件 ，对文件进行regex分析 可以获取一份文件列表
    