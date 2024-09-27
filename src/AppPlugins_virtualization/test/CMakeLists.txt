# Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
# Description: CMakeLists.txt
# Author: hejianfan h30009301
# Create: 2022-05-20

cmake_minimum_required(VERSION 3.13)

project(virtualization_plugin_test VERSION 1.0 LANGUAGES C CXX)
add_definitions(-DBOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)
file(GLOB_RECURSE native_dtfuzz_srcs dt_fuzz/src/*.cpp)
set(CMAKE_CXX_FLAGS -g)
message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
option(DTFUZZ "Compile in DTFUZZ mode" OFF)
add_definitions(-DBOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Python3 COMPONENTS Interpreter Development)
include_directories(${Python3_INCLUDE_DIRS})

if(DTFUZZ)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../build/cmake/global.cmake)
else()
    # 包含所有组件的cmake
    include("${CMAKE_CURRENT_SOURCE_DIR}/conf/comp.cmake")
endif()

# LLT打桩私有函数公有化
add_definitions("-fno-access-control --coverage")

# 添加源代码
# 添加测试代码
# -------------------------------------------- common --------------------------------------------
aux_source_directory(src/common COMMON_SRCS)
aux_source_directory(common COMMON_TEST)
aux_source_directory(src/common/sha256 SHA256_SRCS)
aux_source_directory(common/sha256 SHA256_TEST)

aux_source_directory(common/token_mgr TOKEN_MGR_TEST)
aux_source_directory(src/common/token_mgr TOKEN_MGR_SRCS)

aux_source_directory(common/cert_mgr CERT_MGR_TEST)
aux_source_directory(src/common/cert_mgr CERT_MGR_SRCS)

aux_source_directory(common/httpclient HTTP_CLIENT_TEST)
aux_source_directory(src/common/httpclient HTTP_CLIENT_SRCS)

aux_source_directory(common/client REST_CLIENT_TEST)
aux_source_directory(src/common/client REST_CLIENT_SRCS)

# -------------------------------------------- job_controller --------------------------------------------
aux_source_directory(job_controller/io_scheduler SCHEDULER_TEST)
aux_source_directory(src/job_controller/io_scheduler SCHEDULER_SRCS)

aux_source_directory(job_controller/jobs JOBS_TEST)
aux_source_directory(src/job_controller/jobs JOBS_SRCS)
aux_source_directory(job_controller/jobs/backup BACKUP_TEST)
aux_source_directory(src/job_controller/jobs/backup BACKUP_SRCS)
aux_source_directory(job_controller/jobs/verify COPY_VERIFY_TEST)
aux_source_directory(src/job_controller/jobs/verify COPY_VERIFY_SRCS)
aux_source_directory(job_controller/jobs/restore RESTORE_TEST)
aux_source_directory(src/job_controller/jobs/restore RESTORE_SRCS)
aux_source_directory(job_controller/factory FACTORY_TEST)
aux_source_directory(src/job_controller/factory FACTORY_SRCS)

# -------------------------------------------- repository_handlers --------------------------------------------
aux_source_directory(repository_handlers/filesystem FILESYSTEM_TEST)
aux_source_directory(src/repository_handlers/filesystem FILESYSTEM_SRCS)
aux_source_directory(repository_handlers/mock FILESYSTEM_MOCK)

# -------------------------------------------- volume_handlers --------------------------------------------
aux_source_directory(volume_handlers/cinder_volume CINDER_VOLUME_HANDLER_TEST)
aux_source_directory(src/volume_handlers/cloud_volume/cinder_volume CINDER_VOLUME_HANDLER_SRCS)
aux_source_directory(src/volume_handlers/cloud_volume CLOUD_VOLUME_HANDLER_SRCS)
aux_source_directory(volume_handlers/fusionstorage/fusionstorage_api_factory FUSIONSTORAGE_API_FACTORY_TEST)
aux_source_directory(src/volume_handlers/fusionstorage/fusionstorage_api_factory FUSIONSTORAGE_API_FACTORY_SRCS)
aux_source_directory(volume_handlers/fusionstorage/client FUSIONSTORAGE_CLIENT_TEST)
aux_source_directory(src/volume_handlers/fusionstorage/client FUSIONSTORAGE_CLIENT_SRCS)

aux_source_directory(src/volume_handlers/fusionstorage FUSIONSTORAGE_VOLUME_HANDLER_SRCS)
list(REMOVE_ITEM FUSIONSTORAGE_VOLUME_HANDLER_SRCS src/volume_handlers/fusionstorage/FusionStorageApi.cpp)
list(REMOVE_ITEM FUSIONSTORAGE_VOLUME_HANDLER_SRCS src/volume_handlers/fusionstorage/FusionStorageBitmapHandle.cpp)
list(REMOVE_ITEM FUSIONSTORAGE_VOLUME_HANDLER_SRCS src/volume_handlers/fusionstorage/FusionStorageVolumeHandler.cpp)
aux_source_directory(volume_handlers/fusionstorage FUSIONSTORAGE_VOLUME_HANDLER_TEST)
list(REMOVE_ITEM FUSIONSTORAGE_VOLUME_HANDLER_TEST volume_handlers/fusionstorage/FusionStorageApiTest.cpp)
list(REMOVE_ITEM FUSIONSTORAGE_VOLUME_HANDLER_TEST volume_handlers/fusionstorage/FusionStorageBitmapHandleTest.cpp)
list(REMOVE_ITEM FUSIONSTORAGE_VOLUME_HANDLER_TEST volume_handlers/fusionstorage/FusionStorageVolumeHandlerTest.cpp)
# -------------------------------------------- engines_factory --------------------------------------------
aux_source_directory(protect_engines/engine_factory ENGINE_FACTORY_TEST)
aux_source_directory(src/protect_engines/engine_factory ENGINE_FACTORY_SRCS)

# -------------------------------------------- kubernetes --------------------------------------------
aux_source_directory(protect_engines/kubernetes KUBERNETES_TEST)
aux_source_directory(src/protect_engines/kubernetes KUBERNETES_SRCS)
aux_source_directory(protect_engines/kubernetes/rest KUBERNETES_REST_TEST)
aux_source_directory(src/protect_engines/kubernetes/rest KUBERNETES_REST_SRCS)
aux_source_directory(protect_engines/kubernetes/rest/client KUBERNETES_CLIENT_TEST)
aux_source_directory(src/protect_engines/kubernetes/rest/client KUBERNETES_CLIENT_SRCS)
aux_source_directory(protect_engines/kubernetes/rest/config KUBERNETES_CONFIG_TEST)
aux_source_directory(src/protect_engines/kubernetes/rest/config KUBERNETES_CONFIG_SRCS)
aux_source_directory(protect_engines/kubernetes/rest/util KUBERNETES_UTIL_TEST)
aux_source_directory(src/protect_engines/kubernetes/rest/util KUBERNETES_UTIL_SRCS)

# -------------------------------------------- openstack --------------------------------------------
aux_source_directory(src/protect_engines/openstack/utils OPENSTACK_BASE_SRCS)
aux_source_directory(src/protect_engines/openstack/api/cinder/model OPENSTACK_API_SRCS)
aux_source_directory(src/protect_engines/openstack/api/cinder OPENSTACK_API_SRCS)
aux_source_directory(src/protect_engines/openstack/api/keystone/model OPENSTACK_API_SRCS)
aux_source_directory(src/protect_engines/openstack/api/keystone OPENSTACK_API_SRCS)
aux_source_directory(src/protect_engines/openstack/api/neutron/model OPENSTACK_API_SRCS)
aux_source_directory(src/protect_engines/openstack/api/neutron OPENSTACK_API_SRCS)
aux_source_directory(src/protect_engines/openstack/api/nova/model OPENSTACK_API_SRCS)
aux_source_directory(src/protect_engines/openstack/api/nova OPENSTACK_API_SRCS)
aux_source_directory(src/protect_engines/openstack/resource_discovery OPENSTACK_RESOURCE_ACCESS_SRCS)
aux_source_directory(src/protect_engines/openstack/consistent_snapshot OPENSTACK_CONSISTENT_SNAPSHOT_SRCS)
aux_source_directory(src/protect_engines/openstack OPENSTACK_SRCS)

aux_source_directory(protect_engines/openstack/api OPENSTACK_API_TEST)
aux_source_directory(protect_engines/openstack/api/cinder OPENSTACK_API_TEST)
aux_source_directory(protect_engines/openstack OPENSTACK_TEST)
aux_source_directory(protect_engines/openstack/consistent_snapshot OPENSTACK_CONSISTENT_SNAPSHOT_TEST)

# -------------------------------------------- hcs --------------------------------------------
aux_source_directory(protect_engines/hcs/utils HCS_UTILS_TEST)
aux_source_directory(src/protect_engines/hcs/utils HCS_UTILS_RECS)
aux_source_directory(protect_engines/hcs/api HCS_API_TEST)
aux_source_directory(src/protect_engines/hcs/api/iam HCS_API_IAM_RECS)
aux_source_directory(src/protect_engines/hcs/api/ecs HCS_API_ECS_RECS)
aux_source_directory(src/protect_engines/hcs/resource_discovery HCS_RES_DIS_RECS)
aux_source_directory(protect_engines/hcs/resource_discovery HCS_RES_DIS_TEST)

aux_source_directory(protect_engines/hcs HCS_TEST)
aux_source_directory(src/protect_engines/hcs HCS_SRCS)

aux_source_directory(protect_engines/hcs/api/evs HCS_API_EVS_TEST)
aux_source_directory(src/protect_engines/hcs/api/evs HCS_API_EVS_SRCS)
aux_source_directory(protect_engines/hcs/api/cinder HCS_API_CINDER_TEST)
aux_source_directory(src/protect_engines/hcs/api/cinder HCS_API_CINDER_SRCS)



set(PLUGIN_MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../src/PluginMain.cpp)
set(PLUGIN_MAIN ./PluginMainTest.cpp)

set(VIRT_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR})
set(VIRT_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs)

message("-------------VIRT_INC_DIR: ${VIRT_INC_DIR}")
message("-------------VIRT_LIB_DIR: ${VIRT_LIB_DIR}")
set(PROTECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(MODULE_ROOT_PATH ${PROTECT_ROOT_DIR}/Module)
set(MODULE_OPEN_SRC_PATH ${MODULE_ROOT_PATH}/third_open_src)
set(MODULE_PLATFORM_PATH ${MODULE_ROOT_PATH}/platform)
set(DT_UTILS_PATH ${MODULE_ROOT_PATH}/dt_utils)
set(GOOGLE_TEST_PATH ${DT_UTILS_PATH}/gmock/googletest-release)

if(DTFUZZ)
message(STATUS "This DTFUZZ compile.")
else()
add_subdirectory(${MODULE_ROOT_PATH}/src/system system)
endif()

set(PLUGIN_FRAMEWORK_LIBS
    ${PROTECT_ROOT_DIR}/framework/lib
    ${PROTECT_ROOT_DIR}/framework/lib/agent_sdk
    ${PROTECT_ROOT_DIR}/framework/lib/3rd
    ${MODULE_OPEN_SRC_PATH}/lz4_rel/lib
    ${MODULE_OPEN_SRC_PATH}/boost_rel/lib
    ${MODULE_OPEN_SRC_PATH}/jsoncpp_rel/libs
    ${MODULE_OPEN_SRC_PATH}/curl_rel/lib/
    ${MODULE_OPEN_SRC_PATH}/thrift_rel/lib
    ${MODULE_OPEN_SRC_PATH}/libaio_rel/lib
    ${MODULE_OPEN_SRC_PATH}/libevent_rel/lib
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/lib
    ${MODULE_OPEN_SRC_PATH}/openssl_rel/lib
    ${MODULE_OPEN_SRC_PATH}/c-ares_rel/lib
)

set(PLUGIN_FRAMEWORK_INC
    ${PROTECT_ROOT_DIR}/framework/inc
    ${PROTECT_ROOT_DIR}/framework/inc/common
    ${PROTECT_ROOT_DIR}/framework/inc/client
    ${PROTECT_ROOT_DIR}/framework/inc/rpc
    ${PROTECT_ROOT_DIR}/framework/inc/rpc/certificateservice/
    ${PROTECT_ROOT_DIR}/framework/inc/thrift_interface
    ${PROTECT_ROOT_DIR}/plugins/virtualization/test/llt_stub/

    ${MODULE_ROOT_PATH}/src
    ${MODULE_OPEN_SRC_PATH}/boost_rel/include
    ${MODULE_OPEN_SRC_PATH}/lz4_rel/include
    ${MODULE_OPEN_SRC_PATH}/jsoncpp_rel/include
    ${MODULE_OPEN_SRC_PATH}/curl_rel/include
    ${MODULE_OPEN_SRC_PATH}/openssl_rel/include
    ${MODULE_OPEN_SRC_PATH}/thrift_rel/include
    ${MODULE_OPEN_SRC_PATH}/libaio_rel/include
    ${MODULE_OPEN_SRC_PATH}/libevent_rel/include
    ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include
    ${MODULE_OPEN_SRC_PATH}/libssh2_rel/include
)

set(YAML_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/local/lib)
set(YAML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/local/include)
set(SECODE_FUZZ_INCLUDE_PATH ${DT_UTILS_PATH}/secodefuzz_rel/include)
set(SECODE_FUZZ_LIB_PATH ${DT_UTILS_PATH}/secodefuzz_rel/lib)
set(SECODE_FUZZ_LIB_NAME Secodefuzz)
message("-------------YAML_INCLUDE_DIR: ${YAML_INCLUDE_DIR}")

if(DTFUZZ)
link_directories(
    ${VIRT_LIB_DIR}
    ${PLUGIN_FRAMEWORK_LIBS}
    # dtfuzz
    ${SECODE_FUZZ_LIB_PATH}
    ${GOOGLE_TEST_PATH}/lib
)

else()
link_directories(
    ${VIRT_LIB_DIR}
    ${PLUGIN_FRAMEWORK_LIBS}
)
endif()

include_directories(
    ${VIRT_INC_DIR}
    ${PLUGIN_FRAMEWORK_INC}
    ${YAML_INCLUDE_DIR}
    ${PROTECT_ROOT_DIR}/framework/dep/agent_sdk/include/message/archivestream
    ${PROTECT_ROOT_DIR}/framework/dep/agent_sdk/include
    ${PROTECT_ROOT_DIR}/plugins/virtualization/src/volume_handlers/fusionstorage
    # gtest
    ${GOOGLE_TEST_PATH}/googletest/include
    ${GOOGLE_TEST_PATH}/googlemock/include
    ${SECODE_FUZZ_INCLUDE_PATH}
)

# 生成目标
if (DTFUZZ)
add_executable(virtualization_plugin_test
    ${PLUGIN_MAIN_SRC}
    ${PLUGIN_MAIN}
    ${native_dtfuzz_srcs}
    dt_fuzz/src/test_dtfuzz.cpp
)
else()
add_library(common_obj
    OBJECT
    ${REST_CLIENT_SRCS}
    ${TOKEN_MGR_SRCS}
    ${CERT_MGR_SRCS}
    ${HTTP_CLIENT_SRCS}
)
target_compile_options(common_obj
    PRIVATE
    -w
    -fno-access-control
    -fprofile-arcs -ftest-coverage ${FUZZ_OPT}
)

add_library(fs_handler_obj
    OBJECT
    ${FILESYSTEM_SRCS}
)
target_compile_options(fs_handler_obj
    PRIVATE
    -w
    -fno-access-control
    -fprofile-arcs -ftest-coverage ${FUZZ_OPT}
)

add_executable(virt_common_test
    # common
    ${COMMON_SRCS}
    ${COMMON_TEST}
    ${SHA256_SRCS}
    ${SHA256_TEST}
    ${TOKEN_MGR_TEST}
    ${CERT_MGR_TEST}
    ${REST_CLIENT_TEST}
    ${HTTP_CLIENT_TEST}
    ${FILESYSTEM_MOCK}
)
target_compile_options(virt_common_test
    PRIVATE
    -w
    -fno-access-control
    -fprofile-arcs -ftest-coverage ${FUZZ_OPT}
)
target_link_libraries(virt_common_test
    PRIVATE
    virtualization_plugin
    apsara_stack_engine
    hcs_engine
    cnware_engine
    fs_handler_obj
    oceanstor_obj
    volume_fusionstorage_obj
    common_obj
    securec
    basic_job
    thrift_interface
    thrift_client
    rpc
    event
    utils
    -lpluginsdk
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_regex
    boost_log
    boost_date_time
    kubernetes_engine
    curl_http_util
    lz4
    common_util
    config_reader_util
    log_util
    jsoncpp
    thrift_server
    cares
    crypto
    curl
    ssl
    system
    security_cbb
)
add_executable(virt_job_controller_test
    # job controller
    ${SCHEDULER_TEST}
    ${SCHEDULER_SRCS}
    ${JOBS_TEST}
    ${JOBS_SRCS}
    ${BACKUP_TEST}
    ${BACKUP_SRCS}
    ${COPY_VERIFY_TEST}
    ${COPY_VERIFY_SRCS}
    ${RESTORE_TEST}
    ${RESTORE_SRCS}
    ${FACTORY_TEST}
    ${FACTORY_SRCS}
    ${FILESYSTEM_MOCK}
)
target_compile_options(virt_job_controller_test
    PRIVATE
    -w
    -fno-access-control
    -fprofile-arcs -ftest-coverage ${FUZZ_OPT}
)

target_include_directories(virt_job_controller_test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/job_controller/aio_datamover/
)

target_link_libraries(virt_job_controller_test
    PRIVATE
    virtualization_plugin
    apsara_stack_engine
    hcs_engine
    cnware_engine
    fs_handler_obj
    securec
    basic_job
    thrift_interface
    thrift_client
    rpc
    event
    utils
    -lpluginsdk
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_regex
    boost_log
    boost_date_time
    kubernetes_engine
    curl_http_util
    lz4
    common_util
    config_reader_util
    log_util
    jsoncpp
    thrift_server
    cares
    crypto
    curl
    ssl
    system
    security_cbb
)

add_executable(virt_handlers_test
    # repository handler
    ${FILESYSTEM_TEST}
    # volume_handlers
    ${CLOUD_VOLUME_HANDLER_SRCS}
    ${CINDER_VOLUME_HANDLER_SRCS}
    ${CINDER_VOLUME_HANDLER_TEST}
    ${FUSIONSTORAGE_API_FACTORY_SRCS}
    ${FUSIONSTORAGE_API_FACTORY_TEST}
    ${FUSIONSTORAGE_CLIENT_SRCS}
    ${FUSIONSTORAGE_CLIENT_TEST}
    ${FUSIONSTORAGE_VOLUME_HANDLER_SRCS}
    ${FUSIONSTORAGE_VOLUME_HANDLER_TEST}
    # engine factory
    ${ENGINE_FACTORY_TEST}
    ${ENGINE_FACTORY_SRCS}
)
target_compile_options(virt_handlers_test
    PRIVATE
    -w
    -fno-access-control
    -fprofile-arcs -ftest-coverage ${FUZZ_OPT}
)
target_link_libraries(virt_handlers_test
    PRIVATE
    virtualization_plugin
    apsara_stack_engine
    hcs_engine
    openstack_engine
    cnware_engine
    fs_handler_obj
    oceanstor_obj
    volume_fusionstorage_obj
    securec
    basic_job
    thrift_interface
    thrift_client
    rpc
    event
    utils
    -lpluginsdk
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_regex
    boost_log
    boost_date_time
    kubernetes_engine
    curl_http_util
    lz4
    common_util
    config_reader_util
    log_util
    jsoncpp
    thrift_server
    cares
    crypto
    curl
    ssl
    system
    security_cbb
)

add_executable(virt_kubernetes_test
    # kubernetes
    ${KUBERNETES_TEST}
    ${KUBERNETES_SRCS}
    ${KUBERNETES_CLIENT_TEST}
    ${KUBERNETES_CLIENT_SRCS}
    ${KUBERNETES_REST_TEST}
    ${KUBERNETES_REST_SRCS}
    ${KUBERNETES_CONFIG_TEST}
    ${KUBERNETES_CONFIG_SRCS}
    ${KUBERNETES_UTIL_TEST}
    ${KUBERNETES_UTIL_SRCS}
)
target_compile_options(virt_kubernetes_test
    PRIVATE
    -w
    -fno-access-control
    -fprofile-arcs -ftest-coverage ${FUZZ_OPT}
)
target_link_libraries(virt_kubernetes_test
    PRIVATE
    virtualization_plugin
    oceanstor_obj
    securec
    basic_job
    thrift_interface
    thrift_client
    rpc
    event
    utils
    -lpluginsdk
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_regex
    boost_log
    boost_date_time
    kubernetes_engine
    curl_http_util
    lz4
    common_util
    config_reader_util
    log_util
    jsoncpp
    thrift_server
    cares
    crypto
    curl
    ssl
    system
    security_cbb
)

add_executable(virt_hcs_openstack_test
    ${REST_CLIENT_TEST}
    ${TOKEN_MGR_TEST}
    ${CERT_MGR_TEST}
    ${HTTP_CLIENT_TEST}
    ${OPENSTACK_BASE_SRCS}
    ${OPENSTACK_API_SRCS}
    ${OPENSTACK_RESOURCE_ACCESS_SRCS}
    ${OPENSTACK_CONSISTENT_SNAPSHOT_SRCS}
    ${OPENSTACK_SRCS}
    ${OPENSTACK_API_TEST}
    ${OPENSTACK_TEST}
    ${OPENSTACK_CONSISTENT_SNAPSHOT_TEST}
    ${HCS_UTILS_TEST}
    ${HCS_UTILS_RECS}
    ${HCS_API_TEST}
    ${HCS_API_IAM_RECS}
    ${HCS_API_ECS_RECS}
    ${HCS_TEST}
    ${HCS_SRCS}
    ${HCS_API_EVS_TEST}
    ${HCS_API_EVS_SRCS}
    ${HCS_API_CINDER_TEST}
    ${HCS_API_CINDER_SRCS}
    ${HCS_RES_DIS_RECS}
    ${HCS_RES_DIS_TEST}
)
target_compile_options(virt_hcs_openstack_test
    PRIVATE
    -w
    -fno-access-control
    -fprofile-arcs -ftest-coverage ${FUZZ_OPT}
)
target_link_libraries(virt_hcs_openstack_test
    PRIVATE
    virtualization_plugin
    hcs_engine
    openstack_engine
    common_obj
    oceanstor_obj
    volume_fusionstorage_obj
    securec
    basic_job
    thrift_interface
    thrift_client
    rpc
    event
    utils
    -lpluginsdk
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_regex
    boost_log
    boost_date_time
    kubernetes_engine
    curl_http_util
    lz4
    common_util
    config_reader_util
    log_util
    jsoncpp
    thrift_server
    cares
    crypto
    curl
    ssl
    system
    security_cbb
)


endif()

if(DTFUZZ)
target_link_libraries(virtualization_plugin_test
    PRIVATE
    virtualization_plugin
    apsara_stack_engine
    hcs_engine
    cnware_engine
    securec
    basic_job
    thrift_interface
    thrift_client
    rpc
    event
    utils
    -lpluginsdk
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_regex
    boost_log
    boost_date_time
    kubernetes_engine
    curl_http_util
    lz4
    common_util
    config_reader_util
    log_util
    jsoncpp
    thrift_server
    cares
    crypto
    curl
    ssl
    system
    security_cbb
    # dtfuzz
    gmock gtest gtest_main gmock_main gcov
    ${SECODE_FUZZ_LIB_NAME}
    pthread
    vir_safe_cmpexec
)
else()
endif()

# 打印构建选项
if(DTFUZZ)
    get_target_property(COMPILE_FLAGS virtualization_plugin_test COMPILE_OPTIONS)
    get_target_property(LINK_FLAGS virtualization_plugin_test LINK_OPTIONS)
    message(STATUS "Compiler id: ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "Compile flags: ${COMPILE_FLAGS}")
    message(STATUS "Link flags: ${LINK_FLAGS}")
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
else()
    get_target_property(COMPILE_FLAGS virt_common_test COMPILE_OPTIONS)
    get_target_property(LINK_FLAGS virt_common_test LINK_OPTIONS)
    message(STATUS "Compiler id: ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "Compile flags: ${COMPILE_FLAGS}")
    message(STATUS "Link flags: ${LINK_FLAGS}")
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
endif()
