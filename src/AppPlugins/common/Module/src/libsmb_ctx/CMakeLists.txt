file(GLOB_RECURSE libsmb_ctx_srcs *.cpp)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)

SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(PLATFORM ${MODULE_PATH}/../platform)
set(OPENSRC_INC_PATH "${MODULE_PATH}/../third_open_src")
set(BOOST_PATH "${OPENSRC_INC_PATH}/boost_rel")
set(SMB_PATH "${OPENSRC_INC_PATH}/lsmb2_rel")


add_library(smb_ctx
    SHARED
    ${libsmb_ctx_srcs}
)

target_include_directories(smb_ctx
    PUBLIC 
    ${MODULE_PATH}
    ${SMB_PATH}/include
    ${BOOST_PATH}/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../log
    ${CMAKE_CURRENT_SOURCE_DIR}/../threadpool
)

target_link_directories(smb_ctx
    PUBLIC
    ${SMB_PATH}/lib/
    ${PLATFORM}/SecureCLib_rel/lib
)

target_link_libraries(smb_ctx
    PUBLIC
    safe_cmplib
    smb2
    securec
)
