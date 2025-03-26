if(NAS)
    file(GLOB service_srcs *.cpp)
else()
    set(service_srcs
        ControlGenerator.cpp
        DefaultFolderTraversal.cpp
        PosixFolderTraversal.cpp
        ScannerImpl.cpp
    )
endif()

if(OBS)
    add_subdirectory(ObjectStorage)
endif()

MESSAGE(STATUS "Current SERVICE_SRC_PATH path: ${SERVICE_SRC_PATH}")

add_library(service_obj
    OBJECT
    ${service_srcs}
)

target_include_directories(service_obj
    PUBLIC
    ${OBS_LIB_INC}
    ${SERVICE_SRC_PATH}/
    ${SERVICE_SRC_PATH}/ObjectStorage
    ${INTERFACE_SRC_PATH}/
    ${COMMON_SRC_PATH}/
    ${UTILS_SRC_PATH}/
    ${PARSER_SRC_PATH}/
    ${LIBSMB_SRC_PATH}/
    ${LIBNFS_SRC_PATH}/
    ${LIBOBS_SRC_PATH}/
    ${THREADPOOL_SRC_PATH}/
    ${MODEL_SRC_PATH}/
    ${MODULE_ROOT_DIR}/src
    ${MODULE_ROOT_DIR}/src/parser
    ${STATS_SRC_PATH}/
    ${FILTER_SRC_PATH}/
    ${CHKPNT_SRC_PATH}/
    ${DEVICE_ACCESS_SRC_PATH}/
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/include
    ${MODULE_OPENSRC_PATH}/lnfs_rel/include
    ${MODULE_OPENSRC_PATH}/lsmb2_rel/include
    ${MODULE_OPENSRC_PATH}/esdk_rel/include
)
