# 递归添加nas_service目录下所有的cpp文件

add_library(host_restore_obj
    OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/HostRestore.cpp


)

target_include_directories(host_restore_obj
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${PLUGIN_FRAMEWORK_PATH}/inc/job/
    ${PLUGIN_FRAMEWORK_PATH}/inc/thrift_interface
    ${PLUGIN_FRAMEWORK_PATH}/inc
    ${PLUGIN_FRAMEWORK_PATH}/inc/client
    ${PLUGIN_FRAMEWORK_PATH}/inc/common
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc/certificateservice


    ${PLUGIN_SRC_PATH}/utils


)

target_link_libraries(host_restore_obj
    PRIVATE
    host_obj
    common_dep
)