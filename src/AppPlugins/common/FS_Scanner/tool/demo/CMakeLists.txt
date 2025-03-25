project(ScannerDemo)
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
include(../../build/cmake/global.cmake)

add_definitions(-DBOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)
add_definitions(-D NAS_SNAPDIFF)

if (NAS)
    MESSAGE("--------------defined NAS--------------")
    add_definitions(-D_NAS)
endif()

if (OBS)
    MESSAGE("--------------defined OBS--------------")
    add_definitions(-D_OBS)
endif()

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
SET(SCANNER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
SET(MODULE_ROOT_PATH ${SCANNER_ROOT_PATH}/Module)
MESSAGE("MODULE_ROOT_PATH : ${MODULE_ROOT_PATH}")
SET(SCANNER_INC_PATH ${SCANNER_ROOT_PATH}/localhost_src/interface)
SET(SCANNER_COMMON_PATH ${SCANNER_ROOT_PATH}/localhost_src/common)
SET(SCANNER_STATS_PATH ${SCANNER_ROOT_PATH}/localhost_src/statistics)
SET(SCANNER_CHECKPNT_PATH ${SCANNER_ROOT_PATH}/localhost_src/checkpoint)
SET(SCANNER_SERVICE_PATH ${SCANNER_ROOT_PATH}/localhost_src/service)
SET(SCANNER_MODEL_PATH ${SCANNER_ROOT_PATH}/localhost_src/model)
SET(SCANNER_UTILS_PATH ${SCANNER_ROOT_PATH}/localhost_src/utils)
SET(SCANNER_PARSER_PATH ${SCANNER_ROOT_PATH}/localhost_src/parser)
SET(SCANNER_FILTER_PATH ${SCANNER_ROOT_PATH}/localhost_src/filter)
SET(SCANNER_SRC_PATH ${SCANNER_ROOT_PATH}/localhost_src)

add_executable(ScannerDemo
    ScannerDemo.cpp
    ScannerSerializableConfig.cpp
    ReadMetaDataTool.cpp
)

target_include_directories(ScannerDemo
    PRIVATE
    ${SCANNER_INC_PATH}
    ${SCANNER_COMMON_PATH}
    ${SCANNER_STATS_PATH}
    ${SCANNER_CHECKPNT_PATH}
    ${SCANNER_SERVICE_PATH}
    ${SCANNER_MODEL_PATH}
    ${SCANNER_UTILS_PATH}
    ${SCANNER_FILTER_PATH}
    ${SCANNER_SRC_PATH}
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/src/log
    ${MODULE_ROOT_PATH}/src/parser
    ${MODULE_ROOT_PATH}/src/libnfs_ctx
    ${MODULE_ROOT_PATH}/src/libsmb_ctx
    ${MODULE_ROOT_PATH}/src/libobs_ctx
    ${MODULE_ROOT_PATH}/src/common
    ${MODULE_ROOT_PATH}/src/threadpool
    ${MODULE_ROOT_PATH}/platform/SecureCLib_rel/include
    ${MODULE_ROOT_PATH}/platform/ALiCloudSDK_rel/include
    ${MODULE_ROOT_PATH}/platform/KMCv3_infra_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/boost_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/lnfs_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/lsmb2_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/esdk_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/include/json
    ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/libssh2_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/libiconv_rel/include
    ${SCANNER_FILTER_PATH}
)

target_link_directories(ScannerDemo
    PRIVATE
    ${EXECUTABLE_OUTPUT_PATH}
    ${MODULE_ROOT_PATH}/lib
    ${MODULE_ROOT_PATH}/platform/SecureCLib_rel/lib
    ${MODULE_ROOT_PATH}/platform/ALiCloudSDK_rel/lib
    ${MODULE_ROOT_PATH}/platform/KMCv3_infra_rel/lib
    ${MODULE_ROOT_PATH}/third_open_src/boost_rel/lib
    ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/libs
    ${MODULE_ROOT_PATH}/third_open_src/openssl_rel/lib
    ${MODULE_ROOT_PATH}/third_open_src/lnfs_rel/lib
    ${MODULE_ROOT_PATH}/third_open_src/lsmb2_rel/lib
    ${MODULE_ROOT_PATH}/third_open_src/esdk_rel/lib
    ${MODULE_ROOT_PATH}/third_open_src/icu_rel/libs/lib
    ${MODULE_ROOT_PATH}/third_open_src/libssh2_rel/lib
    ${MODULE_ROOT_PATH}/third_open_src/libiconv_rel/lib
    ${SCANNER_ROOT_PATH}/build-cmake-file
)

set (SCANNER_DEMO_LIBS
    Scanner
    log_util
    common_util
    securec
    pthread
    json
    parser
    kmcv3
    curl
    ssh2
    ssl
    threadpool
    crypto
    boost_chrono
    boost_atomic
    boost_system
    boost_thread
    boost_regex
    boost_log
    boost_date_time
    boost_filesystem
    system
    config_reader_util
    device_access
    curl_http_util
    icudata
    icuuc
    icui18n
    iconv
)

if (NAS)
    set(SCANNER_DEMO_LIBS
        ${SCANNER_DEMO_LIBS}
        smb_ctx
        nfs_ctx
        nfs
        smb2
    )
endif()


if (OBS)
    set(SCANNER_DEMO_LIBS
        ${SCANNER_DEMO_LIBS}
        obs_ctx
    )
endif()

target_link_libraries(ScannerDemo
    PRIVATE
    ${SCANNER_DEMO_LIBS}
)