project(biliLiveDownload)
cmake_minimum_required(VERSION 3.26)
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(workflow REQUIRED )
find_package(RapidJSON )
find_package(OpenSSL  REQUIRED)
if(workflow_FOUND)
    message("FOUND WORKFLOW package")
else(workflow_FOUND)
    message(FATAL_ERROR "WORKFLOW NOT FOUND")
endif(workflow_FOUND)


set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -DDebug")



set(
    Bili_Live_Download
    src/fetch_live_status.cpp
    src/m3u8fetch.cpp
)


set(
    Bili_Live_Download_Header
    inc/
)



set(
    EXEC_FETCH_STATUS 
    src/fetch_live_status.cpp
    src/m3u8fetch.cpp
    example/example_fetch_live_status.cpp
)

set(
    HEADER_FETCH_STATUS
    inc/
)

set(WORKFLOW_LIB  
    ${RapidJSON} workflow pthread OpenSSL::SSL OpenSSL::Crypto)

set(
    proj_libs
    ${workflow_INCLUDE_DIR}
    pthread
)

set(CMAKE_CXX_STANDARD 17)

# FETCH_LIVE_STATUS

add_library( Bili_Live_Download_CORE SHARED ${Bili_Live_Download} )
message("entry to lib ")
# set_target_properties(Bili_Live_Download_CORE PROPERTIES  CMAKE_CXX_STANDARD 17)
set_property(TARGET Bili_Live_Download_CORE PROPERTY CXX_STANDARD 20 )
target_include_directories(Bili_Live_Download_CORE PUBLIC ${Bili_Live_Download_Header})
target_link_libraries( Bili_Live_Download_CORE  ${WORKFLOW_LIB})
message("exit from entry")


add_executable( FETCH_LIVE_STATUS example/example_fetch_live_status.cpp )
target_link_directories(FETCH_LIVE_STATUS  PUBLIC build/)
target_include_directories(FETCH_LIVE_STATUS PUBLIC  ${HEADER_FETCH_STATUS} )
target_link_libraries(FETCH_LIVE_STATUS libBili_Live_Download_CORE.so  ${WORKFLOW_LIB})

add_executable(TestSpliteFile  example/splitefile.cpp)
target_link_directories(TestSpliteFile PUBLIC build)
target_include_directories(TestSpliteFile  PUBLIC  ${HEADER_FETCH_STATUS} )
target_link_libraries(TestSpliteFile  PUBLIC  libBili_Live_Download_CORE.so  ${WORKFLOW_LIB} )
# target_link_libraries(TestSpliteFile  PUBLIC    libBili_Live_Download_CORE.a)









add_executable(timetask  example/Timer_loop_event.cpp)
target_link_libraries(timetask  PUBLIC ${WORKFLOW_LIB})
