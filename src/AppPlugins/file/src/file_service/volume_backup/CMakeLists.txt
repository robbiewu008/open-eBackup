add_library(volume_backup_obj
    OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/VolumeBackup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LinuxVolumeBackup.cpp
)

target_include_directories(volume_backup_obj
    PRIVATE
    ${PLUGIN_FRAMEWORK_PATH}/inc
    ${PLUGIN_FRAMEWORK_PATH}/inc/client
    ${PLUGIN_FRAMEWORK_PATH}/inc/common
    ${PLUGIN_FRAMEWORK_PATH}/inc/job/
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc/certificateservice
    ${PLUGIN_BACKUP_PATH}/src/volume
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_SRC_PATH}/common
    ${PLUGIN_SRC_PATH}/common/constant
    ${PLUGIN_SRC_PATH}/common/volume
    ${PLUGIN_SRC_PATH}/file_service/snapshot_provider
    ${PLUGIN_SRC_PATH}/file_service/volume_service
    ${PLUGIN_SRC_PATH}/utils
    ${PLUGIN_BACKUP_PATH}/src
    ${PLUGIN_BACKUP_PATH}/src/volume
    ${PLUGIN_BACKUP_PATH}/src/volume/native

)

target_link_libraries(volume_backup_obj
    PRIVATE
    volume_obj
    common_dep
)