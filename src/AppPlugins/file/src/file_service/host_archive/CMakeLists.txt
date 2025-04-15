file(GLOB_RECURSE host_archive_restore_srcs *.cpp)

add_library(host_archive_restore_obj
    OBJECT
    ${host_archive_restore_srcs}
)

target_include_directories(host_archive_restore_obj
    PRIVATE
    ${MODULE_ROOT_PATH}/src/parser
    ${PLUGIN_ROOT_PATH}
    ${PLUGIN_FRAMEWORK_PATH}/inc/job/
    ${PLUGIN_FRAMEWORK_PATH}/inc/thrift_interface
    ${PLUGIN_FRAMEWORK_PATH}/inc
    ${PLUGIN_FRAMEWORK_PATH}/inc/client
    ${PLUGIN_FRAMEWORK_PATH}/inc/common
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc/certificateservice
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_SRC_PATH}/utils/
    ${PLUGIN_SRC_PATH}/common
    ${PLUGIN_SRC_PATH}/file_service/host_restore
    ${PLUGIN_SRC_PATH}/common/host
    ${PLUGIN_SRC_PATH}/component
    ${PLUGIN_BACKUP_PATH}/src
    ${PLUGIN_BACKUP_PATH}/src/model
    ${PLUGIN_SCANNER_PATH}/include
    ${PLUGIN_SCANNER_PATH}/include/service
    ${PLUGIN_FRAMEWORK_PATH}/dep/agent_sdk/include
    PUBLIC
    ${PLUGIN_ROOT_PATH}
    ${MODULE_ROOT_PATH}
    ${PLUGIN_BACKUP_PATH}/src/engines
    ${PLUGIN_SCANNER_PATH}/include/interface
)

target_link_libraries(host_archive_restore_obj
    PRIVATE
    host_obj
    common_dep
)
