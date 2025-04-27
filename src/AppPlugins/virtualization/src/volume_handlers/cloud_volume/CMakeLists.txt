aux_source_directory(. CLOUD_VOLUME_SRCS)
aux_source_directory(cinder_volume CINDER_VOLUME_SRCS)
aux_source_directory(cnware_volume CNWARE_VOLUME_SRCS)
aux_source_directory(nutanix_volume NUTANIX_VOLUME_SRCS)

add_library(cloud_volume_obj
        OBJECT
        ${CLOUD_VOLUME_SRCS}
        ${CINDER_VOLUME_SRCS}
        ${CNWARE_VOLUME_SRCS}
        ${NUTANIX_VOLUME_SRCS}
        )

target_link_libraries(cloud_volume_obj
    PRIVATE
    vir_safe_cmplib

    PUBLIC
    apsara_volume_handler
)

target_link_directories(cloud_volume_obj
    PUBLIC
    ${PROTECT_ROOT_DIR}/libs
)