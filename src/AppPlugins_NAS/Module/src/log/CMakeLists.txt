MESSAGE("Compiling log, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/path.cmake)
SET(FRAMEWORK_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)

MESSAGE("OPEN_SRC: ${OPEN_SRC}")
MESSAGE("PLATFORM: ${PLATFORM}")

file(GLOB_RECURSE log_util_srcs *.cpp)

add_library(log_util
    SHARED
    ${log_util_srcs}
)

target_include_directories(log_util
    PUBLIC
    ${FRAMEWORK_BASE_PATH}
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/boost_rel/include
)

target_link_directories(log_util
    PUBLIC
    ${OPEN_SRC}/boost_rel/lib
    ${PLATFORM}/SecureCLib_rel/lib
)

target_link_libraries(log_util
    PUBLIC
    boost_filesystem
    safe_cmplib
    securec
)
