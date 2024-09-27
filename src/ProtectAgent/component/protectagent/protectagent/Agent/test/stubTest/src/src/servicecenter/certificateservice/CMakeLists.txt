aux_source_directory(. SRC_LIST)
add_library(testCertificateServiceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testCertificateServiceobj
    PRIVATE
    safe_cmplib
    agent_platform
)