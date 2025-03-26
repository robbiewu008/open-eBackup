# 递归添加nas_service目录下所有的cpp文件
file(GLOB_RECURSE app_service_srcs AppService.cpp PosixHandler.cpp)

add_library(app_service_obj
    OBJECT
    ${app_service_srcs}
)

target_include_directories(app_service_obj
    PRIVATE
    ${PLUGIN_FRAMEWORK_PATH}/inc/thrift_interface
    ${PLUGIN_FRAMEWORK_PATH}/inc
    ${DME_FRAMEWORK_PATH}/src/inc
    ${PLUGIN_COMMON_PATH}/application
    ${PLUGIN_COMMON_PATH}/constant
    ${PLUGIN_COMMON_PATH}
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_ROOT_PATH}/common
    ${MODULE_ROOT_PATH}/src
    ${OPEN_SRC_PATH}/boost_rel/include
    ${OPEN_SRC_PATH}/jsoncpp_rel/include
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${OPENSRC_THRIFT_INC_PATH}
)

target_link_libraries(app_service_obj
    PRIVATE
    common_dep
    application_obj
)