project(scanner_demo_exe)
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

include(../../build/cmake/global.cmake)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

SET(SCANNER_INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../src/interface)
SET(SCANNER_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../src/config)
SET(SCANNER_SCANNER_COMMON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../src/scanner/common)
SET(SCANNER_SCANNER_SERVICE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../src/scanner/service)
SET(SCANNER_SCANNER_MODEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../src/scanner/model)
SET(SCANNER_SCANNER_UTILS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../src/scanner/utils)
SET(SCANNER_SCANNER_PARSER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../src/scanner/nas_metafile_parser)
SET(SCANNER_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
SET(SCANNER_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../build-cmake)

SET(LIB_OPENSRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../Module/third_open_src)
SET(DME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../data_move_engine)
SET(DME_OPENSRC_PATH ${DME_PATH}/DME_Framework/third_party_groupware)
SET(DME_FRAMEWORK_PLATFORM_PATH ${DME_PATH}/DME_Framework/platform)
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../Module)
MESSAGE(${DME_PATH})
MESSAGE(${DME_OPENSRC_PATH})
MESSAGE(${DME_FRAMEWORK_PLATFORM_PATH})
MESSAGE(${LIB_OPENSRC_PATH})

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

add_executable(scanner_demo_exe
    scanner_demo.cpp
)

target_include_directories(scanner_demo_exe
    PRIVATE
    ${SCANNER_INC_PATH}
    ${SCANNER_CONFIG_PATH}
    ${SCANNER_SCANNER_COMMON_PATH}
    ${SCANNER_SCANNER_SERVICE_PATH}
    ${SCANNER_SCANNER_MODEL_PATH}
    ${SCANNER_SCANNER_UTILS_PATH}
    ${SCANNER_SCANNER_PARSER_PATH}
    ${SCANNER_SRC_PATH}
    ${SCANNER_SRC_PATH}/parser
    ${LIB_OPENSRC_PATH}/lnfs_rel/include
    ${LIB_OPENSRC_PATH}/lsmb2_rel/include
    ${DME_PATH}/DME_Framework/src/inc
    ${DME_PATH}/DME_Framework/src/inc/common
    ${DME_PATH}/DME_Framework/var
    ${DME_PATH}/Data_Transmission_Frame/inc

    ${DME_FRAMEWORK_PLATFORM_PATH}/SecureCLib_rel/include
    ${DME_FRAMEWORK_PLATFORM_PATH}/FillpLib_rel/include
    ${DME_FRAMEWORK_PLATFORM_PATH}/KMCV3_rel/include
    ${DME_FRAMEWORK_PLATFORM_PATH}/TracePoint_rel/include
    ${DME_FRAMEWORK_PLATFORM_PATH}/gaussdb_rel/include
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
)

target_link_directories(scanner_demo_exe
    PRIVATE
    ${SCANNER_LIB_PATH}
    ${DME_FRAMEWORK_PLATFORM_PATH}/SecureCLib_rel/lib
    ${DME_OPENSRC_PATH}/jsoncpp_rel/libs
    ${DME_OPENSRC_PATH}/boost_rel/lib
    ${DME_OPENSRC_PATH}/esdk_rel/lib
    ${DME_OPENSRC_PATH}/curl_rel/lib
    ${LIB_OPENSRC_PATH}/lsmb2_rel/lib
    ${LIB_OPENSRC_PATH}/lnfs_rel/lib
    ${DME_OPENSRC_PATH}/leveldb_rel/out-shared
    ${DME_OPENSRC_PATH}/protobuf_rel/lib
    ${DME_OPENSRC_PATH}/sqlite_rel/unixODBC/lib
    ${DME_OPENSRC_PATH}/LibRdKafka_rel/libs
    ${DME_OPENSRC_PATH}/libssh2_rel/lib
    ${DME_OPENSRC_PATH}/soci_rel/lib64
    ${DME_OPENSRC_PATH}/libiconv_rel/lib
    ${DME_OPENSRC_PATH}/openssl_rel/lib
    ${MODULE_PATH}/third_open_src/icu_rel/libs/lib
    ${DME_PATH}/framework/lib
    ${DME_PATH}/transmission/lib
    ${DME_OPENSRC_CARES_LIB_PATH}
    ${DME_OPENSRC_KMCV3_LIB_PATH}
)

target_link_libraries(scanner_demo_exe
    PRIVATE
    Scanner
    ebkcommon
    FrameCommon
    Gaussdb
    kafkaclient
    job_pool
    backup_storage_plugin
    scheduler
    securec
    json
    curl
    nfs
    smb2
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_regex
    boost_log
    boost_date_time
    eSDKOBS
    eSDKLogAPI
    log4cpp
    leveldb
    protobuf
    odbc
    rdkafka
    rdkafka++
    ssh2
    soci_core
    soci_sqlite3
    iconv
    pthread
    crypto
    ssl
    icudata
    icui18n
    icuuc
    ${DME_OPENSRC_CARES_LIB_NAME}
    ${DME_OPENSRC_KMCV3_LIB_NAME}
)
