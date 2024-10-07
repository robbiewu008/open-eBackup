MESSAGE("Compiling device_access code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
SET(MODULE_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(OPEN_SRC ${MODULE_SRC_PATH}/../third_open_src)
SET(PLATFORM ${MODULE_SRC_PATH}/../platform)

add_compile_options("-fPIC")

# 查看目标库log_util是否存在
if(TARGET log_util)
    MESSAGE(STATUS "log_util existed")
else()
    add_subdirectory(${MODULE_SRC_PATH}/log log_util)
endif()

# 查看目标库log_util是否存在
if(TARGET curl_http_util)
    MESSAGE(STATUS "curl_http_util existed")
else()
    add_subdirectory(${MODULE_SRC_PATH}/curl_http curl_http)
endif()

# 查看目标库config_reader_util是否存在
if(TARGET config_reader_util)
    MESSAGE(STATUS "config_reader_util existed")
else()
    add_subdirectory(${MODULE_SRC_PATH}/config_reader_util config_reader_util)
endif()

add_subdirectory(k8s)
add_subdirectory(dorado)
add_subdirectory(oceanstor)
add_subdirectory(fusionstorage)
add_subdirectory(netapp)
add_subdirectory(cache)

add_library(device_access
    SHARED
)

target_link_directories(device_access
    PUBLIC
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPEN_SRC}/boost_rel/lib
    ${OPEN_SRC}/jsoncpp_rel/libs
    ${OPEN_SRC}/curl_rel/include
)

target_link_libraries(device_access
    PRIVATE
    safe_cmplib
    securec
    log_util
    curl_http_util
    config_reader_util
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_thread
    boost_log
    boost_date_time

    k8s_obj
    dorado_device_obj
    oceanstor_device_obj
    fusionstorage_device_obj
    netapp_device_obj
    session_device_obj
)
