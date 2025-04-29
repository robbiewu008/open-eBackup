cmake_minimum_required(VERSION 3.16)
project(FS_Backup)
add_definitions(-DBOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)
MESSAGE(STATUS "Current config path: ${CMAKE_CURRENT_SOURCE_DIR}")
# 查看目标库是否存在
if(TARGET safe_cmplib)
    MESSAGE(STATUS "safe_cmplib existed")
else()
    include(${CMAKE_CURRENT_SOURCE_DIR}/build/src/global.cmake)
endif()

if (DEFINED MODULE_ROOT_PATH)
    SET(MODULE_ROOT_PATH ${MODULE_ROOT_PATH})
else()
    SET(MODULE_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Module)
endif()
MESSAGE(${MODULE_ROOT_PATH})

# 打开编译告警
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "AIX")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-parameter -Wextra")
else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-parameter -Wextra -Werror")
endif()

SET(LIBNFS_OPENSRC_DIR ${MODULE_ROOT_PATH}/third_open_src/lnfs_rel)
SET(LIBSMB_OPENSRC_DIR ${MODULE_ROOT_PATH}/third_open_src/lsmb2_rel)
SET(LIBESDK_OPENSRC_DIR ${MODULE_ROOT_PATH}/third_open_src/esdk_rel)
SET(LIBALI_PLATFORM_DIR ${MODULE_ROOT_PATH}/platform/ALiCloudSDK_rel)
SET(ACL_OPENSRC_DIR ${MODULE_ROOT_PATH}/third_open_src/acl_rel)
SET(LOG_UTIL ${MODULE_ROOT_PATH}/build-cmake/log/)
SET(LIBSQL_OPENSRC_DIR ${MODULE_ROOT_PATH}/third_open_src/sqlite_rel)
SET(LIBUUID_OPENSRC_DIR ${MODULE_ROOT_PATH}/third_open_src/libuuid_rel)
SET(LIBSSL_OPENSRC_DIR ${MODULE_ROOT_PATH}/third_open_src/openssl_rel)
SET(BOOST_OPENSRC_DIR ${MODULE_ROOT_PATH}/third_open_src/boost_rel)

MESSAGE("MODULE_ROOT_PATH:" ${MODULE_ROOT_PATH})
add_definitions(-DBOOST_FILESYSTEM_DEPRECATED)
if (NAS)
    MESSAGE("------------define _NAS-----------------")
    add_definitions(-D_NAS)
endif ()
if (VOLUME)
    MESSAGE("------------define _VOLUME-----------------")
endif()
if (OBS)
    MESSAGE("------------define _OBS-----------------")
    add_definitions(-D_OBS)
endif ()
add_compile_options(-fPIC)

# opensrc_dep
add_library(opensrc_dep
    INTERFACE
    IMPORTED
    GLOBAL
)

target_link_directories(opensrc_dep
    INTERFACE
    ${LIBSQL_OPENSRC_DIR}/sqlite-autoconf/lib
    ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/libs
)

target_link_libraries(opensrc_dep
    INTERFACE
    sqlite3
    json
)

# AIX不可以在target_include_directories中使用INTERFACE
if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_include_directories(opensrc_dep
        INTERFACE
        ${ACL_OPENSRC_DIR}/include
        ${LIBSQL_OPENSRC_DIR}/sqlite-autoconf/include
        ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/include
    )

    target_link_directories(opensrc_dep
        INTERFACE
        ${ACL_OPENSRC_DIR}/lib
        ${BOOST_OPENSRC_DIR}/lib
        ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/libs
    )

    target_link_libraries(opensrc_dep
        INTERFACE
        acl
        json
        boost_chrono
        boost_atomic
        boost_thread
        boost_filesystem
        boost_date_time
        boost_system
        boost_log
    )
endif()


if(NAS)
    target_include_directories(opensrc_dep
        INTERFACE
        ${LIBNFS_OPENSRC_DIR}/include
        ${LIBSMB_OPENSRC_DIR}/include
    )
    target_link_directories(opensrc_dep
        INTERFACE
        ${LIBNFS_OPENSRC_DIR}/lib
        ${LIBSMB_OPENSRC_DIR}/lib
    )

    target_link_libraries(opensrc_dep
        INTERFACE
        nfs
        smb2
    )
endif()
if (VOLUME AND "${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
        target_include_directories(opensrc_dep
            INTERFACE
            ${LIBUUID_OPENSRC_DIR}/include
        )
        target_link_directories(opensrc_dep
            INTERFACE
            ${LIBUUID_OPENSRC_DIR}/lib
            ${LIBSSL_OPENSRC_DIR}/lib
        )
        target_link_libraries(opensrc_dep
            INTERFACE
            uuid
            ssl
            crypto
        )
endif()
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    target_include_directories(opensrc_dep
        INTERFACE
        ${LIBUUID_OPENSRC_DIR}/include
    )
    target_link_directories(opensrc_dep
        INTERFACE
        ${LIBUUID_OPENSRC_DIR}/lib
        ${LIBSSL_OPENSRC_DIR}/lib
    )
    target_link_libraries(opensrc_dep
        INTERFACE
        uuid
        ssl
        crypto
    )
endif()

if (OBS)
    target_include_directories(opensrc_dep
        INTERFACE
        ${MODULE_ROOT_PATH}/src/libobs_ctx
        ${LIBESDK_OPENSRC_DIR}/include
        ${LIBALI_PLATFORM_DIR}/include
    )
    target_link_directories(opensrc_dep
        INTERFACE
        ${LIBESDK_OPENSRC_DIR}/lib
        ${LIBALI_PLATFORM_DIR}/lib
    )
    target_link_libraries(opensrc_dep
        INTERFACE
    )
endif()

# platform_dep
add_library(platform_dep
    INTERFACE
    IMPORTED
    GLOBAL
)

target_include_directories(platform_dep
    INTERFACE
    ${MODULE_ROOT_PATH}/platform/SecureCLib_rel/include
)

target_link_directories(platform_dep
    INTERFACE
    ${MODULE_ROOT_PATH}/platform/SecureCLib_rel/lib
)

target_link_libraries(platform_dep
    INTERFACE
    securec
)

link_libraries(
    safe_cmplib
    opensrc_dep
    platform_dep
)

add_subdirectory(src)

add_library(Backup
    SHARED
)
target_link_directories(Backup
    PUBLIC
    ${MODULE_ROOT_PATH}/lib
    ${MODULE_ROOT_PATH}/build-cmake/metafile_parser
    ${MODULE_ROOT_PATH}/build-cmake/libnfs_ctx
    ${MODULE_ROOT_PATH}/build-cmake/libsmb_ctx
    ${MODULE_ROOT_PATH}/build-cmake/libobs_ctx
    ${MODULE_ROOT_PATH}/build-cmake/log
    ${MODULE_ROOT_PATH}/build-cmake/system
    ${MODULE_ROOT_PATH}/build-cmake/threadpool
    ${MODULE_ROOT_PATH}/build-cmake/parser
    ${MODULE_ROOT_PATH}/build-cmake/common
)

target_link_libraries(Backup
    PUBLIC
    common_obj
    posix_obj
    hostcommon_obj
    model_obj
    archive_client_obj
    service_copy_obj
    service_delete_obj
    service_dir_obj
    service_hardlink_obj
    service_obj
    metafile_parser
    log_util
    threadpool
    parser
    system
    opensrc_dep
    common_util
)

if(NAS)
    target_link_libraries(Backup
        PUBLIC
        libaio_obj
        libnfs_obj
        libsmb_obj
        nfs_anti_ransomware_obj
        service_antiransomware_obj
        smb_ctx
        nfs_ctx
    )
endif ()
if (VOLUME AND "${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(
        Backup
        PUBLIC
        bare_metal_recovery_obj
        VOLUMEPROTECT_OBJ
    )
endif()
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    target_link_libraries(
        Backup
        PUBLIC
        bare_metal_recovery_obj
        VOLUMEPROTECT_OBJ
    )
endif()
if (OBS)
    target_link_libraries(Backup
        PUBLIC
        libobs_obj
        obs_ctx
    )
endif()