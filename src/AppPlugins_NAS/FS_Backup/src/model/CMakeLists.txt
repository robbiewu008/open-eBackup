file(GLOB_RECURSE model_srcs *.cpp)

set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")
MESSAGE("MODULE_ROOT_PATH=${MODULE_ROOT_PATH}")

add_library(model_obj
    OBJECT
    ${model_srcs}
)

target_include_directories(model_obj
    PRIVATE
    ${BACKUP_SRC_PATH}/
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/src/log
    ${MODULE_ROOT_PATH}/src/libobs_ctx
    ${OPEN_SRC_PATH}/lnfs_rel/include
    ${OPEN_SRC_PATH}/lsmb2_rel/include
    ${OPEN_SRC_PATH}/esdk_rel/include/
    ${LIBSQL_OPENSRC_DIR}/sqlite-autoconf/include
)

target_link_libraries(model_obj
    PRIVATE
)