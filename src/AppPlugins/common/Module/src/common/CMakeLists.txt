MESSAGE("Compiling common, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/path.cmake)
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(MODULE_ROOT_PATH ${MODULE_PATH}/..)

if(TARGET log_util)
    MESSAGE(STATUS "log_util existed")
else()
    add_subdirectory(${MODULE_PATH}/log log_util)
endif()

file(GLOB_RECURSE common_srcs *.cpp)

add_library(common_util
    SHARED
    ${common_srcs}
    )

target_include_directories(common_util
    PUBLIC
    ${MODULE_PATH}
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/jsoncpp_rel/include
)

target_link_directories(common_util
    PUBLIC
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPEN_SRC}/jsoncpp_rel/libs
)

target_link_libraries(common_util
    PUBLIC
    safe_cmplib
    dl
    pthread
    securec
    log_util
    json
)
