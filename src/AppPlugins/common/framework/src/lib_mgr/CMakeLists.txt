MESSAGE("Compiling lib manager, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)

# 递归添加lib_mgr目录下所有的cpp文件
file(GLOB_RECURSE lib_mgr_srcs *.cpp)

if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "AIX")
    add_library(lib_mgr_obj
        SHARED
        ${lib_mgr_srcs}
    )
else()
    add_library(lib_mgr_obj
        OBJECT
        ${lib_mgr_srcs}
    )
endif()

target_include_directories(lib_mgr_obj
    PUBLIC
    ${INC_PATH}/lib_mgr
    ${INC_PATH}
)
