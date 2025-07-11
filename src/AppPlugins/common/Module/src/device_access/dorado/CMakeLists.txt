
file(GLOB_RECURSE dorado_srcs *.cpp)

add_library(dorado_device_obj
    OBJECT
    ${dorado_srcs}
)

target_include_directories(dorado_device_obj
    PUBLIC
    ${MODULE_SRC_PATH}
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/boost_rel/include
    ${OPEN_SRC}/jsoncpp_rel/include
    ${OPEN_SRC}/curl_rel/include
    ${OPEN_SRC}/curl_rel/include
    ${OPEN_SRC}/tinyxml2_rel/include
    ${MODULE_SRC_PATH}/src

)

target_link_directories(dorado_device_obj
    PUBLIC
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPEN_SRC}/boost_rel/lib
    ${OPEN_SRC}/jsoncpp_rel/libs
)

target_link_libraries(dorado_device_obj
    PUBLIC
    securec
    log_util

    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_thread
    boost_log
    boost_date_time
    curl_http_util
)
