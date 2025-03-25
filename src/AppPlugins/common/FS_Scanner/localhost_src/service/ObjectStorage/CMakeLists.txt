
file(GLOB_RECURSE obs_src *.cpp)

MESSAGE(STATUS "Current MODEL_SRC_PATH path: ${model_srcs}")

add_library(obs_src_obj
    OBJECT
    ${obs_src}
)

target_include_directories(obs_src_obj
    PUBLIC
    ${OBS_LIB_INC}
    ${MODEL_SRC_PATH}/
    ${COMMON_SRC_PATH}/
    ${SERVICE_SRC_PATH}/
    ${LIBOBS_SRC_PATH}/
    ${PARSER_SRC_PATH}/
    ${STATS_SRC_PATH}/
    ${THREADPOOL_SRC_PATH}/
    ${UTILS_SRC_PATH}/
    ${FILTER_SRC_PATH}/
    ${CHKPNT_SRC_PATH}/
    ${DEVICE_ACCESS_SRC_PATH}/
    ${MODULE_ROOT_DIR}/src
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/include
    ${MODULE_OPENSRC_PATH}/esdk_rel/include
    ${MODULE_OPENSRC_PATH}/boost_rel/include
)

target_link_libraries(obs_src_obj
    PUBLIC
)