MESSAGE("Compiling database, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
# 去除编译告警
cmake_minimum_required(VERSION 3.16)
project(DatabasePlugin)
include(${CMAKE_CURRENT_SOURCE_DIR}/../build/cmake/global.cmake)
add_library(database_inc
    INTERFACE
    IMPORTED
    GLOBAL)

SET(FRAMEWORK_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../common/framework)
SET(FRAMEWORK_INC_PATH ${FRAMEWORK_BASE_PATH}/inc)
SET(FRAMEWORK_LIB_PATH ${FRAMEWORK_BASE_PATH}/lib)
SET(MODULE_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../common/Module)
SET(MODULE_LIB_PATH ${MODULE_BASE_PATH}/lib)
SET(MODULE_OPENSRC_PATH ${MODULE_BASE_PATH}/third_open_src)
SET(MODULE_SECUREC_INC_PATH ${MODULE_BASE_PATH}/platform/SecureCLib_rel/include)
SET(MODULE_THRIFT_INC_PATH ${MODULE_OPENSRC_PATH}/thrift_rel/include)
SET(AGENT_SDK_PATH ${FRAMEWORK_BASE_PATH}/dep/agent_sdk)

target_include_directories(database_inc
    INTERFACE
    ${MODULE_OPENSRC_PATH}/openssl_rel/include
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/include
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${FRAMEWORK_INC_PATH}
    ${FRAMEWORK_INC_PATH}/thrift_interface
    ${AGENT_SDK_PATH}/include
)
target_link_directories(database_inc
    INTERFACE
    ${MODULE_OPENSRC_PATH}/openssl_rel/lib
    ${MODULE_OPENSRC_PATH}/boost_rel/lib
    ${MODULE_BASE_PATH}/platform/KMCv3_infra_rel/lib
    ${MODULE_BASE_PATH}/platform/SecureCLib_rel/lib
    ${MODULE_OPENSRC_PATH}/libssh2_rel/lib
    ${MODULE_OPENSRC_PATH}/c-ares_rel/lib
    ${MODULE_OPENSRC_PATH}/libevent_rel/lib
    ${MODULE_OPENSRC_PATH}/curl_rel/lib
    ${MODULE_OPENSRC_PATH}/icu_rel/libs/lib
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/libs
    ${MODULE_OPENSRC_PATH}/thrift_rel/lib
    ${MODULE_OPENSRC_PATH}/zlib_rel/lib
    ${MODULE_LIB_PATH}
    ${FRAMEWORK_LIB_PATH}
    ${FRAMEWORK_LIB_PATH}/agent_sdk
    ${FRAMEWORK_LIB_PATH}/3rd
    ${FRAMEWORK_LIB_PATH}/thrift_interface
    ${DME_FRAMEWORK_PLATFORM_PATH}/KMCv3_infra_rel/lib
    ${AGENT_SDK_PATH}/lib
    ${FRAMEWORK_LIB_PATH}
)
if(AIX_COMPILE)
    target_link_libraries(database_inc
        INTERFACE
        event
        thrift_server
        thrift
        thriftnb
        thriftz
        crypto
        config_reader_util
        z
        safe_cmplib
        securec
        json
        ssl
        boost_chrono
        boost_atomic
        boost_thread
        boost_filesystem
        boost_system
        boost_regex
        boost_log
        boost_date_time
        rpc
        basic_job
        thrift_interface
        thrift_client
        # module
        common_util
        log_util
        pluginsdk
        icudata
        icuuc
        icui18n
    )
else()
    target_link_libraries(database_inc
        INTERFACE
        safe_cmplib
        securec
        json
        ssl
        boost_chrono
        boost_atomic
        boost_thread
        boost_filesystem
        boost_system
        boost_regex
        boost_log
        boost_date_time
        rpc
        basic_job
        thrift_interface
        thrift_client
        # module
        common_util
        log_util
        pluginsdk
        icudata
        icuuc
        icui18n
    )
endif()
add_subdirectory(applications)
add_subdirectory(common)
add_subdirectory(executor)
add_subdirectory(job)
add_subdirectory(tools)

add_library(database-1.0.0
    SHARED
)
target_link_libraries(database-1.0.0
    PRIVATE
    executor_obj
    job_obj
    general_obj
    cluster_obj
    database_inc
    common_obj
)
