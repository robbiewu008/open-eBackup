file(GLOB_RECURSE stats_srcs *.cpp)

MESSAGE(STATUS "Current STATS_SRC_PATH path: ${STATS_SRC_PATH}")

add_library(stats_obj
    OBJECT
    ${stats_srcs}
)

target_include_directories(stats_obj
    PUBLIC
    ${OBS_LIB_INC}
    ${MODEL_SRC_PATH}/
    ${MODULE_ROOT_DIR}/src
    ${SERVICE_SRC_PATH}/ObjectStorage
    ${STATS_SRC_PATH}/
    ${UTILS_SRC_PATH}/
    ${LIBOBS_SRC_PATH}/
    ${MODULE_OPENSRC_PATH}/esdk_rel/include
)