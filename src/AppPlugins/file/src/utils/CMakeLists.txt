# 递归添加nas_service目录下所有的cpp文件
file(GLOB utility_srcs *.cpp)

add_library(utility_obj
    OBJECT
    ${utility_srcs}
)

target_include_directories(utility_obj
    PRIVATE
    ${PLUGIN_FRAMEWORK_PATH}/inc/thrift_interface
    ${PLUGIN_FRAMEWORK_PATH}/inc
    ${DME_FRAMEWORK_PATH}/src/inc
    ${PLUGIN_COMMON_PATH}/application
    ${PLUGIN_COMMON_PATH}/constant
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_ROOT_PATH}
    ${OPEN_SRC_PATH}/boost_rel/include
    ${MODULE_ROOT_PATH}/src
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
)

target_link_libraries(utility_obj
    PRIVATE
    common_dep
)