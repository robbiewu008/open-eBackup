aux_source_directory(. VIRTUAL_IO_SCHEDULER)

add_library(io_schedulerobj
    OBJECT
    ${VIRTUAL_IO_SCHEDULER}
)

target_include_directories(io_schedulerobj
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/
)

target_link_libraries(io_schedulerobj
    PRIVATE
    securec
    vir_safe_cmplib
)