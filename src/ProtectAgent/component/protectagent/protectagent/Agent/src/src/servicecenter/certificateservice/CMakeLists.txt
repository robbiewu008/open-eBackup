aux_source_directory(. SRC_LIST)
add_library(certificateserviceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(certificateserviceobj
    PRIVATE
    safe_cmplib
    agent_platform
)