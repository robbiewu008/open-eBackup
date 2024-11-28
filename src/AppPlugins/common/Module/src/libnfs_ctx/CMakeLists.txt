file(GLOB_RECURSE libnfs_ctx_srcs *.cpp)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)

SET(PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/../../platform)
set(OPENSRC_INC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../third_open_src")
set(BOOST_PATH "${OPENSRC_INC_PATH}/boost_rel")
set(NFS_PATH "${OPENSRC_INC_PATH}/lnfs_rel")
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(nfs_ctx
    SHARED
    ${libnfs_ctx_srcs}
)

target_include_directories(nfs_ctx
    PUBLIC 
    ${PLATFORM}/SecureCLib_rel/include
    ${MODULE_PATH}
    ${NFS_PATH}/include
    ${BOOST_PATH}/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../log
    ${CMAKE_CURRENT_SOURCE_DIR}/../threadpool
)

target_link_directories(nfs_ctx
    PUBLIC
    ${NFS_PATH}/lib/
)

target_link_libraries(nfs_ctx
    PUBLIC
    safe_cmplib
    nfs
)
