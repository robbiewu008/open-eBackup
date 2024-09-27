# 递归添加nas_service目录下所有的cpp文件

add_library(host_backup_obj
    OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/HostBackup.cpp
)
include_directories(
)

target_include_directories(host_backup_obj
    PRIVATE
    ${PLUGIN_FRAMEWORK_PATH}/inc/job/
    ${PLUGIN_FRAMEWORK_PATH}/inc/thrift_interface
    ${PLUGIN_FRAMEWORK_PATH}/inc
    ${PLUGIN_FRAMEWORK_PATH}/inc/client
    ${PLUGIN_FRAMEWORK_PATH}/inc/common
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc/certificateservice
    ${PLUGIN_COMMON_PATH}
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_SRC_PATH}/common
    ${PLUGIN_SRC_PATH}/file_service/snapshot_provider
    ${PLUGIN_SRC_PATH}/file_service/
    ${PLUGIN_SRC_PATH}/utils
    ${PLUGIN_SRC_PATH}/common/constant
)

target_link_libraries(host_backup_obj
    PRIVATE
    host_obj
    common_dep
)
