aux_source_directory(. SRC_LIST)
add_library(servicesDeviceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(servicesDeviceobj
    PRIVATE
    safe_cmplib
    agent_platform
)