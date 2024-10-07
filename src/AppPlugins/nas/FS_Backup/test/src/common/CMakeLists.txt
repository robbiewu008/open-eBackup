MESSAGE("Compiling config_reader code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE common_test_srcs *.cpp)

add_library(common_test_obj
    OBJECT
    ${common_test_srcs}
)

target_include_directories(common_test_obj
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
    ${MODULE_ROOT_PATH}/src/log
    ${MODULE_ROOT_PATH}/src
)

target_link_directories(common_test_obj
    PUBLIC
    ${PLATFORM_PATH}/securec/lib
)

target_link_libraries(common_test_obj
    PUBLIC
    securec
    log_util
    config_reader_util
)
