aux_source_directory(. SRC_LIST)
add_library(datamessageobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(datamessageobj
    PRIVATE
    safe_cmplib
    agent_platform
)