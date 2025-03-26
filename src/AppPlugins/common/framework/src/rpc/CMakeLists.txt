MESSAGE("Compiling thrift server code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)
add_subdirectory(certificateservice)
add_subdirectory(servicefactory)
add_subdirectory(thriftservice)

add_library(rpc
    SHARED
    PluginThriftClient.cpp
    PluginThriftClientConfig.cpp
)

target_include_directories(rpc
    PRIVATE
    ${INC_PATH}/rpc
    ${INC_PATH}/rpc/certificateservice
    ${INC_PATH}/rpc/thriftservice
    ${INC_PATH}/rpc/servicefactory
    ${INC_PATH}/utils
    ${MODULE_PATH}
    ${MODULE_PATH}/src
)

target_link_directories(rpc
    PRIVATE
    ${AGENT_SDK_PATH}/lib
)

target_link_libraries(rpc
    PRIVATE
    pthread
    servicefactory_obj
    certificate_service_obj
    thriftservice_obj
    pluginsdk
    utils
)

if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(rpc
        PRIVATE
        utils
    )
endif()