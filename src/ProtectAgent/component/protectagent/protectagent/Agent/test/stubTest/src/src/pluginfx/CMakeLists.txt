aux_source_directory(. SRC_LIST)
add_library(testPluginfxobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testPluginfxobj
    PRIVATE
    safe_cmplib
    agent_platform
)