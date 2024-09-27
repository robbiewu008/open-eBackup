# 递归添加 aio 所有cpp文件
file(GLOB_RECURSE libsmb_srcs *.cpp)

set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(MODULE_PATH ${MODULE_ROOT_PATH})
set(SMB_PATH "${MODULE_PATH}/third_open_src/lsmb2_rel")
MESSAGE(libsmb - ${BACKUP_SRC_PATH})
 
include_directories(../../service/copy)
include_directories(../../service/delete)
include_directories(../../service/hardlink)
include_directories(../../service/dir)

add_library(libsmb_obj
    OBJECT
    ${libsmb_srcs}
)

target_include_directories(libsmb_obj
    PUBLIC
    ${BACKUP_SRC_PATH}/
    ${BACKUP_SRC_PATH}/service
    ${BACKUP_SRC_PATH}/common
    ${BACKUP_SRC_PATH}/engines/libsmb/
    ${BACKUP_SRC_PATH}/engines/libsmb/interface
    ${BACKUP_SRC_PATH}/model
    ${BACKUP_SRC_PATH}/
    ${SMB_PATH}/include/
    ${MODULE_PATH}/src/libsmb_ctx

    ${MODULE_PATH}/src/parser/
    ${MODULE_PATH}/src
    ${MODULE_PATH}/src/log/
    ${MODULE_ROOT_PATH}/src/common
    ${MODULE_PATH}/src/metafile_parser/
    ${MODULE_PATH}/platform/SecureCLib_rel/include/
    ${LIBSMB_OPENSRC_DIR}/include
    ${LIBNFS_OPENSRC_DIR}/include
)

target_link_libraries(libsmb_obj
    PRIVATE
)