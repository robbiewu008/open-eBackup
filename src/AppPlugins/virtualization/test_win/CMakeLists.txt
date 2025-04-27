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

add_definitions(-DWIN32 -D_WIN32 -DNDEBUG -D_WINDOWS -DWIN32_LEAN_AND_MEAN -D_USRDLL -D__WINDOWS__ -D_CRT_NO_VA_START_VALIDATION)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:LIBCMT /NODEFAULTLIB:libcrpt.lib /NODEFAULTLIB:msvcprt.lib /NODEFAULTLIB:libcpmt.lib")

set(VIRT_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR})
set(PROTECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(MODULE_ROOT_PATH ${PROTECT_ROOT_DIR}/Module)
set(FRAMEWORK_ROOT_PATH ${PROTECT_ROOT_DIR}/common/framework)
set(MODULE_OPEN_SRC_PATH ${MODULE_ROOT_PATH}/open_src)
set(MODULE_PLATFORM_PATH ${MODULE_ROOT_PATH}/platform)
set(DT_UTILS_PATH ${MODULE_ROOT_PATH}/dt_utils)
set(GOOGLE_TEST_PATH ${DT_UTILS_PATH}/gmock/googletest-release)
set(HDT_TEST OFF)


if(DTFUZZ)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../build/cmake/global.cmake)
elseif(HDT_TEST)
    # 包含所有组件的cmake
    include("${CMAKE_CURRENT_SOURCE_DIR}/conf/comp.cmake")
endif()

# LLT打桩私有函数公有化
add_definitions("-fno-access-control --coverage")

# 添加源代码
# 添加测试代码
# -------------------------------------------- common --------------------------------------------
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/common COMMON_SRCS)
aux_source_directory(common COMMON_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/common/sha256 SHA256_SRCS)
aux_source_directory(common/sha256 SHA256_TEST)

aux_source_directory(common/token_mgr TOKEN_MGR_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/common/token_mgr TOKEN_MGR_SRCS)

aux_source_directory(common/cert_mgr CERT_MGR_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/common/cert_mgr CERT_MGR_SRCS)

aux_source_directory(common/httpclient HTTP_CLIENT_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/common/httpclient HTTP_CLIENT_SRCS)

aux_source_directory(common/client REST_CLIENT_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/common/client REST_CLIENT_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/common/utils COMMON_UTILS_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/common/model COMMON_MODEL_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/common/uuid COMMON_UUID_SRCS)

## -------------------------------------------- job_controller --------------------------------------------
aux_source_directory(job_controller/io_scheduler SCHEDULER_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/job_controller/io_scheduler SCHEDULER_SRCS)

aux_source_directory(job_controller/jobs JOBS_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/job_controller/jobs JOBS_SRCS)
aux_source_directory(job_controller/jobs/backup BACKUP_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/job_controller/jobs/backup BACKUP_SRCS)
aux_source_directory(job_controller/jobs/verify COPY_VERIFY_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/job_controller/jobs/verify COPY_VERIFY_SRCS)
aux_source_directory(job_controller/jobs/restore RESTORE_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/job_controller/jobs/restore RESTORE_SRCS)
aux_source_directory(job_controller/factory FACTORY_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/job_controller/factory FACTORY_SRCS)

## -------------------------------------------- repository_handlers --------------------------------------------
#aux_source_directory(repository_handlers/win32filesystem WIN32_FILESYSTEM_TEST)
#aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/repository_handlers/win32filesystem WIN32_FILESYSTEM_SRCS)

# -------------------------------------------- volume_handlers --------------------------------------------
aux_source_directory(volume_handlers/hyperv_volume HYPERV_VOLUME_HANDLER_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/volume_handlers/hyperv_volume HYPERV_VOLUME_HANDLER_SRCS)

# -------------------------------------------- engines_factory --------------------------------------------
aux_source_directory(protect_engines/engine_factory ENGINE_FACTORY_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/protect_engines/engine_factory ENGINE_FACTORY_SRCS)

# -------------------------------------------- hyperv --------------------------------------------
aux_source_directory(protect_engines/hyperv HYPERV_TEST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/protect_engines/hyperv HYPERV_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/protect_engines/hyperv/resource_discovery HYPERV_RESOURCE_DISCOVERY_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/protect_engines/hyperv/utils HYPERV_UTILS_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/protect_engines/hyperv/utils/executor HYPERV_EXECUTOR_SRCS)
#aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/protect_engines/hyperv/utils/executor HYPERV_EXECUTOR_SRCS)

message("-------------VIRT_INC_DIR: ${VIRT_INC_DIR}")


set(PLUGIN_FRAMEWORK_INC
    ${PROTECT_ROOT_DIR}/common/framework/inc
    ${PROTECT_ROOT_DIR}/common/framework/inc/common
    ${PROTECT_ROOT_DIR}/common/framework/inc/client
    ${PROTECT_ROOT_DIR}/common/framework/inc/server
    ${PROTECT_ROOT_DIR}/common/framework/inc/rpc
    ${PROTECT_ROOT_DIR}/common/framework/inc/rpc/certificateservice/
    ${PROTECT_ROOT_DIR}/common/framework/inc/thrift_interface
    ${PROTECT_ROOT_DIR}/common/framework/dep/agent_sdk/include/message/archivestream
    ${PROTECT_ROOT_DIR}/common/framework/dep/agent_sdk/include
)
set(PLUGIN_MODULE_INC
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/dt_utils/gmock/googletest-release/googletest/include
    ${MODULE_ROOT_PATH}/dt_utils/gmock/googletest-release/googlemock/include
    ${MODULE_PLATFORM_PATH}/securec/include
    ${MODULE_OPEN_SRC_PATH}/thrift/lib/cpp/include
    ${MODULE_OPEN_SRC_PATH}/jsoncpp/include
    ${MODULE_OPEN_SRC_PATH}/tinyxml2/include
    ${MODULE_OPEN_SRC_PATH}/openssl/include
    ${MODULE_OPEN_SRC_PATH}/boost/.libs/include
)

set(YAML_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/local/lib)
set(YAML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/local/include)
set(SECODE_FUZZ_INCLUDE_PATH ${DT_UTILS_PATH}/secodefuzz_rel/include)
set(SECODE_FUZZ_LIB_PATH ${DT_UTILS_PATH}/secodefuzz_rel/lib)
set(SECODE_FUZZ_LIB_NAME Secodefuzz)
message("-------------YAML_INCLUDE_DIR: ${YAML_INCLUDE_DIR}")


link_directories(
    ${FRAMEWORK_ROOT_PATH}/lib
)

include_directories(
    ${VIRT_INC_DIR}
    ${PLUGIN_FRAMEWORK_INC}
    ${PLUGIN_MODULE_INC}
    #${PROTECT_ROOT_DIR}/plugins/virtualization/test/llt_stub/
)
link_libraries(
    log_util.lib
    system.lib
    metafile_parser.lib
    config_reader_util.lib
    common_util.lib
    basic_job.lib
    rpc.lib
    thrift_client.lib
    thrift_server.lib
    securec.lib
    libssl.lib
    libcrypto.lib
    agentsdk.lib
    thrift_interface.lib
    servicefactory_obj.lib
    gtest.lib
    gmock.lib
    gtest_main.lib
    gmock_main.lib
)
add_compile_options(
    -w
    -fno-access-control
    -fprofile-arcs -ftest-coverage ${FUZZ_OPT}
)
# 生成目标

#add_executable(virt_common_test
#    # common
#    ${COMMON_SRCS}
#    ${COMMON_TEST}
#    ${SHA256_SRCS}
#    ${SHA256_TEST}
#    ${CERT_MGR_TEST}
#    ${CERT_MGR_SRCS}
#)
#
#add_executable(virt_job_controller_test
#    # job controller
#    ${SCHEDULER_TEST}
#    ${SCHEDULER_SRCS}
#    ${JOBS_TEST}
#    ${JOBS_SRCS}
#    ${BACKUP_TEST}
#    ${BACKUP_SRCS}
#    ${COPY_VERIFY_TEST}
#    ${COPY_VERIFY_SRCS}
#    ${RESTORE_TEST}
#    ${RESTORE_SRCS}
#    ${FACTORY_TEST}
#    ${FACTORY_SRCS}
#    ${WIN32_FILESYSTEM_SRCS}
#    ${WIN32_FILESYSTEM_MOCK}
#)
#
#add_executable(virt_handlers_test
#    # repository handler
#    ${WIN32_FILESYSTEM_SRCS}
#    ${WIN32_FILESYSTEM_TEST}
#    # volume_handlers
#    ${HYPERV_VOLUME_HANDLER_SRCS}
#    ${HYPERV_VOLUME_HANDLER_TEST}
#    # engine factory
#    ${ENGINE_FACTORY_TEST}
#    ${ENGINE_FACTORY_SRCS}
#)
add_executable(virt_hyperv_test
    # hyperv
    ${HYPERV_TEST}
    ${HYPERV_SRCS}
    ${HYPERV_RESOURCE_DISCOVERY_SRCS}
    ${BACKUP_SRCS}
    ${JOBS_SRCS}
    ${RESTORE_SRCS}
    ${SCHEDULER_SRCS}
    ${CERT_MGR_SRCS}
    ${COMMON_UTILS_SRCS}
    ${COMMON_MODEL_SRCS}
    ${COMMON_UUID_SRCS}
    ${FACTORY_SRCS}
    ${ENGINE_FACTORY_SRCS}
    ${COMMON_SRCS}
    ${SHA256_SRCS}
    ${TOKEN_MGR_SRCS}
    #${HYPERV_VOLUME_HANDLER_SRCS}
#    ${HYPERV_UTILS_SRCS}
)

#get_target_property(COMPILE_FLAGS virt_common_test COMPILE_OPTIONS)
#get_target_property(LINK_FLAGS virt_common_test LINK_OPTIONS)
message(STATUS "Compiler id: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compile flags: ${COMPILE_FLAGS}")
message(STATUS "Link flags: ${LINK_FLAGS}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
