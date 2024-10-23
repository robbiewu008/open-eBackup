cmake_minimum_required(VERSION 3.5)

project(virtualization_plugin)
add_definitions(-DBOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)
add_compile_options(-fPIC -ftest-coverage)
set(CMAKE_CXX_FLAGS -g)

# set project variable
if (OPENSOURCE)
    set(PROTECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../common)
    set(VIRT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
    set(MODULE_ROOT_PATH ${PROTECT_ROOT_DIR}/Module)
    set(MODULE_OPEN_SRC_PATH ${MODULE_ROOT_PATH}/third_open_src)
    set(MODULE_PLATFORM_PATH ${MODULE_ROOT_PATH}/platform)
else()
    set(PROTECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
    set(VIRT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
    set(MODULE_ROOT_PATH ${PROTECT_ROOT_DIR}/Module)
    set(MODULE_OPEN_SRC_PATH ${MODULE_ROOT_PATH}/third_open_src)
    set(MODULE_PLATFORM_PATH ${MODULE_ROOT_PATH}/platform)
endif()

find_package(Python3 COMPONENTS Interpreter Development)
include_directories(${Python3_INCLUDE_DIRS})

include(${PROTECT_ROOT_DIR}/plugins/virtualization/build/cmake/global.cmake)

if (TRACEPOINT)
    add_definitions(-DEBK_TRACE_POINT)
endif()

set(FRAMEWORK_LIBS
    ${PROTECT_ROOT_DIR}/framework/lib
    ${PROTECT_ROOT_DIR}/framework/3rd
    ${PROTECT_ROOT_DIR}/framework/lib/agent_sdk
    ${MODULE_OPEN_SRC_PATH}/lz4_rel/lib
    ${MODULE_OPEN_SRC_PATH}/boost_rel/lib
    ${MODULE_OPEN_SRC_PATH}/jsoncpp_rel/libs
    ${MODULE_OPEN_SRC_PATH}/curl_rel/lib/
    ${MODULE_OPEN_SRC_PATH}/thrift_rel/lib
    ${MODULE_OPEN_SRC_PATH}/libaio_rel/lib
    ${MODULE_OPEN_SRC_PATH}/tinyxml2_rel/lib
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/lib
)

message(STATUS "FRAMEWORK_LIBS: ${FRAMEWORK_LIBS}")

set(PLUGIN_FRAMEWORK_INC
    ${PROTECT_ROOT_DIR}/framework/inc
    ${PROTECT_ROOT_DIR}/framework/inc/common
    ${PROTECT_ROOT_DIR}/framework/inc/client
    ${PROTECT_ROOT_DIR}/framework/inc/rpc
    ${PROTECT_ROOT_DIR}/framework/inc/rpc/certificateservice/
    ${PROTECT_ROOT_DIR}/framework/inc/thrift_interface

    ${MODULE_ROOT_PATH}/src
    ${MODULE_OPEN_SRC_PATH}/boost_rel/include
    ${MODULE_OPEN_SRC_PATH}/lz4_rel/include
    ${MODULE_OPEN_SRC_PATH}/jsoncpp_rel/include
    ${MODULE_OPEN_SRC_PATH}/curl_rel/include
    ${MODULE_OPEN_SRC_PATH}/openssl_rel/include
    ${MODULE_OPEN_SRC_PATH}/thrift_rel/include
    ${MODULE_OPEN_SRC_PATH}/libaio_rel/include
    ${MODULE_OPEN_SRC_PATH}/esdk_rel/include
    ${MODULE_OPEN_SRC_PATH}/tinyxml2_rel/include
    ${MODULE_OPEN_SRC_PATH}/libssh2_rel/include
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
)

add_subdirectory(src)

add_library(virtualization_plugin SHARED src/PluginMain.cpp)

if (TRACEPOINT)
target_link_libraries(virtualization_plugin
    PRIVATE
    security_cbb
    com_obj
    repo_factory_obj
    virtualization_job_factory_obj
    fs_handler_obj
    io_schedulerobj
    jobs_obj
    engine_factory_obj
    volume_common_obj
    oceanstor_obj
    cloud_volume_obj
    volume_fusionstorage_obj
    aio_datamoverobj
    thrift_client
    thrift_interface
    utils
    rpc
    kubernetes_engine
    curl_http_util
    hcs_engine
    openstack_engine
    cnware_engine
    apsara_stack_engine

    PUBLIC
    pluginsdk
    basic_job
    lz4
    -laio
    common_util
    config_reader_util
    log_util
    system
    tracepoint
    tinyxml2
    param_checker
    -lpython3

    vir_safe_cmplib
)
else()
target_link_libraries(virtualization_plugin
    PRIVATE
    security_cbb
    com_obj
    repo_factory_obj
    virtualization_job_factory_obj
    fs_handler_obj
    io_schedulerobj
    jobs_obj
    engine_factory_obj
    volume_common_obj
    oceanstor_obj
    cloud_volume_obj
    volume_fusionstorage_obj
    aio_datamoverobj
    thrift_client
    thrift_interface
    utils
    rpc
    kubernetes_engine
    curl_http_util
    hcs_engine
    openstack_engine
    cnware_engine
    apsara_stack_engine

    PUBLIC
    pluginsdk
    basic_job
    lz4
    -laio
    common_util
    config_reader_util
    log_util
    system
    tinyxml2
    param_checker
    -lpython3

    vir_safe_cmplib
)
endif()



target_link_directories(virtualization_plugin
    PUBLIC
    ${FRAMEWORK_LIBS}
    ${VIRT_ROOT_DIR}/lib/service
    ${VIRT_ROOT_DIR}/lib/3rd
)

target_include_directories(virtualization_plugin
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/job_controller
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protect_engines
    ${CMAKE_CURRENT_SOURCE_DIR}/src/repository_handlers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/volume_handlers
    ${PLUGIN_FRAMEWORK_INC}
    ${PLUGIN_FRAMEWORK_INC}/utils
    ${PROTECT_ROOT_DIR}/framework
)
