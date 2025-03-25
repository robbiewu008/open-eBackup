cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
include(CMakePrintHelpers)
project(scanner_test)

set(nas_test_obj
    src/test_scanner.cpp
    src/interface/ScanTaskTest.cpp
    src/scanner/nas_metafile_parser/test_BackupDeleteCtrl.cpp
    src/scanner/nas_metafile_parser/test_BackupMtimeCtrl.cpp
    src/scanner/nas_metafile_parser/test_ScannerBackupCtrl.cpp
    src/scanner/nas_metafile_parser/test_ScannerBackupMeta.cpp
    src/scanner/nas_metafile_parser/test_ScannerCheckPoint.cpp
    src/scanner/nas_metafile_parser/test_ScannerDirCache.cpp
    src/scanner/nas_metafile_parser/test_ScannerFileCache.cpp
    src/scanner/nas_metafile_parser/test_ScannerHardLinkCtrl.cpp
    src/scanner/service/test_ScanFilter.cpp
    src/scanner/service/test_CheckPoint.cpp
    src/interface/ScannerInterfaceTest.cpp
)
message(${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_executable(test_scanner ${nas_test_obj})

set(openSrc_include_dirs
  third_party_software/gmock/googletest-release/googletest/include/
  third_party_software/gmock/googletest-release/googlemock/include/
)

SET(DME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../data_move_engine)
SET(DME_FRAMEWORK_PATH ${DME_PATH}/DME_Framework)
SET(DME_OPENSRC_PATH ${DME_FRAMEWORK_PATH}/third_party_groupware)
SET(DME_FRAMEWORK_PLATFORM_PATH ${DME_FRAMEWORK_PATH}/platform)
SET(LIB_OPENSRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../third_open_src)
MESSAGE(${DME_PATH})

# 处理依赖的Framework模块依赖新增开源的问题
if (EXISTS ${DME_FRAMEWORK_PLATFORM_PATH}/KMCv3_infra_rel/lib)
    SET(DME_OPENSRC_KMCV3_LIB_PATH ${DME_FRAMEWORK_PLATFORM_PATH}/KMCv3_infra_rel/lib)
    SET(DME_OPENSRC_KMCV3_LIB_NAME kmcv3)
endif()
if (EXISTS ${DME_FRAMEWORK_PLATFORM_PATH}/c-ares_rel/lib)
    SET(DME_OPENSRC_CARES_LIB_PATH ${DME_FRAMEWORK_PLATFORM_PATH}/c-ares_rel/lib)
    SET(DME_OPENSRC_CARES_LIB_NAME cares)
    SET(DME_OPENSRC_CARES_INCLUDE_PATH ${DME_FRAMEWORK_PLATFORM_PATH}/c-ares_rel/include)
endif()

set(scanner_src_include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/scanner
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/scanner/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/scanner/common/thread_pool
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/scanner/model
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/scanner/nas_metafile_parser
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/scanner/service
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/scanner/utils

    ${DME_OPENSRC_PATH}/lsmb2_rel/include
    ${DME_OPENSRC_PATH}/lnfs_rel/include
    ${DME_OPENSRC_PATH}/libssh2_rel/include
    ${DME_OPENSRC_PATH}/acl_rel/include
    ${DME_OPENSRC_PATH}/libuuid_rel/include
    ${DME_OPENSRC_PATH}/libaio_rel/include
    ${DME_OPENSRC_PATH}/protobuf_rel/include
    ${DME_OPENSRC_PATH}/libiconv_rel/include
    ${DME_OPENSRC_PATH}/libxml2_rel/include
    ${DME_OPENSRC_PATH}/7z_rel/include
    ${DME_OPENSRC_PATH}/jsoncpp_rel/include
    ${DME_OPENSRC_PATH}/esdk_rel/include
    ${DME_OPENSRC_PATH}/fcgi_rel/include
    ${DME_OPENSRC_PATH}/soci_rel/include
    ${DME_OPENSRC_PATH}/gperftools_rel/include
    ${DME_OPENSRC_PATH}/LibRdKafka_rel/include
    ${DME_OPENSRC_PATH}/lz4_rel/include
    ${DME_OPENSRC_PATH}/nghttp2_rel/include
    ${DME_OPENSRC_PATH}/pcre_rel/include
    ${DME_OPENSRC_PATH}/tinyxml2_rel/include
    ${DME_OPENSRC_PATH}/openssl_rel/include
    ${DME_OPENSRC_PATH}/leveldb_rel/include
    ${DME_OPENSRC_PATH}/curl_rel/include
    ${DME_OPENSRC_PATH}/boost_rel/include
    ${DME_FRAMEWORK_PLATFORM_PATH}/SecureCLib_rel/include
    ${DME_FRAMEWORK_PLATFORM_PATH}/FillpLib_rel/include
    ${DME_FRAMEWORK_PLATFORM_PATH}/KMCV3_rel/include
    ${DME_FRAMEWORK_PLATFORM_PATH}/TracePoint_rel/include
    ${DME_FRAMEWORK_PLATFORM_PATH}/gaussdb_rel/include
    ${DME_FRAMEWORK_PATH}/src/inc
    ${DME_FRAMEWORK_PATH}/var
    ${DME_PATH}/framework/inc
    ${DME_PATH}/transmission/inc
    ${DME_OPENSRC_CARES_INCLUDE_PATH}
)

set(nas_test_src_include_dirs
    src
    llt_stub
)

target_include_directories(test_scanner
    PRIVATE
    ${openSrc_include_dirs}
    ${scanner_src_include_dirs}
    ${nas_test_src_include_dirs}
)

set(openSrc_lib_dirs
  third_party_software/gmock/googletest-release/lib
)

set(scanner_lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../build-cmake/
    ${CMAKE_CURRENT_SOURCE_DIR}/../outpkg/lib
    ${DME_OPENSRC_PATH}/lnfs_rel/lib
    ${DME_OPENSRC_PATH}/lsmb2_rel/lib
    ${DME_FRAMEWORK_PLATFORM_PATH}/SecureCLib_rel/lib
    ${DME_OPENSRC_PATH}/jsoncpp_rel/libs
    ${DME_OPENSRC_PATH}/boost_rel/lib
    ${DME_OPENSRC_PATH}/esdk_rel/lib
    ${DME_OPENSRC_PATH}/leveldb_rel/out-shared
    ${DME_OPENSRC_PATH}/protobuf_rel/lib
    ${DME_OPENSRC_PATH}/sqlite_rel/unixODBC/lib
    ${DME_OPENSRC_PATH}/LibRdKafka_rel/libs
    ${DME_OPENSRC_PATH}/libssh2_rel/lib
    ${DME_OPENSRC_PATH}/soci_rel/lib64
    ${DME_OPENSRC_PATH}/libiconv_rel/lib
    ${DME_OPENSRC_PATH}/curl_rel/lib
    ${DME_OPENSRC_PATH}/openssl_rel/lib
    ${DME_OPENSRC_PATH}/icu_rel/libs/lib
    ${DME_PATH}/framework/lib
    ${DME_PATH}/transmission/lib
    ${DME_OPENSRC_CARES_LIB_PATH}
    ${DME_OPENSRC_KMCV3_LIB_PATH}
)

target_link_directories(test_scanner
    PRIVATE
    ${openSrc_lib_dirs}
    ${scanner_lib}
    ${framework_lib_dirs}
    ${archive_lib_dirs}]]
)

set(KMC_OPT "-DWSEC_COMPILE_CAC_OPENSSL -DWSEC_USE_OPENSSL_110 -DWSEC_LINUX -DKMC_MULTIPROCESS -DWSEC_ERR_CODE_BASE=0x10000000")

target_compile_options(test_scanner
    PRIVATE
    -std=c++1z -fno-access-control -DBOOST_LOG_DYN_LINK
    -g -pipe -fPIC -DLINUX ${KMC_OPT}
    -fprofile-arcs -ftest-coverage
    -Wl,-Bstatic -Wl,-Bdynamic
)

target_link_libraries(test_scanner
    pthread
    Scanner
    FrameCommon
    ebkcommon
    Gaussdb
    kafkaclient
    backup_storage_plugin
    scheduler
    securec
    json
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
    eSDKOBS
    leveldb
    protobuf
    curl
    nfs
    smb2
    odbc
    rdkafka
    rdkafka++
    ssh2
    soci_core
    soci_sqlite3
    iconv
    crypto
    ssl
    ${DME_OPENSRC_CARES_LIB_NAME}
    ${DME_OPENSRC_KMCV3_LIB_NAME}
    gmock gtest gtest_main gmock_main gcov
)
