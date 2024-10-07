# 递归添加 aio 所有cpp文件
file(GLOB_RECURSE libnfs_srcs *.cpp)

set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(MODULE_PATH ${MODULE_ROOT_PATH})
set(BOOST_PATH "${MODULE_PATH}/third_open_src/boost_rel")
set(NFS_PATH "${MODULE_PATH}/third_open_src/lnfs_rel")
set(SMB_PATH "${MODULE_PATH}/third_open_src/lsmb2_rel")
MESSAGE(libnfs - ${BACKUP_SRC_PATH})

include_directories(../../service/copy)
include_directories(../../service/delete)
include_directories(../../service/hardlink)
include_directories(../../service/dir)

add_library(libnfs_obj
    OBJECT
    ${libnfs_srcs}
)

target_include_directories(libnfs_obj
    PRIVATE
    ${BACKUP_SRC_PATH}/
    ${BACKUP_SRC_PATH}/service
    ${BACKUP_SRC_PATH}/common
    ${BACKUP_SRC_PATH}/engines/libnfs/
    ${BACKUP_SRC_PATH}/engines/libnfs/interface
    ${BACKUP_SRC_PATH}/engines/libnfs/requests
    ../../model
    ${NFS_PATH}/include/
    ${SMB_PATH}/include/
    ${MODULE_PATH}/src/threadpool/
    ${MODULE_PATH}/src/libnfs_ctx/
    ${MODULE_PATH}/src/parser/
    ${BOOST_PATH}/include
    ${MODULE_PATH}/src
    ${MODULE_PATH}/src/log/
    ${MODULE_PATH}/platform/SecureCLib_rel/include/
)
target_link_libraries(libnfs_obj
    PRIVATE
)