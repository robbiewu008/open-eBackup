if (DEFINED MODULE_ROOT_DIR)
    SET(MODULE_ROOT_DIR ${MODULE_ROOT_DIR})
else()
    SET(MODULE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Module)
endif()
add_definitions(-DBOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)
SET(MODULE_OPENSRC_PATH ${MODULE_ROOT_DIR}/third_open_src)
SET(MODULE_PLATFORM_PATH ${MODULE_ROOT_DIR}/platform)
MESSAGE(${DME_PATH}, ${MODULE_OPENSRC_PATH}, ${MODULE_PLATFORM_PATH})


set(SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
set(COMMON_SRC_PATH "${SRC_PATH}/common")
set(INTERFACE_SRC_PATH "${SRC_PATH}/interface")
set(MODEL_SRC_PATH "${SRC_PATH}/model")
set(PARSER_SRC_PATH "${MODULE_ROOT_DIR}/src/parser")
set(LIBSMB_SRC_PATH "${MODULE_ROOT_DIR}/src/libsmb_ctx")
set(LIBNFS_SRC_PATH "${MODULE_ROOT_DIR}/src/libnfs_ctx")
set(LIBOBS_SRC_PATH "${MODULE_ROOT_DIR}/src/libobs_ctx")
set(THREADPOOL_SRC_PATH "${MODULE_ROOT_DIR}/src/threadpool")
set(DEVICE_ACCESS_SRC_PATH "${MODULE_ROOT_DIR}/src/device_access")
set(SERVICE_SRC_PATH "${SRC_PATH}/service")
set(UTILS_SRC_PATH "${SRC_PATH}/utils")
set(STATS_SRC_PATH "${SRC_PATH}/statistics")
set(FILTER_SRC_PATH "${SRC_PATH}/filter")
set(CHKPNT_SRC_PATH "${SRC_PATH}/checkpoint")

add_definitions(-DBOOST_FILESYSTEM_DEPRECATED)

link_libraries(
    safe_cmplib
    opensrc_dep
)

if (FILE)
    MESSAGE("--------------defined FILE--------------")
endif()

if (NAS)
    MESSAGE("--------------defined NAS--------------")
    add_definitions(-D_NAS)
    set(DEVICE_ACCESS_LIB
        device_access
        curl_http_util)
    set(KMCV3_LIB_PATH  ${MODULE_PLATFORM_PATH}/KMCv3_infra_rel/lib)
    set(KMCV3_LIB_NAME  kmcv3)
endif()

if (OBS)
    MESSAGE("--------------defined OBS--------------")
    add_definitions(-D_OBS)
endif()

add_library(opensrc_dep
    INTERFACE
    IMPORTED
    GLOBAL
)

target_link_directories(opensrc_dep
    INTERFACE
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/lib
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/libs
    ${MODULE_OPENSRC_PATH}/curl_rel/lib
    ${MODULE_OPENSRC_PATH}/libssh2_rel/lib
    ${MODULE_OPENSRC_PATH}/openssl_rel/lib
    ${MODULE_OPENSRC_PATH}/boost_rel/lib
)

target_link_libraries(opensrc_dep
    INTERFACE
    securec
    pthread
    crypto
    ssl
    json
    boost_filesystem
)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_directories(opensrc_dep
        INTERFACE
        ${MODULE_OPENSRC_PATH}/acl_rel/lib
    )
    target_link_libraries(opensrc_dep
        INTERFACE
        acl
        boost_chrono
        boost_atomic
        boost_thread
        boost_system
        boost_regex
        boost_log
        boost_date_time
    )
elseif (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    target_link_directories(opensrc_dep
        INTERFACE
        ${MODULE_OPENSRC_PATH}/boost_rel/lib
    )
    target_link_libraries(opensrc_dep
        INTERFACE
        boost_chrono
        boost_atomic
        boost_thread
        boost_filesystem
        boost_system
        boost_regex
        boost_log
        boost_date_time
    )
endif()

if (NAS)
    target_include_directories(opensrc_dep
        INTERFACE
        ${MODULE_OPENSRC_PATH}/lsmb2_rel/include
        ${MODULE_OPENSRC_PATH}/lnfs_rel/include
        ${LIBSMB_SRC_PATH}
        ${LIBNFS_SRC_PATH}
    )

    target_link_directories(opensrc_dep
        INTERFACE
        ${MODULE_OPENSRC_PATH}/lsmb2_rel/lib
        ${MODULE_OPENSRC_PATH}/lnfs_rel/lib
        ${KMCV3_LIB_PATH}
    )

    target_link_libraries(opensrc_dep
        INTERFACE
        ${KMCV3_LIB_NAME}
    )
endif()

add_library(module_dep
    INTERFACE
    IMPORTED
    GLOBAL
)

target_link_directories(module_dep
    INTERFACE
    ${MODULE_ROOT_DIR}/lib
    ${MODULE_ROOT_DIR}/Module_rel/lib
)

target_link_libraries(module_dep
    INTERFACE
    parser
    threadpool
    log_util
)

if (NAS)
    target_link_libraries(module_dep
        INTERFACE
        smb_ctx
        nfs_ctx
        ${DEVICE_ACCESS_LIB}
    )
endif()

if (OBS)
    SET(OBS_LIB_INC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${MODULE_PLATFORM_PATH}/ALiCloudSDK_rel/include
    )

    target_link_libraries(module_dep
        INTERFACE
        obs_ctx
    )
endif()

add_subdirectory(common)
add_subdirectory(interface)
add_subdirectory(service)
add_subdirectory(utils)
add_subdirectory(statistics)
add_subdirectory(filter)
add_subdirectory(checkpoint)
