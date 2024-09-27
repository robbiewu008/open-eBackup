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
    ${MODULE_BASE_PATH}/src
    ${MODULE_BASE_PATH}/src/curl_http
    ${MODULE_BASE_PATH}/common
    ${MODULE_SECUREC_INC_PATH}
    ${FRAMEWORK_THRIFT_INC_PATH}
)

if(INTERNAL_PLUGIN)
    set(internal_link_lib
        rpc
        curl_http_util
        thrift_client
    )
    add_definitions(-DINTERNAL_PLUGIN)
endif()

if(AIX_COMPILE)
    target_link_libraries(${PRCCALL_NAME}
        PRIVATE
        database_inc
        database-1.0.0
    )
else()
    target_link_libraries(${PRCCALL_NAME}
        PRIVATE
        kmcv3
        cares
        ssh2
        pluginsdk
        ${internal_link_lib}
        thrift_server
        config_reader_util
        utils
        curl
        crypto
        database_inc
        event
        pthread
        param_checker
        database-1.0.0
    )
endif()