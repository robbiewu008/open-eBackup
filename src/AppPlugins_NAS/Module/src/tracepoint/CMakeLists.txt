MESSAGE("Compiling tracepoint, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/path.cmake)
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)

MESSAGE("OPEN_SRC: ${OPEN_SRC}")
MESSAGE("PLATFORM: ${PLATFORM}")

file(GLOB_RECURSE tracepoint_srcs *.cpp)

add_library(tracepoint
    SHARED
    ${tracepoint_srcs}
)

target_include_directories(tracepoint
    PUBLIC
    ${MODULE_PATH}
    ${OPEN_SRC}/jsoncpp_rel/include
    ${OPEN_SRC}/boost_rel/include
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/tinyxml2_rel/include
)

target_link_directories(tracepoint
    PUBLIC
    ${OPEN_SRC}/jsoncpp_rel/libs
    ${OPEN_SRC}/boost_rel/lib
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPEN_SRC}/tinyxml2_rel/lib
)

target_link_libraries(tracepoint
    PUBLIC
    safe_cmplib
    securec
)
