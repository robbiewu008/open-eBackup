MESSAGE("Compiling security cbb code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE security_test_srcs *.cpp)

add_library(security_test_obj
    OBJECT
    ${security_test_srcs}
)

target_include_directories(security_test_obj
    PUBLIC
    ${MODULE_ROOT_PATH}/src
    ${DT_UTILS_PATH}/llt_stub
    ${PLATFORM_PATH}/SecureCLib_rel/include
    ${OPEN_SRC_PATH}/boost_rel/include
)

target_link_directories(security_test_obj
    PUBLIC
    ${PLATFORM_PATH}/SecureCLib_rel/lib
)

target_link_libraries(security_test_obj
    PUBLIC
    securec
    log_util
    common_util
    log_util
)
