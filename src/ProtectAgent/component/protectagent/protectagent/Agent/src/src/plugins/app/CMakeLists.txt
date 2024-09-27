aux_source_directory(. SRC_LIST)
add_library(pluginsAppobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(pluginsAppobj
    PRIVATE
    safe_cmplib
    agent_platform
)