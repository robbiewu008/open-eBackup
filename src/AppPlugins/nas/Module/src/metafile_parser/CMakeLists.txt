file(GLOB_RECURSE metafile_parser_srcs *.cpp)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)

SET(PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/../../platform)
set(OPENSRC_INC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../third_open_src")
set(BOOST_PATH "${OPENSRC_INC_PATH}/boost_rel")
set(NFS_PATH "${OPENSRC_INC_PATH}/lnfs_rel")

add_library(metafile_parser
    SHARED
    ${metafile_parser_srcs}
)

target_include_directories(metafile_parser
    PRIVATE
    ${PLATFORM}/SecureCLib_rel/include
    ${NFS_PATH}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../log
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    PUBLIC
    ${BOOST_PATH}/include/
    ${CMAKE_CURRENT_SOURCE_DIR}
    
)

target_link_directories(metafile_parser
    PUBLIC
    ${BOOST_PATH}/lib/
    ${PLATFORM}/SecureCLib_rel/lib
)

target_link_libraries(metafile_parser
    PUBLIC
    log_util
    safe_cmplib
    log_util
    securec
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_thread
    boost_log
    boost_date_time
)