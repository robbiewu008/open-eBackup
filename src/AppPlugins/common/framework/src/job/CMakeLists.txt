MESSAGE("Compiling common job info, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)

# 递归添加job目录下所有的cpp文件
file(GLOB_RECURSE job_srcs *.cpp)
add_library(basic_job
    SHARED
    ${job_srcs}
)

target_include_directories(basic_job
    PUBLIC
    ${INC_PATH}/job
    ${INC_PATH}/common
    ${INC_PATH}/lib_mgr
    ${INC_PATH}/rpc
    ${INC_PATH}/rpc/certificateservice
    ${INC_PATH}/client
    ${MODULE_PATH}
)

target_link_libraries(basic_job
    PUBLIC
    thrift_interface
)