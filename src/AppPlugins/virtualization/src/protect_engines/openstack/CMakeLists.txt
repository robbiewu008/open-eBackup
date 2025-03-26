file(GLOB_RECURSE openstack_srcs *.cpp *.h)
add_library(openstack_engine
        SHARED
        ${openstack_srcs})

target_link_libraries(openstack_engine
    PUBLIC
    vir_safe_cmplib
)