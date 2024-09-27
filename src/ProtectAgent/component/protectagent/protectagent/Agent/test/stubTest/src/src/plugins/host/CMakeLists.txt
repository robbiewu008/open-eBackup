aux_source_directory(. SRC_LIST)
add_library(testPluginsHostobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testPluginsHostobj
    PRIVATE
    safe_cmplib
    agent_platform
)