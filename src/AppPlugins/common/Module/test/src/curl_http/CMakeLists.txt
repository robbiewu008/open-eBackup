MESSAGE("Compiling curl_http code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE curl_http_test_src *.cpp)

add_library(curl_http_test
    OBJECT
    ${curl_http_test_src}
)

target_include_directories(curl_http_test
    PUBLIC
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/third_open_src/openssl_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/boost_rel/include
    ${MODULE_ROOT_PATH}/platform/KMCv3_infra_rel/include
    ${MODULE_ROOT_PATH}/platform/KMCV3_rel/sdp
    ${PLATFORM_PATH}/SecureCLib_rel/include
    ${DT_UTILS_PATH}/llt_stub
)

target_link_directories(curl_http_test
    PUBLIC
    ${PLATFORM_PATH}/SecureCLib_rel/lib
    ${MODULE_ROOT_PATH}/third_open_src/openssl_rel/lib
    ${MODULE_ROOT_PATH}/build-cmake/curl_http/
)

target_link_libraries(curl_http_test
    PUBLIC
    securec
    log_util
    config_reader_util
    curl_http_util
)
