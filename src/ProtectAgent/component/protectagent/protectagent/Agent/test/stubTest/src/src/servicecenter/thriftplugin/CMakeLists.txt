aux_source_directory(. SRC_LIST)
add_library(testThriftPluginobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testThriftPluginobj
    PRIVATE
    safe_cmplib
    agent_platform
)