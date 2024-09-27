# 递归添加nas_service目录下所有的cpp文件

add_library(host_restore_obj
    OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/HostRestore.cpp


)

target_include_directories(host_restore_obj
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc/job/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc/thrift_interface
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc/client
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc/rpc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc/rpc/certificateservice


    ${PLUGIN_SRC_PATH}/utils


)

target_link_libraries(host_restore_obj
    PRIVATE
    host_obj
    common_dep
)