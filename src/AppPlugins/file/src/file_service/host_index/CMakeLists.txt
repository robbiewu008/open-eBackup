add_library(host_index_obj
    OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/HostIndex.cpp
)

SET(PLUGIN_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Module)
MESSAGE("MODULE_PATH : ${PLUGIN_MODULE_PATH}")

target_include_directories(host_index_obj
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
    ${PLUGIN_MODULE_PATH}/third_open_src/thrift_rel/include
    ${PLUGIN_MODULE_PATH}/third_open_src/boost_rel/include
    ${PLUGIN_MODULE_PATH}/src
    ${PLUGIN_MODULE_PATH}/src/libsmb_ctx
    ${PLUGIN_SCANNER_PATH}/interface
    ${PLUGIN_ROOT_PATH}
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${OPEN_SRC_PATH}/jsoncpp_rel/include
    ${PLUGIN_SCANNER_PATH}/localhost_src/common
    ${PLUGIN_SCANNER_PATH}/localhost_src/checkpoint
    ${PLUGIN_SCANNER_PATH}/localhost_src/filter
    ${PLUGIN_SCANNER_PATH}/localhost_src/interface
    ${PLUGIN_SCANNER_PATH}/localhost_src/model
    ${PLUGIN_SCANNER_PATH}/localhost_src/service
    ${PLUGIN_SCANNER_PATH}/localhost_src/statistics
    ${PLUGIN_SCANNER_PATH}/localhost_src/utils
)

target_link_libraries(host_index_obj
    PRIVATE
    common_dep
)