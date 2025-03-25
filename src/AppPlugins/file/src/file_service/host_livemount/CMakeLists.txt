add_library(host_livemount_obj
    OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/HostLivemount.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HostCancelLivemount.cpp
)

SET(PLUGIN_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Module)
MESSAGE("MODULE_PATH : ${PLUGIN_MODULE_PATH}")

target_include_directories(host_livemount_obj
    PRIVATE
    ${PLUGIN_FRAMEWORK_PATH}/inc/job/
    ${PLUGIN_FRAMEWORK_PATH}/inc/thrift_interface
    ${PLUGIN_FRAMEWORK_PATH}/inc
    ${PLUGIN_FRAMEWORK_PATH}/inc/client
    ${PLUGIN_FRAMEWORK_PATH}/inc/common
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc/certificateservice
    ${PLUGIN_COMMON_PATH}
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_SRC_PATH}/common
    ${PLUGIN_SRC_PATH}/utils
    ${PLUGIN_SRC_PATH}/file_service/snapshot_provider
    ${PLUGIN_MODULE_PATH}/third_open_src/thrift_rel/include
    ${PLUGIN_MODULE_PATH}/third_open_src/boost_rel/include
    ${PLUGIN_MODULE_PATH}/src
    ${PLUGIN_ROOT_PATH}
    ${OPEN_SRC_PATH}/jsoncpp_rel/include
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
)

target_link_libraries(host_livemount_obj
    PRIVATE
    common_dep
)