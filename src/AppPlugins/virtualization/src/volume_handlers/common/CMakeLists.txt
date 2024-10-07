aux_source_directory(. volume_common_srcs)
add_library(volume_common_obj
    OBJECT
    ${volume_common_srcs})

target_link_libraries(volume_common_obj
        PRIVATE
        vir_safe_cmplib
        )
