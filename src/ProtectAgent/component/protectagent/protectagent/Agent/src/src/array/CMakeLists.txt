aux_source_directory(. SRC_LIST)
add_library(arrayobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(arrayobj
    PRIVATE
    safe_cmplib
    agent_platform
)