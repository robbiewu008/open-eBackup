file(GLOB_RECURSE libobs_srcs *.cpp)

set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(MODULE_PATH ${MODULE_ROOT_PATH})
set(MODULE_OPEN_SRC_PATH "${MODULE_PATH}/third_open_src")
MESSAGE(libobs - ${BACKUP_SRC_PATH})

include_directories(../../service/copy)
include_directories(../../service/delete)

add_library(libobs_obj
    OBJECT
    ${libobs_srcs}
)

target_include_directories(libobs_obj
    PUBLIC
    ${BACKUP_SRC_PATH}/
    ${BACKUP_SRC_PATH}/service
    ${BACKUP_SRC_PATH}/common
    ${BACKUP_SRC_PATH}/engines/object
    ${BACKUP_SRC_PATH}/model

    ${MODULE_OPEN_SRC_PATH}/boost_rel/include
    ${MODULE_OPEN_SRC_PATH}/esdk_rel/include
    ${MODULE_PATH}/src/libobs_ctx
    ${MODULE_PATH}/src/parser
    ${MODULE_PATH}/src
    ${MODULE_PATH}/src/log
    ${MODULE_PATH}/src/common
    ${MODULE_PATH}/src/metafile_parser
    ${MODULE_PATH}/src/threadpool
    ${MODULE_PATH}/platform/SecureCLib_rel/include
)

target_link_libraries(libobs_obj
    PRIVATE
)
