# 递归添加common目录下所有cpp文件
file(GLOB_RECURSE common_srcs *.cpp)

set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")

add_library(common_obj
    OBJECT
    backup_layout/SqliteOps.cpp
    ${common_srcs}
)

target_include_directories(common_obj
    PRIVATE
    ${BACKUP_SRC_PATH}/common/
    ${BACKUP_SRC_PATH}/model/
    ${BACKUP_SRC_PATH}/engines/posix
    ${LIBNFS_OPENSRC_DIR}/include
    ${LIBSMB_OPENSRC_DIR}/include
    ${MODULE_ROOT_PATH}/src/
    ${MODULE_ROOT_PATH}/src/threadpool
    ${MODULE_ROOT_PATH}/src/common/
    ${MODULE_ROOT_PATH}/src/log/
    ${MODULE_ROOT_PATH}/src/parser/
    ${MODULE_ROOT_PATH}/src/metafile_parser/
    ${MODULE_ROOT_PATH}/src/libobs_ctx
    ${MODULE_ROOT_PATH}/third_open_src/boost_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/esdk_rel/include
    ${LIBSQL_OPENSRC_DIR}/sqlite-autoconf/include
)

target_link_directories(common_obj
    PUBLIC
    ${MODULE_ROOT_PATH}/third_open_src/boost_rel/lib
)

target_link_libraries(common_obj
    PUBLIC
    boost_filesystem
)

target_link_libraries(common_obj
    PRIVATE
    opensrc_dep
    platform_dep
)
