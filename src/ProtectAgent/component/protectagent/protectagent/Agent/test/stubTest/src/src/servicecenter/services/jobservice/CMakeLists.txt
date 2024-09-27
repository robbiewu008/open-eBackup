aux_source_directory(. SRC_LIST)
add_library(testServicesJobServiceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testServicesJobServiceobj
    PRIVATE
    safe_cmplib
    agent_platform
)