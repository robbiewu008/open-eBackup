MESSAGE("Compiling layout code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/path.cmake)
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)

file(GLOB_RECURSE io_device_srcs *.cpp)

add_library(layout
    SHARED
    ${io_device_srcs}
)

target_include_directories(layout
    PUBLIC
    ${MODULE_PATH}
    ${MODULE_PATH}/layout
    ${MODULE_PATH}/io_device
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/jsoncpp_rel/include
    ${OPEN_SRC}/esdk_rel/include
    ${OPEN_SRC}/openssl_rel/include
    ${OPEN_SRC}/sqlite_rel/sqlite-autoconf/include
    ${OPEN_SRC}/boost_rel/include
)

target_link_directories(layout
    PUBLIC
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPEN_SRC}/jsoncpp_rel/libs
    ${OPEN_SRC}/esdk_rel/lib
    ${OPEN_SRC}/openssl_rel/lib
    ${OPEN_SRC}/sqlite_rel/sqlite-autoconf/lib
    ${OPEN_SRC}/boost_rel/lib
)

target_link_libraries(layout
    PUBLIC
    securec
    log_util
)