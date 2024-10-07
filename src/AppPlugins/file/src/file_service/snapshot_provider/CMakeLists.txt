# 递归添加snapshot_provider目录下所有的cpp文件
file(GLOB_RECURSE snapshot_provider_srcs *.cpp)

add_library(snapshot_provider_obj
    OBJECT
    ${snapshot_provider_srcs}
)

target_include_directories(snapshot_provider_obj
    PRIVATE
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_SRC_PATH}/common
    ${MODULE_ROOT_PATH}/src
    ${OPEN_SRC_PATH}/boost_rel/include
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${MODULE_ROOT_PATH}/src/common
)

target_link_libraries(snapshot_provider_obj
    PRIVATE
    common_dep
)