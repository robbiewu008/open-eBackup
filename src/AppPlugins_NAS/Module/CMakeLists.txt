cmake_minimum_required(VERSION 3.16)
project(Module)

# 查看目标库是否存在
if(TARGET safe_cmplib)
    MESSAGE(STATUS "safe_cmplib existed")
else()
    include(${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/global.cmake)
endif()
add_compile_options(-fPIC)

add_subdirectory(src)

add_library(module
    SHARED
)

target_include_directories(opensrc_dep
    ${LIBNFS_OPENSRC_DIR}/include
)

target_link_libraries(module
    PRIVATE
    threadpool_obj
    metafile_parser_obj
    parser_obj
    libnfs_ctx_obj
    libsmb_ctx_obj
)
