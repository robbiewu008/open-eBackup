# 递归添加目录下的所有.cpp文件
file(GLOB_RECURSE bmr_srcs *.cpp)

set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")

add_library(bare_metal_recovery_obj
    OBJECT
    ${bmr_srcs}
)
 
target_include_directories(bare_metal_recovery_obj
    PRIVATE
    ${BACKUP_SRC_PATH}/common/
    ${BACKUP_SRC_PATH}/model/
    ${MODULE_ROOT_PATH}/src/
    ${MODULE_ROOT_PATH}/src/common/
    ${MODULE_ROOT_PATH}/third_open_src/boost_rel/include
)