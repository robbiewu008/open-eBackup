INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../../build/cmake/global.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../../build/cmake/path.cmake)
# 递归添加当前目录下所有的cpp文件
file(GLOB_RECURSE sec_cmd_src *.cpp)

add_library(sec_cmd_obj
    OBJECT
    ${sec_cmd_src}
)

target_include_directories(sec_cmd_obj
    PUBLIC
    ${SECURITY_MODULE_INC}
    ${OPEN_SRC_PATH}/boost_rel/include
)

target_link_directories(sec_cmd_obj
    PUBLIC
    ${OPEN_SRC_PATH}/boost_rel/lib
)

target_link_libraries(sec_cmd_obj
    PUBLIC
    safe_cmplib
    boost_system
    boost_filesystem    
)