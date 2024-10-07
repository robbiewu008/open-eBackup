aux_source_directory(. VIRT_BASICJOB_SRCS)
aux_source_directory(backup BACKUP_SRCS)
aux_source_directory(restore RESTORE_SRCS)
aux_source_directory(verify VERIFY_SRCS)
aux_source_directory(livemount LIVEMOUNT_SRCS)

add_library(jobs_obj
    OBJECT
    ${VIRT_BASICJOB_SRCS}
    ${BACKUP_SRCS}
    ${RESTORE_SRCS}
    ${VERIFY_SRCS}
    ${LIVEMOUNT_SRCS}
)

target_include_directories(jobs_obj
    PRIVATE
    ${PROTECT_ROOT_DIR}/framework/dep/agent_sdk/include/message/archivestream
    ${PROTECT_ROOT_DIR}/framework/dep/agent_sdk/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../job_controller/aio_datamover/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    ${PROTECT_ROOT_DIR}/Module/src/common/
)

target_link_libraries(jobs_obj
        PRIVATE
        vir_safe_cmplib
        )
