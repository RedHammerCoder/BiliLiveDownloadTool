project(biliLiveDownload)
cmake_minimum_required(VERSION 3.26)

find_package(workflow REQUIRED )
find_package(RapidJSON )
find_package(OpenSSL  REQUIRED)
if(workflow_FOUND)
    message("FOUND WORKFLOW package")
else(workflow_FOUND)
    message(FATAL_ERROR "WORKFLOW NOT FOUND")
endif(workflow_FOUND)

set(
    BIli_Live_Download
    src/fetch_live_status.cpp
)


set(
    BIli_Live_Download_Header
    inc/
)



set(
    EXEC_FETCH_STATUS 
    src/fetch_live_status.cpp
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

# FETCH_LIVE_STATUS

add_library( BIli_Live_Download_CORE STATIC ${BIli_Live_Download} )
target_include_directories(BIli_Live_Download_CORE PUBLIC ${BIli_Live_Download_Header})
target_link_libraries(BIli_Live_Download_CORE ${WORKFLOW_LIB})


add_executable( FETCH_LIVE_STATUS example/example_fetch_live_status.cpp )
target_link_directories(FETCH_LIVE_STATUS  PUBLIC build)
target_link_libraries(FETCH_LIVE_STATUS libBIli_Live_Download_CORE.a ${WORKFLOW_LIB})

target_include_directories(FETCH_LIVE_STATUS PUBLIC  ${HEADER_FETCH_STATUS})

