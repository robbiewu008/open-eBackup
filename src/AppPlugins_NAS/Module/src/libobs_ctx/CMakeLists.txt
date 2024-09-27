MESSAGE("Compiling libobs code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
SET(MODULE_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(OPEN_SRC_PATH ${MODULE_SRC_PATH}/../third_open_src)
SET(PLATFORM ${MODULE_SRC_PATH}/../platform)

add_compile_options("-fPIC")

SET(STORAGE_COMMON_INC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MODULE_SRC_PATH}/
    ${MODULE_SRC_PATH}/log
    ${PLATFORM}/SecureCLib_rel/include
)

add_subdirectory(parser)
add_subdirectory(storage)
add_subdirectory(common)

add_library(obs_ctx
    SHARED
)

target_link_directories(obs_ctx
    PUBLIC
    ${PLATFORM}/SecureCLib_rel/lib
)

target_link_libraries(obs_ctx
    PUBLIC
    safe_cmplib
    securec
    log_util
    obs_common_obj
    obs_parser_obj
    hcs_obj
    alicloud_obj
)
