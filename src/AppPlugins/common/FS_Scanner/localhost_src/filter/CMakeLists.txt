
file(GLOB_RECURSE filter_src *.cpp)

MESSAGE(STATUS "Current FILTER_SRC_PATH path: ${FILTER_SRC_PATH}")

add_library(filter_obj
    OBJECT
    ${filter_src}
)

target_include_directories(filter_obj
    PUBLIC
    ${OBS_LIB_INC}
    ${FILTER_SRC_PATH}/
    ${MODEL_SRC_PATH}/
    ${PARSER_SRC_PATH}/
    ${MODULE_ROOT_DIR}/src
    ${LIBNFS_SRC_PATH}/
    ${LIBSMB_SRC_PATH}/
    ${LIBOBS_SRC_PATH}/
    ${THREADPOOL_SRC_PATH}/
    ${MODULE_OPENSRC_PATH}/lnfs_rel/include
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${MODULE_OPENSRC_PATH}/esdk_rel/include
)