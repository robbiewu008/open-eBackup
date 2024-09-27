aux_source_directory(. SRC_LIST)
add_library(thriftServiceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(thriftServiceobj
    PRIVATE
    safe_cmplib
    agent_platform
)