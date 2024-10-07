add_subdirectory(copy)
add_subdirectory(delete)
add_subdirectory(dir)
add_subdirectory(hardlink)

set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(THIRD_OPEN_SRC_PATH "${MODULE_ROOT_PATH}/third_open_src")
set(MODULE_PATH "${MODULE_ROOT_PATH}")

MESSAGE("${MODULE_ROOT_PATH}/platform/SecureCLib_rel/include")

add_library(service_obj
    OBJECT
    BackupMgr.cpp
)

target_include_directories(service_obj
    PRIVATE
    ${BACKUP_SRC_PATH}/model
    ${BACKUP_SRC_PATH}/common
    ${BACKUP_SRC_PATH}/common/backup_layout
    ${BACKUP_SRC_PATH}/service
    ${BACKUP_SRC_PATH}/service/copy
    ${BACKUP_SRC_PATH}/service/delete
    ${BACKUP_SRC_PATH}/service/dir
    ${BACKUP_SRC_PATH}/service/hardlink
    ${BACKUP_SRC_PATH}/service/antiransomware
    ${BACKUP_SRC_PATH}/engines
    ${BACKUP_SRC_PATH}/engines/posix
    ${BACKUP_SRC_PATH}/engines/libsmb
    ${BACKUP_SRC_PATH}/engines/libnfs
    ${BACKUP_SRC_PATH}/engines/libnfs/requests
    ${BACKUP_SRC_PATH}/engines/libnfs/interface
    ${BACKUP_SRC_PATH}/engines/archive_client
    ${BACKUP_SRC_PATH}/engines/hostcommon
    ${MODULE_PATH}/src/libobs_ctx
    ${MODULE_PATH}/src/threadpool
    ${MODULE_PATH}/src/libnfs_ctx
    ${MODULE_PATH}/src/common
    ${MODULE_PATH}/src/common/jni
    ${MODULE_PATH}/src/parser
    ${MODULE_PATH}/src/metafile_parser/
    ${MODULE_PATH}/src/log
    ${MODULE_PATH}/src/
    ${MODULE_PATH}/platform/SecureCLib_rel/include
    ${THIRD_OPEN_SRC_PATH}/lnfs_rel/include
    ${THIRD_OPEN_SRC_PATH}/lsmb2_rel/include
    ${THIRD_OPEN_SRC_PATH}/boost_rel/include
    ${THIRD_OPEN_SRC_PATH}/7z_rel/include
    ${THIRD_OPEN_SRC_PATH}/7z_rel/include/CPP
    ${THIRD_OPEN_SRC_PATH}/7z_rel/include/CPP/myWindows
    ${THIRD_OPEN_SRC_PATH}/7z_rel/include/CPP/include_windows
    ${THIRD_OPEN_SRC_PATH}/sqlite_rel/sqlite-autoconf/include
    ${THIRD_OPEN_SRC_PATH}/jsoncpp_rel/include
)