MESSAGE(STATUS "Current path: ${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-fPIC)
file(GLOB_RECURSE plugin_srcs *.cpp)

add_library(host_obj
    OBJECT
    ${plugin_srcs}
)
message( ${PLUGIN_SCANNER_PATH})
target_include_directories(host_obj
    PUBLIC
    ${PLUGIN_FRAMEWORK_PATH}/inc
    ${PLUGIN_FRAMEWORK_PATH}/inc/client
    ${PLUGIN_FRAMEWORK_PATH}/inc/common
    ${PLUGIN_FRAMEWORK_PATH}/inc/job
    ${PLUGIN_FRAMEWORK_PATH}/inc/job_mgr
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc/certificateservice
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc/servicefactory
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc/thriftservice
    ${PLUGIN_FRAMEWORK_PATH}/inc/thrift_interface
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_SRC_PATH}/common
    ${PLUGIN_SRC_PATH}/component/statistics
    ${PLUGIN_SRC_PATH}/component
    ${PLUGIN_BACKUP_PATH}/src/model
    ${PLUGIN_BACKUP_PATH}/src/service
    ${PLUGIN_BACKUP_PATH}/src/common
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/src/parser
    ${MODULE_ROOT_PATH}/src/metafile_parser
    ${MODULE_ROOT_PATH}/src/common
    ${MODULE_ROOT_PATH}/src/libnfs_ctx
    ${MODULE_ROOT_PATH}/src/threadpool
    ${MODULE_ROOT_PATH}/src/libsmb_ctx
    ${PLUGIN_ROOT_PATH}/common
    ${OPENSRC_THRIFT_INC_PATH}
    ${OPEN_SRC_PATH}/boost_rel/include
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${OPEN_SRC_PATH}/jsoncpp_rel/include
    ${PLUGIN_SCANNER_PATH}/include
    ${PLUGIN_SCANNER_PATH}/include/common
    ${PLUGIN_SCANNER_PATH}/include/checkpoint
    ${PLUGIN_SCANNER_PATH}/include/filter
    ${PLUGIN_SCANNER_PATH}/include/interface
    ${PLUGIN_SCANNER_PATH}/include/model
    ${PLUGIN_SCANNER_PATH}/include/service
    ${PLUGIN_SCANNER_PATH}/include/statistics
    ${PLUGIN_SCANNER_PATH}/include/utils
)
 
target_link_libraries(host_obj
    PRIVATE
    basic_job
    common_dep
)
