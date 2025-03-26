file(GLOB_RECURSE nutanix_srcs *.cpp *.h)
add_library(nutanix_engine
        SHARED
        ${nutanix_srcs})

target_link_libraries(nutanix_engine
    PUBLIC
    vir_safe_cmplib
)