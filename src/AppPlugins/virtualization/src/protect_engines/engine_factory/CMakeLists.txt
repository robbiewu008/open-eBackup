
aux_source_directory(. ENGINE_FACTORY_SRCS)

add_library(engine_factory_obj
    OBJECT
    ${ENGINE_FACTORY_SRCS}
)

target_link_libraries(engine_factory_obj
    PRIVATE
    vir_safe_cmplib
)
