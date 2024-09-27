aux_source_directory(. SRC_LIST)
add_library(servicefactoryobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(servicefactoryobj
    PRIVATE
    safe_cmplib
    agent_platform
)