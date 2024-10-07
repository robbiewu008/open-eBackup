cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
include(CMakePrintHelpers)
project(module_test)
set(CMAKE_CXX_STANDARD 17)

# 全局参数配置
#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
SET(MODULE_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(DT_UTILS_PATH ${MODULE_ROOT_PATH}/dt_utils)
SET(GOOGLE_TEST_PATH ${DT_UTILS_PATH}/gmock/googletest-release)
SET(MODULE_SRC_PATH ${MODULE_ROOT_PATH}/src)
SET(OPEN_SRC_PATH ${MODULE_ROOT_PATH}/third_open_src)
SET(PLATFORM_PATH ${MODULE_ROOT_PATH}/platform)
SET(MODULE_CMAKE_PATH ${MODULE_ROOT_PATH}/build-cmake) 
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")

SET(CMAKE_CXX_STANDARD 17)

# 定义全局的编译选项
add_compile_options(-fPIC)

# 定义gtest的使用
add_library(gtest_lib_interface
    INTERFACE
    IMPORTED
    GLOBAL
)

target_include_directories(gtest_lib_interface
    INTERFACE
    # google test folder
    ${GOOGLE_TEST_PATH}/googletest/include
    ${GOOGLE_TEST_PATH}/googlemock/include

    # llt stub folder
    ${DT_UTILS_PATH}
)

target_link_directories(gtest_lib_interface
    INTERFACE
    ${GOOGLE_TEST_PATH}/lib
)

target_link_libraries(gtest_lib_interface
    INTERFACE
    gmock
    gtest
    gtest_main
    gmock_main
    gcov
)

# 定义全局使用的库
link_libraries(
    gtest_lib_interface
)

# 添加子目录
add_subdirectory(${MODULE_SRC_PATH}/threadpool src_threadpool)
add_subdirectory(${MODULE_SRC_PATH}/system src_system)
add_subdirectory(${MODULE_SRC_PATH}/parser src_parser)
add_subdirectory(${MODULE_SRC_PATH}/curl_http src_curl_http)
add_subdirectory(${MODULE_SRC_PATH}/security src_security)
add_subdirectory(${MODULE_SRC_PATH}/data_plane src_data_plane)
add_subdirectory(src/config_reader)
add_subdirectory(src/log)
add_subdirectory(src/threadpool)
add_subdirectory(src/curl_http)
#add_subdirectory(src/parser)
add_subdirectory(src/security)
add_subdirectory(src/data_plane)

# 定义LLT的执行二进制文件
add_executable(test_module
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ModuleTest.cpp
)

target_include_directories(test_module
    PRIVATE
    # src folder
    ${MODULE_SRC_PATH}
)

target_link_directories(test_module
    PRIVATE
    ${OPEN_SRC_PATH}/boost_rel/lib
    ${OPEN_SRC_PATH}/curl_rel/lib
    ${OPEN_SRC_PATH}/libssh2_rel/lib
    ${OPEN_SRC_PATH}/c-ares_rel/lib
    ${OPEN_SRC_PATH}/jsoncpp_rel/libs
    ${PLATFORM_PATH}/KMCv3_infra_rel/lib
    ${OPEN_SRC_PATH}/icu_rel/libs/lib
    ${MODULE_CMAKE_PATH}/log
    ${MODULE_CMAKE_PATH}/common
    ${MODULE_CMAKE_PATH}/config_reader
    ${MODULE_CMAKE_PATH}/parser
    ${MODULE_CMAKE_PATH}/system
    ${MODULE_CMAKE_PATH}/threadpool
    ${MODULE_CMAKE_PATH}/security
)

set(KMC_OPT "-DWSEC_COMPILE_CAC_OPENSSL -DWSEC_USE_OPENSSL_110 -DWSEC_LINUX -DKMC_MULTIPROCESS -DWSEC_ERR_CODE_BASE=0x10000000")
target_compile_options(test_module
    PRIVATE
    -std=c++1z -fno-access-control -DBOOST_LOG_DYN_LINK
    -g -pipe -fPIC -DLINUX ${KMC_OPT}
    -fprofile-arcs -ftest-coverage
    -Wl,-Bstatic -Wl,-Bdynamic
)

set(THREADS_PREFER_PTHRED_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(test_module
    pthread
    Threads::Threads

    # 开源库
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_regex
    boost_log
    boost_date_time
    ssl
    crypto
    curl
    ssh2
    json
    cares
    kmcv3

    # 源码的obj
    log_util
    common_util
    threadpool
    parser
    curl_http_util
    security_cbb
    data_plane_util

    # LLT源码obj
    config_reader_test_obj
    log_test_obj
    threadpool_test_obj
    #parser_test_obj
    curl_http_test
    security_test_obj
    data_plane_protocol_test_obj
    data_plane_session_test_obj
)