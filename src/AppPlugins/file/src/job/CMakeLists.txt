file(GLOB_RECURSE job_obj_srcs *.cpp)

add_library(job_obj
    OBJECT
    ${job_obj_srcs}
)
MESSAGE("framework_path:${FRAMEWORK_INC_PATH}")
target_include_directories(job_obj
    PUBLIC
    ${PLUGIN_FRAMEWORK_PATH}/inc/job
    ${PLUGIN_FRAMEWORK_PATH}/inc/thrift_interface
    ${PLUGIN_FRAMEWORK_PATH}/inc/common
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc/certificateservice
    ${PLUGIN_BACKUP_PATH}/src/volume
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_SRC_PATH}/common
    ${PLUGIN_SRC_PATH}/common/application
    ${PLUGIN_SRC_PATH}/common/constant
    ${PLUGIN_SRC_PATH}/common/host
    ${PLUGIN_SRC_PATH}/common/volume
    ${PLUGIN_SRC_PATH}/file_service
    ${PLUGIN_SRC_PATH}/file_service/file_resource
    ${PLUGIN_SRC_PATH}/file_service/host_backup
    ${PLUGIN_SRC_PATH}/file_service/host_resource
    ${PLUGIN_SRC_PATH}/file_service/host_restore
    ${PLUGIN_SRC_PATH}/file_service/host_livemount
    ${PLUGIN_SRC_PATH}/file_service/host_index
    ${PLUGIN_SRC_PATH}/file_service/host_archive
    ${PLUGIN_SRC_PATH}/file_service/snapshot_provider
    ${PLUGIN_SRC_PATH}/file_service/volume_backup
    ${PLUGIN_SRC_PATH}/file_service/volume_restore
    ${PLUGIN_SRC_PATH}/file_service/volume_livemount
    ${PLUGIN_SRC_PATH}/file_service/volume_index
    PRIVATE
    ${PLUGIN_FRAMEWORK_PATH}/dep/agent_sdk/include
)

target_link_libraries(job_obj
    PRIVATE
    common_dep
    host_obj
    host_restore_obj
    host_backup_obj
    host_archive_restore_obj
)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(job_obj
        PRIVATE
        volume_obj
        volume_backup_obj
        volume_restore_obj
    )
endif()