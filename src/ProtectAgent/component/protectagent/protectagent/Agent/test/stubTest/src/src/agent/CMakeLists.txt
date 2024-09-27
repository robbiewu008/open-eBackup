aux_source_directory(. SRC_LIST)
add_library(testAgentobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testAgentobj
    PRIVATE
    safe_cmplib
    agent_platform
)