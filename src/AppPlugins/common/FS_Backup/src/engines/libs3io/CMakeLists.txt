# 递归添加 libs3io 所有cpp文件
file(GLOB_RECURSE libs3io_srcs *.cpp)

set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(MODULE_OPEN_SRC_PATH "${MODULE_ROOT_PATH}/third_open_src")
set(BOOST_PATH "${MODULE_ROOT_PATH}/third_open_src/boost_rel")
MESSAGE(libs3io - ${BACKUP_SRC_PATH})

include_directories(../../service/copy)
include_directories(../../service/delete)
include_directories(../../service/hardlink)
include_directories(../../service/dir)

add_library(libs3io_obj
    OBJECT
    ${libs3io_srcs}
)

target_include_directories(libs3io_obj
    PRIVATE
    ${BACKUP_SRC_PATH}/
    ${BACKUP_SRC_PATH}/common
    ${BACKUP_SRC_PATH}/engines/libs3io/interface
    ${BACKUP_SRC_PATH}/model
    ${LIBNFS_OPENSRC_DIR}/include
    ${LIBSMB_OPENSRC_DIR}/include
    ${MODULE_OPEN_SRC_PATH}/jsoncpp_rel/include
    ${MODULE_OPEN_SRC_PATH}/boost_rel/include
    ${MODULE_OPEN_SRC_PATH}/esdk_rel/include
    ${MODULE_ROOT_PATH}/src/threadpool
    ${BOOST_PATH}/include
    ${MODULE_ROOT_PATH}/src/log
    ${MODULE_ROOT_PATH}/src/common
    ${MODULE_ROOT_PATH}/src/io_device
    ${MODULE_ROOT_PATH}/src
)
target_link_libraries(libs3io_obj
    PRIVATE
)