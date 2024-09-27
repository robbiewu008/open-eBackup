aux_source_directory(. SRC_LIST)
add_library(pluginsOracleobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(pluginsOracleobj
    PRIVATE
    safe_cmplib
    agent_platform
)