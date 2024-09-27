aux_source_directory(. SRC_LIST)
add_library(servicesJobserviceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(servicesJobserviceobj
    PRIVATE
    safe_cmplib
    agent_platform
)