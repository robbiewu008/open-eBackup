MESSAGE("Compiling config_reader code, OPEN_SRC is ${OPEN_SRC}")
MESSAGE("Compiling config_reader code, PLATFORM is ${PLATFORM}")

file(GLOB_RECURSE oceanstor_srcs *.cpp)

add_library(oceanstor_device_obj
    OBJECT
    ${oceanstor_srcs}
)

target_include_directories(oceanstor_device_obj
    PUBLIC
    ${MODULE_SRC_PATH}
    ${MODULE_SRC_PATH}/src
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/boost_rel/include
    ${OPEN_SRC}/jsoncpp_rel/include
)

target_link_directories(oceanstor_device_obj
    PUBLIC
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPEN_SRC}/boost_rel/lib
    ${OPEN_SRC}/jsoncpp_rel/libs
)

target_link_libraries(oceanstor_device_obj
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
    config_reader_util
)
