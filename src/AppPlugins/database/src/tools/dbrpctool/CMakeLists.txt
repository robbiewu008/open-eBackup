MESSAGE("Compiling rpc tool code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

if (NOT DEFINED PRCCALL_NAME)
    SET(PRCCALL_NAME dbrpctool)
endif()

# 递归添加目录下所有的cpp文件
file(GLOB_RECURSE dprpctool_obj_srcs *.cpp)

add_executable(${PRCCALL_NAME}
    ${dprpctool_obj_srcs}
)

target_include_directories(${PRCCALL_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../executor
    ${FRAMEWORK_INC_PATH}/common
    ${FRAMEWORK_INC_PATH}/thrift_interface
    ${FRAMEWORK_INC_PATH}/rpc
    ${FRAMEWORK_INC_PATH}/rpc/thriftservice
    ${FRAMEWORK_INC_PATH}/rpc/certificateservice
    ${FRAMEWORK_INC_PATH}/utils
    ${FRAMEWORK_INC_PATH}
    ${MODULE_OPENSRC_PATH}/fcgi_rel/include
    ${MODULE_OPENSRC_PATH}/thrift_rel/include
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/include
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${MODULE_BASE_PATH}/src
    ${MODULE_BASE_PATH}/src/curl_http
    ${MODULE_BASE_PATH}/common
    ${MODULE_SECUREC_INC_PATH}
    ${FRAMEWORK_THRIFT_INC_PATH}
    ${AGENT_SDK_PATH}/include
)

if(INTERNAL_PLUGIN)
    set(internal_link_lib
        rpc
        curl_http_util
        thrift_client
    )
    add_definitions(-DINTERNAL_PLUGIN)
endif()

target_link_directories(${PRCCALL_NAME}
    PRIVATE
    ${FRAMEWORK_LIB_PATH}
    ${FRAMEWORK_LIB_PATH}/agent_sdk
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/libs
)

if(AIX_COMPILE)
    target_link_libraries(${PRCCALL_NAME}
        PRIVATE
        safe_cmplib
        basic_job
        log_util
        common_util
        config_reader_util
        json
        rpc
        thrift_interface
        thrift_client
        pluginsdk
        database-1.0.0
    )
else()
    target_link_directories(${PRCCALL_NAME}
        PRIVATE
        ${MODULE_BASE_PATH}/platform/KMCv3_infra_rel/lib
        ${MODULE_BASE_PATH}/platform/SecureCLib_rel/lib
        ${MODULE_OPENSRC_PATH}/openssl_rel/lib
        ${MODULE_OPENSRC_PATH}/libssh2_rel/lib
        ${MODULE_OPENSRC_PATH}/curl_rel/lib
        ${MODULE_OPENSRC_PATH}/libevent_rel/lib
        ${MODULE_OPENSRC_PATH}/c-ares_rel/lib
        ${MODULE_OPENSRC_PATH}/boost_rel/lib
        ${MODULE_OPENSRC_PATH}/icu_rel/libs/lib
        ${FRAMEWORK_LIB_PATH}
    )
    target_link_libraries(${PRCCALL_NAME}
        PRIVATE
        safe_cmplib
        rpc
        basic_job
        log_util
        common_util
        config_reader_util
        kmcv3
        cares
        ssh2
        pluginsdk
        ${internal_link_lib}
        thrift_server
        thrift_client
        thrift_interface
        utils
        curl
        ssl
        crypto
        event
        pthread
        boost_chrono
        boost_regex
        boost_filesystem
        boost_atomic
        boost_system
        boost_thread
        boost_log
        boost_date_time
        icudata
        icuuc
        icui18n
        securec
        jsoncpp
        param_checker
        database-1.0.0
    )
endif()