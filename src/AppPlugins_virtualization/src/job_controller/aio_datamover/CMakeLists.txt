aux_source_directory(. VIRTUAL_IO_SCHEDULER)

add_library(aio_datamoverobj
    OBJECT
    ${VIRTUAL_IO_SCHEDULER}
)

target_include_directories(aio_datamoverobj
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Module/third_open_src/libaio_rel/include/
)

target_link_libraries(aio_datamoverobj
    PRIVATE
    securec
    vir_safe_cmplib
)