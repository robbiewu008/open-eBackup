cmake_minimum_required(VERSION 3.16)
include(CMakePrintHelpers)
project(scanner_obj)
add_definitions(-w)
add_definitions(-DBOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)
MESSAGE("Compling the llt of scanner.")

add_definitions(-D_FILE)
add_definitions(-D NAS_SNAPDIFF)
add_definitions(-D_NAS)
add_definitions(-D_OBS)

# 设置全局变量
SET(MODULE_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../Module)

SET(DT_UTILS_PATH ${MODULE_ROOT_PATH}/dt_utils)
SET(GOOGLE_TEST_PATH ${DT_UTILS_PATH}/gmock/googletest-release)
SET(MOCKCPP_PATH ${DT_UTILS_PATH}/mockcpp/build/mockcpp)
SET(MODULE_SRC_PATH ${MODULE_ROOT_PATH}/src)
SET(MODULE_OPENSRC_PATH ${MODULE_ROOT_PATH}/third_open_src)
SET(MODULE_PLATFORM_PATH ${MODULE_ROOT_PATH}/platform)

# 定义一些公共变量
SET(SCANNER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
SET(SCANNER_SRC_PATH ${SCANNER_ROOT_PATH}/localhost_src)

SET(MODULE_ROOT_DIR ${MODULE_ROOT_PATH})
SET(FILE ON)
SET(NAS ON)
SET(OBS ON)
SET(LLT ON)

aux_source_directory(src ScannerTest)
aux_source_directory(src/checkpoint checkpoint_test)
aux_source_directory(src/common common_test)
aux_source_directory(src/filter filter_test)
aux_source_directory(src/interface interface_test)
aux_source_directory(src/model model_test)
aux_source_directory(src/service service_test)
aux_source_directory(src/statistics statistics_test)
aux_source_directory(src/utils utils_test)

add_executable(scanner_test
    ${ScannerTest}
    ${checkpoint_test}
    ${common_test}
    ${filter_test}
    ${interface_test}
    ${model_test}
    ${service_test}
    ${statistics_test}
    ${utils_test}
)

add_subdirectory(../../ scanner_src)

target_include_directories(scanner_test
    PRIVATE
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/src/parser
    # google test folder
    ${GOOGLE_TEST_PATH}/googletest/include
    ${GOOGLE_TEST_PATH}/googlemock/include
    # llt stub folder
    ${DT_UTILS_PATH}/llt_stub
    # mockcpp
    ${MOCKCPP_PATH}/include

    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${MODULE_PLATFORM_PATH}/ALiCloudSDK_rel/include
    ${MODULE_PLATFORM_PATH}/AwsSDK_rel/include
    ${MODULE_OPENSRC_PATH}/libssh2_rel/include
    ${MODULE_OPENSRC_PATH}/acl_rel/include
    ${MODULE_OPENSRC_PATH}/curl_rel/include
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${MODULE_OPENSRC_PATH}/lsmb2_rel/include
    ${MODULE_OPENSRC_PATH}/lnfs_rel/include
    ${MODULE_OPENSRC_PATH}/esdk_rel/include
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/include

    ${MODULE_SRC_PATH}/threadpool
    ${MODULE_SRC_PATH}/libsmb_ctx
    ${MODULE_SRC_PATH}/libnfs_ctx
    ${MODULE_SRC_PATH}/libobs_ctx
    ${MODULE_SRC_PATH}/device_access

    ${SCANNER_SRC_PATH}/model
    ${SCANNER_SRC_PATH}/filter
    ${SCANNER_SRC_PATH}/checkpoint
    ${SCANNER_SRC_PATH}/common
    ${SCANNER_SRC_PATH}/statistics
    ${SCANNER_SRC_PATH}/utils
    ${SCANNER_SRC_PATH}/interface
    ${SCANNER_SRC_PATH}/service
)

target_link_directories(scanner_test
    PRIVATE
    ${GOOGLE_TEST_PATH}/lib
    # mockcpp
    ${MOCKCPP_PATH}/lib

    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/lib
    ${MODULE_PLATFORM_PATH}/ALiCloudSDK_rel/lib
    ${MODULE_PLATFORM_PATH}/AwsSDK_rel/lib
    ${MODULE_PLATFORM_PATH}/KMCv3_infra_rel/lib
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/libs
    ${MODULE_OPENSRC_PATH}/boost_rel/lib
    ${MODULE_OPENSRC_PATH}/curl_rel/lib
    ${MODULE_OPENSRC_PATH}/lsmb2_rel/lib
    ${MODULE_OPENSRC_PATH}/lnfs_rel/lib
    ${MODULE_OPENSRC_PATH}/esdk_rel/lib
    ${MODULE_OPENSRC_PATH}/libssh2_rel/lib
    ${MODULE_OPENSRC_PATH}/openssl_rel/lib
    ${MODULE_OPENSRC_PATH}/icu_rel/libs/lib
    ${MODULE_OPENSRC_PATH}/c-ares_rel/lib
    ${MODULE_OPENSRC_PATH}/libiconv_rel/lib

    ${SCANNER_ROOT_PATH}/build-cmake-file
    ${MODULE_ROOT_PATH}/lib
)

set(KMC_OPT "-DWSEC_COMPILE_CAC_OPENSSL -DWSEC_USE_OPENSSL_110 -DWSEC_LINUX -DKMC_MULTIPROCESS -DWSEC_ERR_CODE_BASE=0x10000000")

target_compile_options(scanner_test
    PRIVATE
    -std=c++1z -fno-access-control -DBOOST_LOG_DYN_LINK
    -g -pipe -fPIC -DLINUX ${KMC_OPT}
    -fprofile-arcs -ftest-coverage
    -Wl,-Bstatic -Wl,-Bdynamic
)

set(THREADS_PREFER_PTHRED_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(scanner_test
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
    nfs
    smb2
    iconv
    icudata
    icuuc
    icui18n
    kmcv3

    # # 源码的obj
    log_util
    common_util
    Scanner
    securec
    config_reader_util
    system
    threadpool
    device_access
    curl_http_util
    parser
    smb_ctx
    nfs_ctx
    obs_ctx

    gmock gtest gtest_main gmock_main gcov
    mockcpp
)