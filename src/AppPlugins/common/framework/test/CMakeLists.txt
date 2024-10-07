cmake_minimum_required(VERSION 3.16)
include(CMakePrintHelpers)
project(file_plugin_obj)

MESSAGE("Compling the llt of file plugin.")
add_definitions(-w)
SET(LLT ON)

# 设置全局变量
SET(WORKSPACE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../)
SET(MODULE_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../Module)
SET(PLATFORM_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../Module/platform)
SET(OPEN_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../Module/third_open_src)
SET(PLATFORM_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../Module/platform)
SET(MOCKCPP_PATH ${MODULE_ROOT_PATH}/dt_utils/mockcpp/build/mockcpp)

SET(AGENT_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../dep/agent_sdk)

# # 定义一些公共变量
SET(PLUGIN_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
SET(PLUGIN_FRAMEWORK_PATH ${PLUGIN_ROOT_PATH}/framework)
SET(FRAMEWORK_INC_PATH ${PLUGIN_ROOT_PATH}/framework/inc)
set(AGENT_SDK_PATH ${PLUGIN_FRAMEWORK_PATH}/dep/agent_sdk)

aux_source_directory(src SRC_TEST)
aux_source_directory(src/client CLIENT_TEST)
aux_source_directory(src/job JOB_TEST)
aux_source_directory(src/job_mgr JOB_MGR_TEST)
aux_source_directory(src/server SERVER_TEST)
aux_source_directory(src/startup STARTUP_TEST)
aux_source_directory(src/utils UTILS_TEST)

add_executable(framework_test
    ${SRC_TEST}
    ${CLIENT_TEST}
    ${JOB_TEST}
    ${JOB_MGR_TEST}
    ${SERVER_TEST}
    ${STARTUP_TEST}
    ${UTILS_TEST}
)

add_subdirectory(../src framework_src)

target_include_directories(framework_test
    PRIVATE
    ${WORKSPACE_PATH}
    ${MODULE_ROOT_PATH}/dt_utils/llt_stub
    ${MODULE_ROOT_PATH}/dt_utils/gmock/googletest-release/googletest/include
    ${MODULE_ROOT_PATH}/dt_utils/gmock/googletest-release/googlemock/include
    # mockcpp
    ${MOCKCPP_PATH}/include

    ${OPEN_SRC_PATH}/thrift_rel/include
    ${OPEN_SRC_PATH}/boost_rel/include

    ${MODULE_ROOT_PATH}/src

    ${FRAMEWORK_INC_PATH}
    ${FRAMEWORK_INC_PATH}/job
    ${FRAMEWORK_INC_PATH}/job_mgr
    ${FRAMEWORK_INC_PATH}/common
    ${FRAMEWORK_INC_PATH}/thrift_interface
    ${FRAMEWORK_INC_PATH}/rpc
    ${FRAMEWORK_INC_PATH}/rpc/certificateservice
    ${FRAMEWORK_INC_PATH}/server
    ${FRAMEWORK_INC_PATH}/utils
    ${FRAMEWORK_INC_PATH}/client
    
)

target_link_directories(framework_test
    PRIVATE
    ${FILEPLUGIN_LIB_FOLDER}
    ${AGENT_SDK_PATH}/lib
    ${PLUGIN_FRAMEWORK_PATH}/lib
    ${OPEN_SRC_PATH}/boost_rel/lib
    ${OPEN_SRC_PATH}/jsoncpp_rel/libs
    ${OPEN_SRC_PATH}/thrift_rel/lib
    ${OPEN_SRC_PATH}/icu_rel/libs/lib
    ${PLATFORM_PATH}/SecureCLib_rel/lib
    ${MODULE_ROOT_PATH}/dt_utils/gmock/googletest-release/lib
    # mockcpp
    ${MOCKCPP_PATH}/lib
)

set(KMC_OPT "-DWSEC_COMPILE_CAC_OPENSSL -DWSEC_USE_OPENSSL_110 -DWSEC_LINUX -DKMC_MULTIPROCESS -DWSEC_ERR_CODE_BASE=0x10000000")

target_compile_options(framework_test
    PRIVATE
    -std=c++1z -fno-access-control -DBOOST_LOG_DYN_LINK
    -g -pipe -fPIC -DLINUX ${KMC_OPT}
    -fprofile-arcs -ftest-coverage
    -Wl,-Bstatic -Wl,-Bdynamic
)

target_link_libraries(framework_test
    PRIVATE
    pthread
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
    icudata
    icuuc
    icui18n

    log_util
    config_reader_util
    common_util
    
    pluginsdk

    thrift_client
    rpc
    thrift_server
    utils
    basic_job
    thrift_interface

    gmock gtest gtest_main gmock_main gcov
    mockcpp
)