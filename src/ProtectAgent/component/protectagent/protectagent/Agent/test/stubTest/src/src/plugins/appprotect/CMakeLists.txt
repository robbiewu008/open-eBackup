aux_source_directory(. SRC_LIST)
add_library(testPluginsAppprotectobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testPluginsAppprotectobj
    PRIVATE
    safe_cmplib
    agent_platform
)