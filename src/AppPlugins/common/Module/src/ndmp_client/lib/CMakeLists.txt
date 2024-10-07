INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../../build/cmake/global.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../../build/cmake/path.cmake)

SET(MODULE_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../)
SET(PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/../../../platform)
SET(OPENSRC_INC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_open_src")

set(CMAKE_C_COMPILER g++)
set(CMAKE_CXX_COMPILER g++)

MESSAGE("Start generate ndmp rpc code")
execute_process(COMMAND sh ${MODULE_ROOT_PATH}/build/ndmp-gencode.sh)
add_compile_options(-fPIC)

add_library(ndmp
    STATIC
    comm.c
    ndmp_xdr.c
)

target_include_directories(ndmp
    PRIVATE
    ${OPENSRC_INC_PATH}/tirpc_rel/include/tirpc
    ${PLATFORM}/SecureCLib_rel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
)

target_link_directories(ndmp
    PRIVATE
    ${OPENSRC_INC_PATH}/tirpc_rel/libs
)

target_link_libraries(ndmp
    PRIVATE    
    tirpc
)