# cmake_minimum_required(VERSION 3.16)
project(FilePlugin)

add_definitions(-DBOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)
# 定义工程需要的变量
SET(PLUGIN_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
SET(PLUGIN_FRAMEWORK_PATH ${PLUGIN_ROOT_PATH}/common/framework)
include(${PLUGIN_FRAMEWORK_PATH}/build/cmake/global.cmake)

SET(PLUGIN_SCANNER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../common/FS_Scanner)
SET(PLUGIN_BACKUP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../common/FS_Backup)
SET(PLUGIN_COMMON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/common)
SET(PLUGIN_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})
SET(PLUGIN_FRAMEWORK_LIB_PATH ${PLUGIN_FRAMEWORK_PATH}/lib)

SET(MODULE_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../common/Module)
SET(OPEN_SRC_PATH ${MODULE_ROOT_PATH}/third_open_src)

SET(DME_LIB_PATH ${PLUGIN_FRAMEWORK_LIB_PATH})
SET(MODULE_PLATFORM_PATH ${MODULE_ROOT_PATH}/platform)
SET(OPENSRC_THRIFT_INC_PATH ${MODULE_ROOT_PATH}/third_open_src/thrift_rel/include/)

# 打开编译告警
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
add_definitions(-DBOOST_FILESYSTEM_DEPRECATED)

# 定义依赖Scanner相关的依赖接口
add_library(scanner_interface
    INTERFACE
    IMPORTED
    GLOBAL
)
 
target_include_directories(scanner_interface
    INTERFACE
    ${PLUGIN_SCANNER_PATH}/include
    ${PLUGIN_SCANNER_PATH}/include/common
    ${PLUGIN_SCANNER_PATH}/include/filter
    ${PLUGIN_SCANNER_PATH}/include/interface
    ${PLUGIN_SCANNER_PATH}/include/model
    ${PLUGIN_SCANNER_PATH}/include/service
    ${PLUGIN_SCANNER_PATH}/include/statistics
    ${PLUGIN_SCANNER_PATH}/include/utils
    ${PLUGIN_SCANNER_PATH}/include/checkpoint
)
 
target_link_directories(scanner_interface
    INTERFACE
    ${MODULE_ROOT_PATH}/
)
 
target_link_libraries(scanner_interface
    INTERFACE
)
   
# 编译子目录
add_subdirectory(common)
add_subdirectory(utils)
add_subdirectory(component)
add_subdirectory(file_service)
add_subdirectory(job)

# 生成插件动态库
add_library(file_plugin
    SHARED
)

target_link_directories(file_plugin
    PRIVATE
    ${PLUGIN_SCANNER_PATH}/lib
    ${PLUGIN_BACKUP_PATH}/build-cmake
    ${PLUGIN_FRAMEWORK_PATH}/dep/agent_sdk/lib
)

target_link_libraries(file_plugin
    PRIVATE
    app_service_obj
    common_dep
    job_obj
    utility_obj
    application_obj
    safe_cmplib
    host_obj
    host_backup_obj
    host_restore_obj
    host_livemount_obj
    host_index_obj
    host_archive_restore_obj
    snapshot_provider_obj
    component_obj
    Scanner
    Backup
    pluginsdk
)

# 插件使用的外部依赖项，比如开源三方、插件框架、Module
add_library(common_dep
    INTERFACE
    IMPORTED
    GLOBAL
)

target_link_directories(common_dep
    INTERFACE
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/lib
    ${OPEN_SRC_PATH}/boost_rel/lib
    ${OPEN_SRC_PATH}/openssl_rel/lib
    ${OPEN_SRC_PATH}/jsoncpp_rel/libs
    ${OPEN_SRC_PATH}/libuuid_rel/lib
    ${DME_LIB_PATH}
    ${DME_LIB_PATH}/3rd
    ${PLUGIN_FRAMEWORK_LIB_PATH}
    ${PLUGIN_FRAMEWORK_LIB_PATH}/thrift_interface
    ${MODULE_ROOT_PATH}/lib
)

target_link_libraries(common_dep
    INTERFACE
    safe_cmplib
    securec
    json
    ssl
    thrift_interface
    thrift_client
    rpc
    common_util
    config_reader_util
    log_util
    parser
    metafile_parser
    system
    boost_filesystem
)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(common_dep
        INTERFACE
        uuid
        boost_chrono
        boost_atomic
        boost_thread
        boost_system
        boost_regex
        boost_log
        boost_date_time
    )
endif()