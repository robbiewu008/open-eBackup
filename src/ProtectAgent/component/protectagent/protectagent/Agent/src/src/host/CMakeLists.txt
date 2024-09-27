aux_source_directory(. SRC_LIST)
add_library(hostobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(hostobj
    PRIVATE
    safe_cmplib
    agent_platform
)