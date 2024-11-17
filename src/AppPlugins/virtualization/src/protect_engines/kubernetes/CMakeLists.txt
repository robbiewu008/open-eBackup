file(GLOB_RECURSE kubernetes_srcs *.cpp *.hpp *.h)
add_library(kubernetes_engine
        SHARED
        ${kubernetes_srcs})

target_compile_features(kubernetes_engine PRIVATE cxx_std_17)

set(YAML_LIB_DIR ${PROJECT_SOURCE_DIR}/deps/local/lib)
set(YAML_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/deps/local/include)

target_include_directories(kubernetes_engine PRIVATE ${YAML_INCLUDE_DIR})
target_link_libraries(kubernetes_engine PRIVATE ${YAML_LIB_DIR}/libyaml-cpp.a vir_safe_cmplib)

option(KUBERNETES_TEST "Build the tests" OFF)
if(KUBERNETES_TEST)
    enable_testing()
    set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)
    set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
    file(GLOB KUBERNETES_TEST_SRCS
            ${TEST_DIR}/protect_engines/kubernetes/*.cpp
            ${TEST_DIR}/protect_engines/kubernetes/rest/*.cpp
            ${TEST_DIR}/protect_engines/kubernetes/rest/config/*.cpp
            ${TEST_DIR}/protect_engines/kubernetes/rest/client/*.cpp
            ${TEST_DIR}/protect_engines/kubernetes/rest/util/*.cpp
            ${SRC_DIR}/protect_engines/kubernetes/*.cpp
            ${SRC_DIR}/protect_engines/kubernetes/rest/*.cpp
            ${SRC_DIR}/protect_engines/kubernetes/rest/config/*.cpp
            ${SRC_DIR}/protect_engines/kubernetes/rest/client/*.cpp
            ${SRC_DIR}/protect_engines/kubernetes/util/*.cpp)
    add_executable(kubernetes_engine_test
            ${KUBERNETES_TEST_SRCS})
    include("${TEST_DIR}/conf/comp.cmake")
    set(K8S_FRAMEWORK_LIBS
            ${PROTECT_ROOT_DIR}/framework/lib
            ${PROTECT_ROOT_DIR}/framework/lib/agent_sdk
            ${MODULE_OPEN_SRC_PATH}/lz4_rel/lib
            ${MODULE_OPEN_SRC_PATH}/boost_rel/lib
            ${MODULE_OPEN_SRC_PATH}/jsoncpp_rel/libs
            ${MODULE_OPEN_SRC_PATH}/openssl_rel/lib
            ${MODULE_OPEN_SRC_PATH}/curl_rel/lib/
            ${MODULE_OPEN_SRC_PATH}/thrift_rel/lib
            ${MODULE_OPEN_SRC_PATH}/libaio_rel/lib
            ${MODULE_OPEN_SRC_PATH}/libevent_rel/lib
            ${MODULE_PLATFORM_PATH}/SecureCLib_rel/lib
            ${MODULE_OPEN_SRC_PATH}/c-ares_rel/lib
            ${YAML_LIB_DIR}
            )

    set(K8S_FRAMEWORK_INC
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
            ${MODULE_OPEN_SRC_PATH}/openssl_rel/include
            ${MODULE_OPEN_SRC_PATH}/curl_rel/include
            ${MODULE_OPEN_SRC_PATH}/thrift_rel/include
            ${MODULE_OPEN_SRC_PATH}/libaio_rel/include
            ${MODULE_OPEN_SRC_PATH}/libevent_rel/include
            ${MODULE_PLATFORM_PATH}/SecureCLib_rel/include

            ${YAML_INCLUDE_DIR}
            )

    message(STATUS "K8S_FRAMEWORK_LIBS: ${K8S_FRAMEWORK_LIBS}")
    message(STATUS "K8S_FRAMEWORK_INC: ${K8S_FRAMEWORK_INC}")
    message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
    target_include_directories(kubernetes_engine_test
            PRIVATE
            ${TEST_DIR}
            ${K8S_FRAMEWORK_INC}
            )
    target_link_directories(kubernetes_engine_test
            PUBLIC
            ${K8S_FRAMEWORK_LIBS}
            ${COMP_DIR}/gtest/lib
            )
    target_compile_features(kubernetes_engine_test PRIVATE
            cxx_std_17
            )
    target_compile_options(kubernetes_engine_test PUBLIC
            --coverage
            -O0
            -fno-access-control)
    target_link_options(kubernetes_engine_test PUBLIC
            --coverage)
    target_link_libraries(
            kubernetes_engine_test
            PRIVATE
            virtualization_plugin
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
            ssl
            curl
            system
            libyaml-cpp.a
            -lpthread
            libgmock.a
            libgtest.a
            libgtest_main.a
    )
endif()