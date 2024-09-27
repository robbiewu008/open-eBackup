aux_source_directory(. SRC_LIST)
add_library(dataconfigobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(dataconfigobj
    PRIVATE
    safe_cmplib
    agent_platform
)