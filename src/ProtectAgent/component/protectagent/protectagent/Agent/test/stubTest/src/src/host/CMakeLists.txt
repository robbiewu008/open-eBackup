aux_source_directory(. SRC_LIST)
add_library(testHostobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testHostobj
    PRIVATE
    safe_cmplib
    agent_platform
)