aux_source_directory(. SRC_LIST)
add_library(testMessageServiceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testMessageServiceobj
    PRIVATE
    safe_cmplib
    agent_platform
)