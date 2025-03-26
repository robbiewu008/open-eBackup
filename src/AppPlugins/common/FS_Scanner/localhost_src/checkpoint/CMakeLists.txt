file(GLOB_RECURSE chkpnt_srcs *.cpp)

MESSAGE(STATUS "Current CHKPNT_SRC_PATH path: ${CHKPNT_SRC_PATH}")

add_library(checkpnt_obj
    OBJECT
    ${chkpnt_srcs}
)

target_include_directories(checkpnt_obj
    PUBLIC
    ${OBS_LIB_INC}
    ${MODEL_SRC_PATH}/
    ${MODULE_ROOT_DIR}/src
    ${CHKPNT_SRC_PATH}/
    ${MODULE_ROOT_DIR}/src/parser
    ${SRC_PATH}/common
    ${SRC_PATH}/statistics
    ${SRC_PATH}/utils
    ${SRC_PATH}/filter
    ${LIBSMB_SRC_PATH}/
    ${LIBNFS_SRC_PATH}/
    ${LIBOBS_SRC_PATH}/
    ${THREADPOOL_SRC_PATH}/
    ${DEVICE_ACCESS_SRC_PATH}/
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${MODULE_OPENSRC_PATH}/lnfs_rel/include
    ${MODULE_OPENSRC_PATH}/esdk_rel/include
)
