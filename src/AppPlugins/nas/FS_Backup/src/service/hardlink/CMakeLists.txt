file(GLOB_RECURSE service_hardlink_srcs *.cpp)

set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(BOOST_PATH "${MODULE_ROOT_PATH}/third_open_src/boost_rel")

add_library(service_hardlink_obj
    OBJECT
    ${service_hardlink_srcs}
)

target_include_directories(service_hardlink_obj
    PRIVATE
    ${MODULE_ROOT_PATH}/src/parser/
    ${MODULE_ROOT_PATH}/src/metafile_parser/
    ${MODULE_ROOT_PATH}/src/threadpool
    ${MODULE_ROOT_PATH}/src/common
    ${MODULE_ROOT_PATH}/src/define
    ${MODULE_ROOT_PATH}/src/log
    ${MODULE_ROOT_PATH}/src/libnfs_ctx
    ${MODULE_ROOT_PATH}/src/libobs_ctx
    ${MODULE_ROOT_PATH}/src
    ${BACKUP_SRC_PATH}/common/
    ${BACKUP_SRC_PATH}/common/backup_layout
    ${BACKUP_SRC_PATH}/model/
    ${BACKUP_SRC_PATH}/engines/
    ${MODULE_ROOT_PATH}/platform/SecureCLib_rel/include/
    ${MODULE_ROOT_PATH}/third_open_src/boost_rel/include/
    ${MODULE_ROOT_PATH}/third_open_src/lnfs_rel/include/
    ${MODULE_ROOT_PATH}/third_open_src/lsmb2_rel/include/
    ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/include/
    ${MODULE_ROOT_PATH}/third_open_src/esdk_rel/include/
    ${MODULE_ROOT_PATH}/third_open_src/protobuf_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/7z_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/7z_rel/include/CPP
    ${MODULE_ROOT_PATH}/third_open_src/7z_rel/include/CPP/include_windows
    ${LIBSQL_OPENSRC_DIR}/sqlite-autoconf/include

    ${BACKUP_SRC_PATH}/engines/hostcommon
    ${BACKUP_SRC_PATH}/engines/posix
    ${BACKUP_SRC_PATH}/engines/libsmb
    ${BACKUP_SRC_PATH}/engines/object
    ${BACKUP_SRC_PATH}/engines/libnfs
    ${BACKUP_SRC_PATH}/engines/libnfs/requests
    ${BACKUP_SRC_PATH}/engines/libnfs/interface
    ${BACKUP_SRC_PATH}/engines/archive_client
    ${BACKUP_SRC_PATH}/service
    ${BACKUP_SRC_PATH}/service/copy
    ${BACKUP_SRC_PATH}/service/delete
    ${BACKUP_SRC_PATH}/service/dir
    ${BACKUP_SRC_PATH}/service/hardlink
    ${BACKUP_SRC_PATH}/service/antiransomware
)

target_link_libraries(service_hardlink_obj
    PRIVATE
    opensrc_dep
    platform_dep
)

if(NAS)
    target_link_libraries(service_hardlink_obj
        PRIVATE
        libsmb_obj
    )
endif()