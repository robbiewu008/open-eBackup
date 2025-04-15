MESSAGE("Compiling database, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
# 去除编译告警
cmake_minimum_required(VERSION 3.14)
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

add_subdirectory(applications)
add_subdirectory(common)
add_subdirectory(executor)
add_subdirectory(job)
add_subdirectory(tools)

add_library(database-1.0.0
    SHARED
)

target_link_directories(database-1.0.0
    PRIVATE
    ${MODULE_BASE_PATH}/platform/SecureCLib_rel/lib
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/libs
    ${FRAMEWORK_LIB_PATH}
    ${MODULE_LIB_PATH}
)

target_link_libraries(database-1.0.0
    PRIVATE
    safe_cmplib
    log_util
    common_util
    basic_job
    config_reader_util
    thrift_interface
    thrift_client
    json
    securec
    executor_obj
    job_obj
    general_obj
    cluster_obj
    common_obj
)
