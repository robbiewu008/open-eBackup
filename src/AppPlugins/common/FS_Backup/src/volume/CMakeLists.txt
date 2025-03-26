file(GLOB_RECURSE VOLUMEPROTECT_SOURCES *.cpp)
 
set(BACKUP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(MODULE_OPEN_SRC_PATH "${MODULE_ROOT_PATH}/third_open_src")

include_directories(
    "."
    "common"
    "task"
    "native"
)

add_library(VOLUMEPROTECT_OBJ
    OBJECT
    ${VOLUMEPROTECT_SOURCES}
)
 
target_include_directories(VOLUMEPROTECT_OBJ
    PRIVATE
    ${MODULE_ROOT_PATH}/platform/SecureCLib_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/include/json
    ${MODULE_ROOT_PATH}/third_open_src/jsoncpp_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/openssl_rel/include
    ${MODULE_ROOT_PATH}/third_open_src/libuuid_rel/include
    ${MODULE_OPEN_SRC_PATH}/boost_rel/include
    ${MODULE_ROOT_PATH}/src/common
    ${MODULE_ROOT_PATH}/src/log
    ${MODULE_ROOT_PATH}/src
)

target_link_directories(VOLUMEPROTECT_OBJ
    PRIVATE
    ${MODULE_ROOT_PATH}/platform/SecureCLib_rel/lib
    ${MODULE_ROOT_PATH}/platform/SecureCLib_rel
    ${MODULE_ROOT_PATH}/SecureCLib_rel/lib
)

target_link_libraries(VOLUMEPROTECT_OBJ
    PRIVATE
    securec
)