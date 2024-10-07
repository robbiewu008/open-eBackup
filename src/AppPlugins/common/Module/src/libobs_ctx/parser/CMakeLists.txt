# 递归添加当前目录下所有的cpp文件
file(GLOB_RECURSE obs_parser_srcs *.cpp)

add_library(obs_parser_obj
    OBJECT
    ${obs_parser_srcs}
)

target_include_directories(obs_parser_obj
    PUBLIC
    ${STORAGE_COMMON_INC}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../parser
)

target_link_directories(obs_parser_obj
    PUBLIC
)

target_link_libraries(obs_parser_obj
    PUBLIC
    parser
)