aux_source_directory(. SRC_LIST)
add_library(appobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(appobj
    PRIVATE
    safe_cmplib
    agent_platform
)