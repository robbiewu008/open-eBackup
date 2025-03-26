if(NAS)
    file(GLOB_RECURSE interface_srcs *.cpp)
else()
    set(interface_srcs
        LibScanner.cpp
        ScanMgr.cpp
    )
endif()

MESSAGE(STATUS "Current INTERFACE_SRC_PATH path: ${INTERFACE_SRC_PATH}")

add_library(interface_obj
    OBJECT
    ${interface_srcs}
)

target_include_directories(interface_obj
    PUBLIC
    ${OBS_LIB_INC}
    ${INTERFACE_SRC_PATH}/
    ${MODEL_SRC_PATH}/
    ${SERVICE_SRC_PATH}/
    ${COMMON_SRC_PATH}/
    ${UTILS_SRC_PATH}/
    ${PARSER_SRC_PATH}/
    ${STATS_SRC_PATH}/
    ${LIBNFS_SRC_PATH}/
    ${LIBSMB_SRC_PATH}/
    ${LIBOBS_SRC_PATH}/
    ${THREADPOOL_SRC_PATH}/
    ${FILTER_SRC_PATH}/
    ${CHKPNT_SRC_PATH}/
    ${MODULE_ROOT_DIR}/src
    ${MODULE_ROOT_DIR}/src/common/jni
    ${DEVICE_ACCESS_SRC_PATH}/
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/include
    ${MODULE_OPENSRC_PATH}/lnfs_rel/include
    ${MODULE_OPENSRC_PATH}/esdk_rel/include
)

target_link_libraries(interface_obj
    PUBLIC
)