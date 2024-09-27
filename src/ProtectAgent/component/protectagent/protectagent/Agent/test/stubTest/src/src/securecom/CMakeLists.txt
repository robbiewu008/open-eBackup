aux_source_directory(. SRC_LIST)
add_library(testSecurecomobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testSecurecomobj
    PRIVATE
    safe_cmplib
    agent_platform
)