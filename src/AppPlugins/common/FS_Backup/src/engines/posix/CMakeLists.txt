# 递归添加 aio 所有cpp文件
file(GLOB_RECURSE posix_srcs *.cpp)

set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(MODULE_OPEN_SRC_PATH "${MODULE_ROOT_PATH}/third_open_src")
set(BOOST_PATH "${MODULE_ROOT_PATH}/third_open_src/boost_rel")
MESSAGE(posix - ${BACKUP_SRC_PATH})

include_directories(../../service/copy)
include_directories(../../service/delete)
include_directories(../../service/hardlink)
include_directories(../../service/dir)

add_library(posix_obj
    OBJECT
    ${posix_srcs}
)

target_include_directories(posix_obj
    PRIVATE
    ${BACKUP_SRC_PATH}/
    ${BACKUP_SRC_PATH}/common
    ${BACKUP_SRC_PATH}/service
    ${BACKUP_SRC_PATH}/engines/hostcommon
    ${BACKUP_SRC_PATH}/engines/posix
    ${BACKUP_SRC_PATH}/engines/posix/interface
    ${BACKUP_SRC_PATH}/model
    ${LIBNFS_OPENSRC_DIR}/include
    ${LIBSMB_OPENSRC_DIR}/include
    ${MODULE_OPEN_SRC_PATH}/boost_rel/include
    ${MODULE_ROOT_PATH}/src/threadpool
    ${BOOST_PATH}/include
    ${MODULE_ROOT_PATH}/src/libobs_ctx
    ${MODULE_ROOT_PATH}/src/common
    ${MODULE_ROOT_PATH}/src/log
    ${MODULE_ROOT_PATH}/src
    ${ACL_OPENSRC_DIR}/include
)

target_link_libraries(posix_obj
    PRIVATE
    platform_dep
)
