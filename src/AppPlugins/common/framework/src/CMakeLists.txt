cmake_minimum_required(VERSION 3.14)
project(Plugin_Framework)
# 查看目标库是否存在
if(TARGET safe_cmplib)
    MESSAGE(STATUS "safe_cmplib existed")
else()
    include(${CMAKE_CURRENT_SOURCE_DIR}/../build/cmake/global.cmake)
endif()

# 设置全局变量
SET(MODULE_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../Module)
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../Module/src)
SET(PLATFORM_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../Module/platform)
SET(OPEN_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../Module/third_open_src)

SET(AGENT_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../dep/agent_sdk)

# 打开编译告警
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-function")
if(DTFUZZ)
MESSAGE(STATUS "******************DTFUZZ is on*************")
SET(SECODE_FUZZ_INCLUDE_PATH ${NAS_OPENSRC_PATH}/secodefuzz_rel/include)
SET(SECODE_FUZZ_LIB_PATH ${NAS_OPENSRC_PATH}/secodefuzz_rel/lib)
SET(SECODE_FUZZ_LIB_NAME Secodefuzz)
SET(GCOV_LIB_NAME gcov)
endif()

#查询需要的库
function(SetStaticLibsPath)
    if (WIN32 OR MSVC)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    elseif (UNIX)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
    endif()

    set(CMAKE_PREFIX_PATH 
        ${OPEN_SRC_PATH}/boost_rel/lib
        ${OPEN_SRC_PATH}/thrift_rel/lib
        ${OPEN_SRC_PATH}/openssl_rel/lib
        ${OPEN_SRC_PATH}/libevent_rel/lib
    )

    if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
        find_library(SLIB_THRIFTZ NAMES thriftz)
        find_library(SLIB_THRIFTNB NAMES thriftnb)
        find_library(SLIB_THRIFT NAMES thrift)
        find_library(SLIB_EVENT NAMES event)
    elseif ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "AIX")
        find_library(SLIB_THRIFTZ NAMES thriftz)
        find_library(SLIB_THRIFTNB NAMES thriftnb)
        find_library(SLIB_THRIFT NAMES thrift)
        find_library(SLIB_EVENT NAMES event)
    elseif ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "SunOS")
        find_library(SLIB_THRIFTZ NAMES thriftz)
        find_library(SLIB_THRIFTNB NAMES thriftnb)
        find_library(SLIB_THRIFT NAMES thrift)
        find_library(SLIB_EVENT NAMES event)
    endif()

    message(STATUS "find library event = " ${SLIB_EVENT})
    message(STATUS "find library thriftz = " ${SLIB_THRIFTZ})
    message(STATUS "find library thriftnb = " ${SLIB_THRIFTNB})
    message(STATUS "find library thrift = " ${SLIB_THRIFT})
    message(STATUS "find library KMC = " ${SLIB_KMC})
endfunction()

SetStaticLibsPath()


# 添加module仓依赖
add_subdirectory(${MODULE_PATH}/log output_log)
add_subdirectory(${MODULE_PATH}/common output_common)
add_subdirectory(${MODULE_PATH}/config_reader output_config_reader)
add_subdirectory(${MODULE_PATH}/param_checker output_param_checker)

link_libraries(
    safe_cmplib
)

if (INTERNAL_PLUGIN)
    set(KMC_LIB_PATH
        ${OPEN_SRC_PATH}/curl_rel/lib
        ${OPEN_SRC_PATH}/libssh2_rel/lib
        ${OPEN_SRC_PATH}/c-ares_rel/lib
    )
    set(KMC_LIB_FILES
        curl
        ssh2
        cares
    )
    add_subdirectory(${MODULE_PATH}/curl_http curl_http)
    set(CURL_HTTP_LIB
        curl_http_util
    )
    add_definitions(-DINTERNAL_PLUGIN_ENABLED)
endif()

link_libraries(
    log_util
    config_reader_util
    common_util
    ${CURL_HTTP_LIB}
)

# 添加框架子目录
add_subdirectory(thrift_interface)
add_subdirectory(client)
add_subdirectory(job)
add_subdirectory(job_mgr)
add_subdirectory(lib_mgr)
add_subdirectory(rpc)
add_subdirectory(rpc_server)
add_subdirectory(server)
add_subdirectory(startup)
add_subdirectory(utils)
