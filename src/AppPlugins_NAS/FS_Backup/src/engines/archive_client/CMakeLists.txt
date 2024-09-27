file(GLOB_RECURSE archive_srcs *.cpp)

set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(MODULE_OPEN_SRC_PATH "${MODULE_ROOT_PATH}/third_open_src")
set(BOOST_PATH "${MODULE_ROOT_PATH}/third_open_src/boost_rel")

include_directories(../../service/copy)
include_directories(../../service/delete)
include_directories(../../service/hardlink)
include_directories(../../service/dir)
include_directories(../win32)

add_library(archive_client_obj
    OBJECT
    ${archive_srcs}
)

target_include_directories(archive_client_obj
    PRIVATE
    ${MODULE_OPEN_SRC_PATH}/boost_rel/include
    ${LIBNFS_OPENSRC_DIR}/include
    ${LIBSMB_OPENSRC_DIR}/include
    ${BACKUP_SRC_PATH}/
    ${BACKUP_SRC_PATH}/copy
    ${BACKUP_SRC_PATH}/common
    ${BACKUP_SRC_PATH}/model
    ${BACKUP_SRC_PATH}/service
    ${MODULE_ROOT_PATH}/src/libobs_ctx
    ${MODULE_ROOT_PATH}/src/threadpool/
    ${MODULE_ROOT_PATH}/src/log/
    ${MODULE_ROOT_PATH}/src
    ${LIBNFS_OPENSRC_DIR}/include
    ${LIBSMB_OPENSRC_DIR}/include
    ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/include
)
target_link_libraries(archive_client_obj
    PRIVATE
)