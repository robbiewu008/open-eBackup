file(GLOB_RECURSE cnware_srcs *.cpp *.h)
add_library(cnware_engine
        SHARED
        ${cnware_srcs})

target_link_libraries(cnware_engine
    PUBLIC
    vir_safe_cmplib
)