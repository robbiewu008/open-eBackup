MESSAGE("Compiling curl_http code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/path.cmake)
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
SET(FRAMEWORK_PATH ${MODULE_PATH}/../framework)

file(GLOB_RECURSE curl_http_srcs *.cpp)

add_library(curl_http_util
    SHARED
    ${curl_http_srcs}
)

target_include_directories(curl_http_util
    PUBLIC
    ${MODULE_PATH}/src
    ${MODULE_PATH}/platform/KMCv3_infra_rel/include
    ${MODULE_PATH}/platform/KMCV3_rel/include
    ${MODULE_PATH}/platform/KMCV3_rel/sdp
    ${OPEN_SRC}/jsoncpp_rel/include
    ${OPEN_SRC}/openssl_rel/include
    ${OPEN_SRC}/boost_rel/include
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/curl_rel/include
)

target_link_directories(curl_http_util
    PUBLIC
    ${FRAMEWORK_PATH}/lib
    ${OPEN_SRC}/openssl_rel/lib
    ${PLATFORM}/SecureCLib_rel/lib
    ${PLATFORM}/KMCv3_infra_rel/lib
    ${OPEN_SRC}/boost_rel/lib
    ${OPEN_SRC}/curl_rel/lib
)

target_link_libraries(curl_http_util
    PUBLIC
    securec
    log_util
    ssl
    crypto
    curl
    kmcv3
    common_util
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_thread
    boost_log
    boost_date_time
)
