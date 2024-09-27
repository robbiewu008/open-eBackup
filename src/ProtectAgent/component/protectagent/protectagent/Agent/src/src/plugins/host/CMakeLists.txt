aux_source_directory(. SRC_LIST)
add_library(pluginsHostobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(pluginsHostobj
    PRIVATE
    safe_cmplib
    agent_platform
)