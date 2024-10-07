MESSAGE("Compiling io_device code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/path.cmake)
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)

file(GLOB_RECURSE io_device_srcs *.cpp)
file(GLOB_RECURSE io_device_hpp_srcs *.hpp)
# 查看目标库是否存在
if(TARGET log_util)
    MESSAGE(STATUS "log_util existed")
else()
    add_subdirectory(${MODULE_PATH}/log log_util)
endif()
# 查看目标库是否存在
if(TARGET common_util)
    MESSAGE(STATUS "common_util existed")
else()
    add_subdirectory(${MODULE_PATH}/common common)
endif()
# 查看目标库是否存在
if(TARGET system)
    MESSAGE(STATUS "system existed")
else()
    add_subdirectory(${MODULE_PATH}/system system)
endif()
# 查看目标库是否存在
if(TARGET config_reader_util)
    MESSAGE(STATUS "config_reader_util existed")
else()
    add_subdirectory(${MODULE_PATH}/config_reader config_reader_util)
endif()


add_library(io_device
    SHARED
    ${io_device_srcs}
    ${io_device_hpp_srcs}
)
MESSAGE("PLATFORM is ${PLATFORM}")
MESSAGE("OPEN_SRC is ${OPEN_SRC}")
target_include_directories(io_device
    PUBLIC
    ${MODULE_PATH}
    ${MODULE_PATH}/io_device
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/jsoncpp_rel/include
    ${OPEN_SRC}/esdk_rel/include
    ${OPEN_SRC}/openssl_rel/include
    ${OPEN_SRC}/sqlite_rel/sqlite-autoconf/include
    ${OPEN_SRC}/boost_rel/include
)

target_link_directories(io_device
    PUBLIC
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPEN_SRC}/jsoncpp_rel/libs
    ${OPEN_SRC}/esdk_rel/lib
    ${OPEN_SRC}/openssl_rel/lib
    ${OPEN_SRC}/sqlite_rel/sqlite-autoconf/lib
    ${OPEN_SRC}/boost_rel/lib
    ${OPEN_SRC}/libiconv_rel/lib
)

target_link_libraries(io_device
    PUBLIC
    pthread
    securec
    log_util
    eSDKOBS
    iconv
    system
    config_reader_util
    common_util
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_regex
    boost_log
    boost_date_time
)