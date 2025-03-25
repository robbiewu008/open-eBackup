
file(GLOB_RECURSE utils_srcs *.cpp)

MESSAGE(STATUS "Current UTILS_SRC_PATH path: ${UTILS_SRC_PATH}")

add_library(utils_obj
    OBJECT
    ${utils_srcs}
)

target_include_directories(utils_obj
    PUBLIC
    ${OBS_LIB_INC}
    ${UTILS_SRC_PATH}/
    ${MODEL_SRC_PATH}/
    ${PARSER_SRC_PATH}/
    ${MODULE_ROOT_DIR}/src
    ${MODULE_ROOT_DIR}/src/common
    ${STATS_SRC_PATH}/
    ${FILTER_SRC_PATH}/
    ${LIBSMB_SRC_PATH}/
    ${LIBNFS_SRC_PATH}/
    ${LIBOBS_SRC_PATH}/
    ${THREADPOOL_SRC_PATH}/
    ${COMMON_SRC_PATH}/
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${MODULE_OPENSRC_PATH}/lnfs_rel/include
    ${MODULE_OPENSRC_PATH}/acl_rel/include
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/include
    ${MODULE_OPENSRC_PATH}/esdk_rel/include
)
