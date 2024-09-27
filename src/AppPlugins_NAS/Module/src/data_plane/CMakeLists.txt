MESSAGE("Compiling session code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/path.cmake)
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
SET(MODULE_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(OPEN_SRC ${MODULE_SRC_PATH}/../third_open_src)
SET(PLATFORM ${MODULE_SRC_PATH}/../platform)

file(GLOB_RECURSE data_plane_src *.cpp)

add_library(data_plane_util
    SHARED
    ${data_plane_src}
)

add_subdirectory(protocol)

target_include_directories(data_plane_util
    PUBLIC
    ${MODULE_PATH}/src/data_plane
    ${MODULE_PATH}/src
    ${OPEN_SRC}/openssl_rel/include
    ${OPEN_SRC}/boost_rel/include
    ${OPEN_SRC}/libfuse_rel/include
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/jsoncpp_rel/include
)

target_link_directories(data_plane_util
    PUBLIC
    ${OPEN_SRC}/openssl_rel/lib
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPEN_SRC}/boost_rel/lib
    ${OPEN_SRC}/libfuse_rel/lib
)

target_link_libraries(data_plane_util
    PUBLIC
    securec
    log_util
    ssl
    fuse3
    crypto
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_thread
    boost_log
    boost_date_time
    curl_http_util
    ${PROTOCOL_TARGET_NAME}
)
