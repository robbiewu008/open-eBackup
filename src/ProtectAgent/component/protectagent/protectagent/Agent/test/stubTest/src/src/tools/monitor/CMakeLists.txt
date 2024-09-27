aux_source_directory(. SRC_LIST)
add_library(testToolsMonitorobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testToolsMonitorobj
    PRIVATE
    safe_cmplib
    agent_platform
)