MESSAGE("Compiling config_reader code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/path.cmake)
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
# 查看目标库是否存在
if(TARGET common_util)
    MESSAGE(STATUS "common_util existed")
else()
    add_subdirectory(${MODULE_PATH}/common common)
endif()

file(GLOB_RECURSE config_reader_srcs *.cpp)

add_library(config_reader_util
    SHARED
    ${config_reader_srcs}
)

target_include_directories(config_reader_util
    PUBLIC
    ${MODULE_PATH}
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/tinyxml2_rel/include
)

target_link_directories(config_reader_util
    PUBLIC
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPEN_SRC}/tinyxml2_rel/lib
)

target_link_libraries(config_reader_util
    PUBLIC
    safe_cmplib
    securec
    tinyxml2
    log_util
    common_util
)
