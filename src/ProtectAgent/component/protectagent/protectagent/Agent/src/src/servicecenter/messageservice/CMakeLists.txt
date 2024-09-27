aux_source_directory(. SRC_LIST)
add_library(messageserviceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(messageserviceobj
    PRIVATE
    safe_cmplib
    agent_platform
)