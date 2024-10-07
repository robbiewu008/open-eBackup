MESSAGE("Compiling security module, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/path.cmake)
SET(MODULE_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(OPEN_SRC_PATH ${MODULE_SRC_PATH}/../third_open_src)
SET(PLATFORM ${MODULE_SRC_PATH}/../platform)

add_compile_options("-fPIC")

SET(SECURITY_MODULE_INC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MODULE_SRC_PATH}/
    ${MODULE_SRC_PATH}/log
    ${PLATFORM}/SecureCLib_rel/include
)

add_subdirectory(cmd)

add_library(security_cbb
    SHARED
)

target_link_directories(security_cbb
    PUBLIC
    ${PLATFORM}/SecureCLib_rel/lib
)

target_link_libraries(security_cbb
    PUBLIC
    safe_cmplib
    sec_cmd_obj
    securec
    common_util
    log_util
)
