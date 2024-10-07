INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../../build/cmake/global.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../../build/cmake/path.cmake)

set(CMAKE_C_COMPILER g++)
set(CMAKE_CXX_COMPILER g++)

# 递归添加thrift目录下所有的cpp文件
file(GLOB_RECURSE ndmpcopy_srcs *.c)

SET(PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/../../../platform)
SET(OPENSRC_INC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_open_src")
SET(BOOST_PATH "${OPENSRC_INC_PATH}/boost_rel")

add_library(ndmp_client
    SHARED
    libndmp.c
    handler_tbl.c
    log.c
    notify.c
    queue.c
    file_history.c
    ndmp_data.c
)

target_include_directories(ndmp_client
    PRIVATE
    ${PLATFORM}/SecureCLib_rel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc/
    ${OPENSRC_INC_PATH}/tirpc_rel/include/tirpc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${OPENSRC_INC_PATH}/openssl_rel/include
)

target_link_directories(ndmp_client
    PRIVATE
    ${OPENSRC_INC_PATH}/protobuf_rel/lib
    ${OPENSRC_INC_PATH}/boost_rel/lib/
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPENSRC_INC_PATH}/tirpc_rel/libs
    ${OPENSRC_INC_PATH}/openssl_rel/lib
)

target_link_libraries(ndmp_client
    PRIVATE
    pthread
    ssl
    crypto
    dl
    protobuf
    ndmp
    tirpc
    securec
    boost_thread
    boost_system
    log_util
    common_util
)