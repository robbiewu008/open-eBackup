file(GLOB_RECURSE parser_srcs *.cpp)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)

SET(PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/../../platform)
set(OPENSRC_INC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../third_open_src")
set(BOOST_PATH "${OPENSRC_INC_PATH}/boost_rel")
set(NFS_PATH "${OPENSRC_INC_PATH}/lnfs_rel")
set(OPENSSL_PATH "${OPENSRC_INC_PATH}/openssl_rel")

add_library(parser
    SHARED
    ${parser_srcs}
)

if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "AIX")
    add_definitions(-D__STDC_FORMAT_MACROS)
elseif (CMAKE_SYSTEM_NAME MATCHES "SunOS")
    add_definitions(-D__STDC_FORMAT_MACROS)
endif()

target_include_directories(parser
    PRIVATE
    ${BOOST_PATH}/include/
    ${NFS_PATH}/include
    ${OPENSSL_PATH}/include
    ${PLATFORM}/SecureCLib_rel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../log
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_directories(parser
    PUBLIC
    ${BOOST_PATH}/lib/
    ${PLATFORM}/SecureCLib_rel/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../build-cmake/log
)

target_link_libraries(parser
    PUBLIC
    safe_cmplib
    securec
    log_util
    common_util
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_thread
    boost_log
    boost_date_time
    system
    json
)