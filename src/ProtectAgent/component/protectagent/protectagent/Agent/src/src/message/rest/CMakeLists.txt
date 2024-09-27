aux_source_directory(. SRC_LIST)
add_library(restobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(restobj
    PRIVATE
    safe_cmplib
    agent_platform
)