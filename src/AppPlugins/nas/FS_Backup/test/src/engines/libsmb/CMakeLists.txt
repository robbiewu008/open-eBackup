MESSAGE("Compiling config_reader code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE libsmb_test_srcs *.cpp)

add_library(libsmb_test_obj
    OBJECT
    ${libsmb_test_srcs}
)

target_include_directories(libsmb_test_obj
    PUBLIC
    ${MODULE_ROOT_PATH}/src
    ${PLATFORM}/securec/include
    ${DT_UTILS_PATH}/llt_stub
    ${BACKUP_SRC_PATH}

    ${BACKUP_SRC_PATH}/common
    ${BACKUP_SRC_PATH}/model
    ${OPEN_SRC_PATH}/lnfs_rel/include
    ${OPEN_SRC_PATH}/lsmb2_rel/include
    ${MODULE_ROOT_PATH}/src/common
    ${MODULE_ROOT_PATH}/src/libsmb_ctx
    ${MODULE_ROOT_PATH}/src/log
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/src/parser/
    ${BACKUP_SRC_PATH}/engines/libsmb
    ${BACKUP_SRC_PATH}/service
    ${BACKUP_SRC_PATH}/service/copy
    ${BACKUP_SRC_PATH}/service/delete
    ${BACKUP_SRC_PATH}/service/hardlink
    ${BACKUP_SRC_PATH}/service/dir
)

target_link_directories(libsmb_test_obj
    PUBLIC
    ${PLATFORM_PATH}/securec/lib
    ${MODULE_ROOT_PATH}/src/libsmb_ctx
    ${BACKUP_SRC_PATH}/engines/libsmb
    ${BACKUP_ROOT_PATH}
)

target_link_libraries(libsmb_test_obj
    PUBLIC
    securec
    log_util
    config_reader_util
)
