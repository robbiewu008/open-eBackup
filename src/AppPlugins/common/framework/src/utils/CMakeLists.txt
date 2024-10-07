MESSAGE("Compiling utilities code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)

# 递归添加thrift目录下所有的cpp文件
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    file(GLOB_RECURSE utilities_srcs *.cpp)
else()
    set(utilities_srcs
        Utils.cpp
        CertCN.cpp
    )
endif()
add_library(utils
    SHARED
    ${utilities_srcs}
)

target_include_directories(utils
    PUBLIC
    ${INC_PATH}/utils
    ${INC_PATH}/common
    ${INC_PATH}/thrift_interface
    ${PLATFORM_PATH}/SecureCLib_rel/include
)

target_link_directories(utils
    PUBLIC
    ${MODULE_ROOT_PATH}/lib
)

target_link_libraries(utils
    PUBLIC
    thrift_interface
    param_checker
    config_reader_util
)