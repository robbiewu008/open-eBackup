MESSAGE("Compiling thrift server code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)

# 递归添加thrift目录下所有的cpp文件
file(GLOB_RECURSE thirft_server_srcs *.cpp)

add_library(thrift_server
    SHARED
    ${thirft_server_srcs}
)

target_include_directories(thrift_server
    PRIVATE
    ${INC_PATH}/common
    ${INC_PATH}/server
    ${INC_PATH}/job_mgr
    ${INC_PATH}/utils
    ${OPEN_SRC_PATH}/jsoncpp_rel/include
    ${MODULE_PATH}
)

target_link_directories(thrift_server
    PRIVATE
    ${MODULE_ROOT_PATH}/lib
)

target_link_libraries(thrift_server
    PRIVATE
    pthread
    job_mgr_obj
    param_checker
    utils
)