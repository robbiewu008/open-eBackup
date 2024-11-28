file(GLOB_RECURSE param_checker_srcs *.cpp)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/path.cmake)
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
MESSAGE("Compiling param_checker code, PATH is ${MODULE_PATH}")

add_library(param_checker
    SHARED
    ${param_checker_srcs}
)

target_include_directories(param_checker
    PRIVATE
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/boost_rel/include
    ${OPEN_SRC}/tinyxml2_rel/include
    ${OPEN_SRC}/jsoncpp_rel/include

    ${MODULE_PATH}
)

target_link_directories(param_checker
    PRIVATE
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPEN_SRC}/boost_rel/lib
    ${OPEN_SRC}/tinyxml2_rel/lib
    ${OPEN_SRC}/jsoncpp_rel/libs
)

target_link_libraries(param_checker
    PRIVATE
    safe_cmplib
    boost_filesystem
    boost_atomic
    tinyxml2
    log_util
    common_util
    json
)