MESSAGE(STATUS "Current path: ${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-fPIC)

add_library(volume_obj
    OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/VolumeCommonService.cpp
)
message( ${PLUGIN_SCANNER_PATH})
target_include_directories(volume_obj
    PUBLIC
    #plugin framework included path
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
    #plugin included path
    ${PLUGIN_ROOT_PATH}
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_SRC_PATH}/common
    ${PLUGIN_SRC_PATH}/common/constant
    ${PLUGIN_SRC_PATH}/component
    ${PLUGIN_SRC_PATH}/component/statistics
    ${PLUGIN_SRC_PATH}/utils
    #FS_Backup included path
    ${PLUGIN_BACKUP_PATH}/src/model
    ${PLUGIN_BACKUP_PATH}/src/service
    ${PLUGIN_BACKUP_PATH}/src/common
    ${PLUGIN_BACKUP_PATH}/src/volume
    ${PLUGIN_BACKUP_PATH}/src/volume/common
    #Module src included path
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/src/common
    ${MODULE_ROOT_PATH}/src/log
    ${MODULE_ROOT_PATH}/src/parser
    ${MODULE_ROOT_PATH}/src/system
    ${MODULE_ROOT_PATH}/src/threadpool
    #Module open third-party included path
    ${OPEN_SRC_PATH}/boost_rel/include
    ${OPEN_SRC_PATH}/jsoncpp_rel/include
    ${OPEN_SRC_PATH}/thrift_rel/include
    #Module platform included path
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    # FS_Scanner
    ${PLUGIN_SCANNER_PATH}/localhost_src
    ${PLUGIN_SCANNER_PATH}/localhost_src/common
    ${PLUGIN_SCANNER_PATH}/localhost_src/checkpoint
    ${PLUGIN_SCANNER_PATH}/localhost_src/filter
    ${PLUGIN_SCANNER_PATH}/localhost_src/interface
    ${PLUGIN_SCANNER_PATH}/localhost_src/model
    ${PLUGIN_SCANNER_PATH}/localhost_src/service
    ${PLUGIN_SCANNER_PATH}/localhost_src/statistics
    ${PLUGIN_SCANNER_PATH}/localhost_src/utils
)

target_link_libraries(volume_obj
    PRIVATE
    scanner_interface
    basic_job
    common_dep
)