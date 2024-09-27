# 递归添加 aio 所有cpp文件
file(GLOB_RECURSE libaio_srcs *.cpp)

set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_PATH}/..")

add_library(libaio_obj
    OBJECT
    ${libaio_srcs}
)

target_include_directories(libaio_obj
    PRIVATE
    ${BACKUP_SRC_PATH}/
)