aux_source_directory(. SRC_LIST)
add_library(testDeviceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testDeviceobj
    PRIVATE
    safe_cmplib
    agent_platform
)