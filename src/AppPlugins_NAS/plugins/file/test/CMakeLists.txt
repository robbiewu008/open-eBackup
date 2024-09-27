cmake_minimum_required(VERSION 3.16)
include(CMakePrintHelpers)
project(file_plugin_obj)
MESSAGE("Compling the llt of file plugin.")
option(DTFUZZ "Compile in DTFUZZ mode" OFF)
add_definitions(-w)
add_definitions(-DBOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)
SET(LLT ON)

add_definitions(-DBOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)

# 定义一些公共变量
SET(PLUGIN_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
SET(PLUGIN_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../src)
SET(PLUGIN_FRAMEWORK_PATH ${PLUGIN_ROOT_PATH}/framework)
SET(FRAMEWORK_INC_PATH ${PLUGIN_ROOT_PATH}/framework/inc)
SET(PLUGIN_COMMON_PATH ${PLUGIN_SRC_PATH}/common)
SET(PLUGIN_FRAMEWORK_LIB_PATH ${PLUGIN_FRAMEWORK_PATH}/lib)
SET(MODULE_ROOT_PATH ${PLUGIN_ROOT_PATH}/Module)
SET(SCANNER_ROOT_PATH ${PLUGIN_ROOT_PATH}/FS_Scanner)
SET(BACKUP_ROOT_PATH ${PLUGIN_ROOT_PATH}/FS_Backup)
SET(OPEN_SRC_PATH ${MODULE_ROOT_PATH}/third_open_src)
SET(PLATFORM_PATH ${MODULE_ROOT_PATH}/platform)
SET(OPENSRC_THRIFT_INC_PATH ${OPEN_SRC_PATH}/thrift_rel/include/)
set(AGENT_SDK_PATH ${PLUGIN_FRAMEWORK_PATH}/dep/agent_sdk)
SET(DT_UTILS_PATH ${MODULE_ROOT_PATH}/dt_utils)
SET(GOOGLE_TEST_PATH ${DT_UTILS_PATH}/gmock/googletest-release)
SET(MOCKCPP_PATH ${DT_UTILS_PATH}/mockcpp/build/mockcpp)

if (DEFINED DTFUZZ)
        message(STATUS "DTFUZZ defined: " ${DTFUZZ})
else ()
        message(STATUS "DTFUZZ un-defined: " ${DTFUZZ})
endif()
if(DTFUZZ)
file(GLOB_RECURSE native_dtfuzz_srcs src/dt_fuzz/*.cpp)
SET(SECODE_FUZZ_INCLUDE_PATH ${DT_UTILS_PATH}/secodefuzz_rel/include)
SET(SECODE_FUZZ_LIB_PATH ${DT_UTILS_PATH}/secodefuzz_rel/lib)
SET(SECODE_FUZZ_LIB_NAME Secodefuzz)
set(file_plugin_src
    src/FileTest.cpp
    ${native_dtfuzz_srcs}
)
set(FUZZ_OPT "-fdump-rtl-expand")
add_definitions(-DFUZZ_ENABLED)
add_executable(test_file_plugin
    ${file_plugin_src}
)
else()
aux_source_directory(src/common/application application_test)
aux_source_directory(src/common/host host_test)
aux_source_directory(src/component component_test)
aux_source_directory(src/file_service/file_resource file_resource_test)
aux_source_directory(src/file_service/host_archive host_archive_test)
aux_source_directory(src/file_service/host_backup host_backup_test)
aux_source_directory(src/file_service/host_index host_index_test)
aux_source_directory(src/file_service/host_livemount host_livemount_test)
aux_source_directory(src/file_service/host_restore host_restore_test)
aux_source_directory(src/file_service/snapshot_provider snapshot_provider_test)
aux_source_directory(src/job job_test)
aux_source_directory(src/utils utils_test)
aux_source_directory(src src_test)

add_executable(test_file_plugin
    ${src_test}
    ${application_test}
    ${host_test}
    ${component_test}
    ${file_resource_test}
    ${host_archive_test}
    ${host_backup_test}
    ${host_index_test}
    ${host_livemount_test}
    ${host_restore_test}
    ${snapshot_provider_test}
    ${job_test}
    ${utils_test}
)

add_subdirectory(../src file_src)
endif()

set(FILEPLUGIN_INC_FOLDER
    ${MODULE_ROOT_PATH}/dt_utils/llt_stub

    ${PLUGIN_ROOT_PATH}
    ${PLUGIN_SRC_PATH}/common
    ${PLUGIN_SRC_PATH}/common/application
    ${PLUGIN_SRC_PATH}/common/constant
    ${PLUGIN_SRC_PATH}/common/host
    ${PLUGIN_SRC_PATH}/component
    ${PLUGIN_SRC_PATH}/component/statistics
    ${PLUGIN_SRC_PATH}/file_service
    ${PLUGIN_SRC_PATH}/file_service/file_resource
    ${PLUGIN_SRC_PATH}/file_service/host_archive
    ${PLUGIN_SRC_PATH}/file_service/host_backup
    ${PLUGIN_SRC_PATH}/file_service/host_index
    ${PLUGIN_SRC_PATH}/file_service/host_livemount
    ${PLUGIN_SRC_PATH}/file_service/host_restore
    ${PLUGIN_SRC_PATH}/file_service/snapshot_provider
    ${PLUGIN_SRC_PATH}/job
    ${PLUGIN_SRC_PATH}/utils
    ${PLUGIN_SRC_PATH}

    # framework，开源代码头文件路径
    ${PLATFORM_PATH}/SecureCLib_rel/include

    ${OPEN_SRC_PATH}/boost_rel/include
    ${OPEN_SRC_PATH}/openssl_rel/include
    ${OPEN_SRC_PATH}/libxml2_rel/include
    ${OPEN_SRC_PATH}/tinyxml2_rel/include
    ${OPEN_SRC_PATH}/jsoncpp_rel/include

    ${PLUGIN_FRAMEWORK_PATH}/dep/agent_sdk/include
    ${FRAMEWORK_INC_PATH}
    ${FRAMEWORK_INC_PATH}/job
    ${FRAMEWORK_INC_PATH}/job_mgr
    ${FRAMEWORK_INC_PATH}/client
    ${FRAMEWORK_INC_PATH}/common
    ${FRAMEWORK_INC_PATH}/thrift_interface
    ${FRAMEWORK_INC_PATH}/rpc
    ${FRAMEWORK_INC_PATH}/rpc/certificateservice
    ${OPENSRC_THRIFT_INC_PATH}

    # Module依赖
    ${MODULE_ROOT_PATH}
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/src/parser
    ${MODULE_ROOT_PATH}/src/metafile_parser
    ${MODULE_ROOT_PATH}/src/common
    ${MODULE_ROOT_PATH}/src/threadpool

    # Backup依赖
    ${BACKUP_ROOT_PATH}/src/model
    ${BACKUP_ROOT_PATH}/src/service
    ${BACKUP_ROOT_PATH}/src/common
    ${BACKUP_ROOT_PATH}/src/engines
    ${BACKUP_ROOT_PATH}/src/engines/posix
    ${BACKUP_ROOT_PATH}/src/service/copy
    ${BACKUP_ROOT_PATH}/src/service/delete
    ${BACKUP_ROOT_PATH}/src/service/hardlink
    ${BACKUP_ROOT_PATH}/src/service/dir
    ${BACKUP_ROOT_PATH}/src/bare_metal_recovery

    # Scanner依赖
    ${SCANNER_ROOT_PATH}/localhost_src
    ${SCANNER_ROOT_PATH}/localhost_src/filter
    ${SCANNER_ROOT_PATH}/localhost_src/interface
    ${SCANNER_ROOT_PATH}/localhost_src/model
    ${SCANNER_ROOT_PATH}/localhost_src/checkpoint
    ${SCANNER_ROOT_PATH}/localhost_src/common
    ${SCANNER_ROOT_PATH}/localhost_src/service
    ${SCANNER_ROOT_PATH}/localhost_src/statistics
    ${SCANNER_ROOT_PATH}/localhost_src/utils

    # gtest
    ${GOOGLE_TEST_PATH}/googletest/include
    ${GOOGLE_TEST_PATH}/googlemock/include
    ${SECODE_FUZZ_INCLUDE_PATH}

    # mockcpp
    ${MOCKCPP_PATH}/include
)

target_include_directories(test_file_plugin
    PRIVATE
    ${FILEPLUGIN_INC_FOLDER}
)

set(FILEPLUGIN_LIB_FOLDER
    ${PLATFORM_PATH}/SecureCLib_rel/lib
    ${OPEN_SRC_PATH}/boost_rel/lib
    ${OPEN_SRC_PATH}/curl_rel/lib
    ${OPEN_SRC_PATH}/c-ares_rel/lib
    ${OPEN_SRC_PATH}/libssh2_rel/lib
    ${OPEN_SRC_PATH}/openssl_rel/lib
    ${OPEN_SRC_PATH}/icu_rel/libs/lib
    ${OPEN_SRC_PATH}/jsoncpp_rel/libs
    ${PLATFORM_PATH}/KMCv3_infra_rel/lib
    ${PLUGIN_FRAMEWORK_LIB_PATH}
    ${PLUGIN_FRAMEWORK_LIB_PATH}/thrift_interface
    ${PLUGIN_FRAMEWORK_PATH}/third_open_src/libevent_rel/lib
    ${FRAMEWORK_INC_PATH}/dep/agent_sdk/lib
    ${PLUGIN_ROOT_PATH}/third_party_software/gmock/googletest-release/lib
    ${SECODE_FUZZ_LIB_PATH}
)

target_link_directories(test_file_plugin
    PRIVATE
    ${FILEPLUGIN_LIB_FOLDER}
    ${AGENT_SDK_PATH}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../build-cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/../build-cmake/system
    ${BACKUP_ROOT_PATH}/build-cmake
    ${SCANNER_ROOT_PATH}/build-cmake-file
    ${GOOGLE_TEST_PATH}/lib
    ${MODULE_ROOT_PATH}/lib
    # mockcpp
    ${MOCKCPP_PATH}/lib
)

set(KMC_OPT "-DWSEC_COMPILE_CAC_OPENSSL -DWSEC_USE_OPENSSL_110 -DWSEC_LINUX -DKMC_MULTIPROCESS -DWSEC_ERR_CODE_BASE=0x10000000")

target_compile_options(test_file_plugin
    PRIVATE
    -std=c++1z -fno-access-control -DBOOST_LOG_DYN_LINK
    -g -pipe -fPIC -DLINUX ${KMC_OPT}
    -fprofile-arcs -ftest-coverage ${FUZZ_OPT}
    -Wl,-Bstatic -Wl,-Bdynamic
)

target_link_libraries(test_file_plugin
    PRIVATE
    securec
    json
    ssl
    crypto
    cares
    pluginsdk
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_regex
    boost_log
    boost_date_time
    basic_job
    thrift_interface
    pthread
    icudata
    icui18n
    icuuc
    crypto
    curl
    ssh2
    kmcv3
    file_plugin
    Scanner
    Backup

    system
    parser
    threadpool
    log_util
    metafile_parser

    thrift_client
    rpc
    thrift_server
    utils
    common_util
    config_reader_util
    gmock gtest gtest_main gmock_main gcov
    ${SECODE_FUZZ_LIB_NAME}
    mockcpp
)
