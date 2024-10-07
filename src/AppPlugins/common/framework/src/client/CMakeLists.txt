MESSAGE("Compiling thirft client, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)

# 递归添加client目录下所有的cpp文件
file(GLOB_RECURSE thirft_client_srcs *.cpp)
add_library(thrift_client
    SHARED
    ${thirft_client_srcs}
)

target_include_directories(thrift_client
    PRIVATE
    ${INC_PATH}/client
    ${INC_PATH}/rpc
    ${INC_PATH}/common
    ${INC_PATH}/rpc/thriftservice
    ${INC_PATH}/rpc/certificateservice
    ${MODULE_PATH}
)

target_link_libraries(thrift_client
    PRIVATE
    pthread
    thrift_interface
)

if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "AIX")
target_link_libraries(thrift_client
    PRIVATE
    rpc
)
endif()
