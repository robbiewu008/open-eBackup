aux_source_directory(. SRC_LIST)
add_library(getinputobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(getinputobj
    PRIVATE
    safe_cmplib
    agent_platform
)