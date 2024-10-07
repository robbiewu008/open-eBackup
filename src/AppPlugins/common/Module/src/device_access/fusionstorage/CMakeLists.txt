
file(GLOB_RECURSE fusionstorage_srcs *.cpp)
MESSAGE("Compiling config_reader code, OPEN_SRC is ${OPEN_SRC}")
MESSAGE("Compiling config_reader code, PLATFORM is ${PLATFORM}")

add_library(fusionstorage_device_obj
    OBJECT
    ${fusionstorage_srcs}
)

target_include_directories(fusionstorage_device_obj
    PUBLIC
    ${MODULE_SRC_PATH}
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/boost_rel/include
    ${OPEN_SRC}/jsoncpp_rel/include
    ${OPEN_SRC}/curl_rel/include
)

target_link_directories(fusionstorage_device_obj
    PUBLIC
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPEN_SRC}/boost_rel/lib
    ${OPEN_SRC}/jsoncpp_rel/libs
)

target_link_libraries(fusionstorage_device_obj
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
