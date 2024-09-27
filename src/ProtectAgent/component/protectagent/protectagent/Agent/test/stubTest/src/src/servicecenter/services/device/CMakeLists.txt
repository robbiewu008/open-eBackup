aux_source_directory(. SRC_LIST)
add_library(testServicesDeviceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testServicesDeviceobj
    PRIVATE
    safe_cmplib
    agent_platform
)