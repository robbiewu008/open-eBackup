MESSAGE("Compiling config_reader code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE common_test_srcs *.cpp)

add_library(archive_restore_test_obj
    OBJECT
    ${common_test_srcs}
)

target_include_directories(archive_restore_test_obj
    PUBLIC
    ${MODULE_ROOT_PATH}/src
    ${PLATFORM}/securec/include
    ${DT_UTILS_PATH}/llt_stub
    ${BACKUP_SRC_PATH}
    ${BACKUP_SRC_PATH}/engines
    ${BACKUP_SRC_PATH}/common
    ${BACKUP_SRC_PATH}/service
    ${BACKUP_SRC_PATH}/service/copy
    ${BACKUP_SRC_PATH}/model
    ${MODULE_ROOT_PATH}/third_open_src/boost_rel/include
    ${OPEN_SRC_PATH}/lnfs_rel/include
    ${OPEN_SRC_PATH}/lsmb2_rel/include
    ${BACKUP_SRC_PATH}/service/dir
    ${BACKUP_SRC_PATH}/../Module/src/threadpool

)

target_link_directories(archive_restore_test_obj
    PUBLIC
    ${PLATFORM_PATH}/securec/lib
)

target_link_libraries(archive_restore_test_obj
    PUBLIC
    securec
    log_util
)
