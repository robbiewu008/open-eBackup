MESSAGE("Compiling thrift server code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)

add_library(rpc_server
    SHARED
    PluginThriftServer.cpp
)

target_include_directories(rpc_server
    PRIVATE
    ${INC_PATH}/rpc
    ${INC_PATH}/rpc_server
    ${INC_PATH}/rpc/certificateservice
    ${INC_PATH}/rpc/thriftservice
    ${INC_PATH}/rpc/servicefactory
    ${INC_PATH}/server
    ${INC_PATH}/utils
    ${MODULE_PATH}
    ${MODULE_PATH}/src
)

target_link_directories(rpc_server
    PRIVATE
    ${AGENT_SDK_PATH}/lib
)

target_link_libraries(rpc_server
    PRIVATE
    pthread
    job_mgr_obj
    servicefactory_obj
    certificate_service_obj
    thriftservice_obj
    thrift_server
    pluginsdk
    utils
)

if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(rpc
        PRIVATE
        utils
    )
endif()