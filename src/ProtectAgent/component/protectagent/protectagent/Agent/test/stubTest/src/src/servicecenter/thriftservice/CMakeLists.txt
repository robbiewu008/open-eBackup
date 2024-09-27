aux_source_directory(. SRC_LIST)
add_library(testThriftServiceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testThriftServiceobj
    PRIVATE
    safe_cmplib
    agent_platform
)