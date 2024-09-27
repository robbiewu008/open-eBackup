
aux_source_directory(. VIRTUALIZATION_JOB_FACTORY_SRCS)

add_library(virtualization_job_factory_obj
    OBJECT
    ${VIRTUALIZATION_JOB_FACTORY_SRCS}
)

target_include_directories(virtualization_job_factory_obj
    PRIVATE
    ${PROTECT_ROOT_DIR}/framework/dep/agent_sdk/include/message/archivestream
    ${PROTECT_ROOT_DIR}/framework/dep/agent_sdk/include
)

target_link_libraries(virtualization_job_factory_obj
        PRIVATE
        vir_safe_cmplib
        )