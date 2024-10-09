add_library(host_index_obj
    OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/HostIndex.cpp
)

MESSAGE("MODULE_PATH : ${MODULE_ROOT_PATH}")

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
    ${MODULE_ROOT_PATH}/third_open_src/thrift_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/boost_rel/include
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/src/libsmb_ctx
    ${PLUGIN_SCANNER_PATH}/interface
    ${PLUGIN_ROOT_PATH}/common
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${OPEN_SRC_PATH}/jsoncpp_rel/include
    ${PLUGIN_SCANNER_PATH}/include/common
    ${PLUGIN_SCANNER_PATH}/include/checkpoint
    ${PLUGIN_SCANNER_PATH}/include/filter
    ${PLUGIN_SCANNER_PATH}/include/interface
    ${PLUGIN_SCANNER_PATH}/include/model
    ${PLUGIN_SCANNER_PATH}/include/service
    ${PLUGIN_SCANNER_PATH}/include/statistics
    ${PLUGIN_SCANNER_PATH}/include/utils
)

target_link_libraries(host_index_obj
    PRIVATE
    common_dep
)