
if(NAS)
    file(GLOB_RECURSE common_srcs *.cpp)
else()
    set(common_srcs
        CommonService.cpp
        ConsumerThread.cpp
        DiffControlService.cpp
        DiffControlThread.cpp
        DirMtimeService.cpp
        MergeDcache.cpp
        MergeDcacheService.cpp
        MergeDcacheThread.cpp
        MetaConsumer.cpp
        MetaProducer.cpp
        PosixMetaProducer.cpp
        PosixProducerThread.cpp
        ScanQueue.cpp
        ProducerThread.cpp
        CtrlFilterManager.cpp
    )
endif()

MESSAGE(STATUS "Current COMMON_SRC_PATH path: ${COMMON_SRC_PATH}")

add_library(common_obj
    OBJECT
    ${common_srcs}
)

target_include_directories(common_obj
    PUBLIC
    ${OBS_LIB_INC}
    ${COMMON_SRC_PATH}/
    ${STATS_SRC_PATH}/
    ${MODEL_SRC_PATH}/
    ${UTILS_SRC_PATH}/
    ${PARSER_SRC_PATH}/
    ${LIBSMB_SRC_PATH}/
    ${LIBNFS_SRC_PATH}/
    ${LIBOBS_SRC_PATH}/
    ${THREADPOOL_SRC_PATH}/
    ${MODULE_ROOT_DIR}/src
    ${FILTER_SRC_PATH}/
    ${CHKPNT_SRC_PATH}/
    ${DEVICE_ACCESS_SRC_PATH}/
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/include
    ${MODULE_OPENSRC_PATH}/acl_rel/include
    ${MODULE_OPENSRC_PATH}/esdk_rel/include
)

if (NAS)
    target_include_directories(common_obj
        PUBLIC
        ${MODULE_OPENSRC_PATH}/lnfs_rel/include
        ${MODULE_OPENSRC_PATH}/lsmb2_rel/include
    )
endif()
