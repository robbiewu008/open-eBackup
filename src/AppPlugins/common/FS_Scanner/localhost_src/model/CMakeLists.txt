
file(GLOB_RECURSE model_srcs *.cpp)

MESSAGE(STATUS "Current MODEL_SRC_PATH path: ${model_srcs}")

add_library(scanner_model_obj
    OBJECT
    ${model_srcs}
)

target_include_directories(scanner_model_obj
    PUBLIC
    ${OBS_LIB_INC}
    ${MODEL_SRC_PATH}/
    ${COMMON_SRC_PATH}/
    ${LIBSMB_SRC_PATH}/
    ${LIBNFS_SRC_PATH}/
    ${LIBOBS_SRC_PATH}/
    ${THREADPOOL_SRC_PATH}/
    ${UTILS_SRC_PATH}/
    ${DEVICE_ACCESS_SRC_PATH}/
    ${MODULE_OPENSRC_PATH}/lnfs_rel/include
    ${MODULE_OPENSRC_PATH}/esdk_rel/include
)

target_link_libraries(scanner_model_obj
    PUBLIC
)