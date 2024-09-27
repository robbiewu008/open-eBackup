aux_source_directory(. SRC_LIST)
add_library(cryptoobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(cryptoobj
    PRIVATE
    safe_cmplib
    agent_platform
)