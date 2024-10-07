MESSAGE("Compiling config_reader code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE libnfs_test_srcs *.cpp)
set(BOOST_PATH "${MODULE_ROOT_PATH}/third_open_src/boost_rel")
MESSAGE(smb_path - ${OPEN_SRC_PATH}/lsmb2_rel/include/)

add_library(libnfs_test_obj
    OBJECT
    ${libnfs_test_srcs}
)

target_include_directories(libnfs_test_obj
    PUBLIC
    ${MODULE_ROOT_PATH}/src
    ${PLATFORM}/securec/include
    ${DT_UTILS_PATH}/llt_stub
    ${BACKUP_SRC_PATH}

    ${BACKUP_SRC_PATH}/common
    ${BACKUP_SRC_PATH}/model
    ${OPEN_SRC_PATH}/lnfs_rel/include
    ${OPEN_SRC_PATH}/lsmb2_rel/include/
    ${MODULE_ROOT_PATH}/src/common
    ${MODULE_ROOT_PATH}/src/libnfs_ctx
    ${MODULE_ROOT_PATH}/src/log
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/src/parser/
    ${MODULE_ROOT_PATH}/src/threadpool/
    ${BACKUP_SRC_PATH}/engines/libnfs
    ${BACKUP_SRC_PATH}/engines/libnfs/requests
    ${BACKUP_SRC_PATH}/engines/libnfs/interface
    ${BACKUP_SRC_PATH}/engines/libsmb
    ${BACKUP_SRC_PATH}/service/
    ${BACKUP_SRC_PATH}/service/copy
    ${BACKUP_SRC_PATH}/service/delete
    ${BACKUP_SRC_PATH}/service/dir
    ${BACKUP_SRC_PATH}/service/hardlink
    ${BOOST_PATH}/include
)

target_link_directories(libnfs_test_obj
    PUBLIC
    ${PLATFORM_PATH}/securec/lib
    ${MODULE_ROOT_PATH}/src/libnfs_ctx
    ${BACKUP_SRC_PATH}/engines/libnfs
    ${BACKUP_SRC_PATH}/engines/libnfs/requests
    ${BACKUP_ROOT_PATH}
)

target_link_libraries(libnfs_test_obj
    PUBLIC
    securec
    log_util
    config_reader_util
)
