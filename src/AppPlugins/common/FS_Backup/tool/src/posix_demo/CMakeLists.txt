project(backup_tool)
cmake_minimum_required(VERSION 3.16)
file(GLOB_RECURSE tool_srcs *.cpp)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)
set(BACKUP_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../)
set(MODULE_SO_PATH ${BACKUP_ROOT_PATH}/Module/build-cmake)
set(LOG_SO_PATH ${MODULE_SO_PATH}/log)
set(SECURE_LIB_PATH ${MODULE_SO_PATH}/../platform/SecureCLib_rel)
set(BACKUP_SRC_PATH ${BACKUP_ROOT_PATH}/src)
set(BACKUP_SO_PATH ${BACKUP_ROOT_PATH}/build-cmake)
set(SCANNER_SO_PATH ${BACKUP_ROOT_PATH}/FS_Scanner/build-cmake)
SET(LIBNFS_OPENSRC_DIR ${MODULE_SO_PATH}/../third_open_src/lnfs_rel)
SET(LIBSMB_OPENSRC_DIR ${MODULE_SO_PATH}/../third_open_src/lsmb2_rel)
SET(LIB7Z_OPENSRC_DIR ${MODULE_SO_PATH}/../third_open_src/7z_rel)

set(BOOST_PATH "${MODULE_SO_PATH}/../third_open_src/boost_rel")
MESSAGE("BACKUP_SO_PATH = ${BACKUP_SO_PATH}")
MESSAGE("MODULE_SO_PATH = ${MODULE_SO_PATH}")
MESSAGE("boost -> ${MODULE_SO_PATH}/../../third_open_src/boost_rel/include")

add_executable(backup_tool
    ${tool_srcs}
)

target_include_directories(backup_tool
    PRIVATE
    ${BACKUP_SRC_PATH}/common
    ${BACKUP_SRC_PATH}/model
    ${BACKUP_SRC_PATH}/service
    ${MODULE_SO_PATH}/../src/log
    ${MODULE_SO_PATH}/../src
    ${MODULE_SO_PATH}/../src/common
    ${MODULE_SO_PATH}/../src/threadpool
    ${MODULE_SO_PATH}/../third_open_src/boost_rel/include
)

target_link_directories(backup_tool
    PRIVATE
    ${BACKUP_SO_PATH}
    ${LOG_SO_PATH}
    ${LOG_SO_PATH}/../common
    ${LOG_SO_PATH}/../system
    ${LOG_SO_PATH}/../config_reader
    ${SECURE_LIB_PATH}/
    ${MODULE_SO_PATH}/../lib
    ${MODULE_SO_PATH}/../third_open_src/jsoncpp_rel/libs
    ${MODULE_SO_PATH}/../third_open_src/icu_rel/libs/lib
    ${LIBNFS_OPENSRC_DIR}/lib
    ${LIBSMB_OPENSRC_DIR}/lib
    ${BOOST_PATH}/lib
    ${LIB7Z_OPENSRC_DIR}/lib
    ${MODULE_SO_PATH}/libnfs_ctx
    ${MODULE_SO_PATH}/libsmb_ctx
    ${MODULE_SO_PATH}/parser
    ${MODULE_SO_PATH}/metafile_parser
    ${MODULE_SO_PATH}/threadpool
)

target_link_libraries(backup_tool
    PRIVATE
    nfs
    smb2
    Backup
    log_util
    system
    config_reader_util
    securec
    common_util
    json
    pthread
    icudata
    icuuc
    icui18n
    metafile_parser
    threadpool
    parser
    nfs_ctx
    smb_ctx
)