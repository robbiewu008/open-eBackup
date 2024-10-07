MESSAGE("Compiling certificate service, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../inc)
SET(DME_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../dme_src)

# 递归添加certificate service目录下所有的cpp文件
file(GLOB_RECURSE certificate_service_srcs *.cpp)
add_library(certificate_service_obj
    OBJECT
    ${certificate_service_srcs}
)

target_include_directories(certificate_service_obj
    PRIVATE
    ${INC_PATH}/rpc/certificateservice
    ${INC_PATH}/rpc/servicefactory
    ${INC_PATH}/common
    ${INC_PATH}/utils
    ${MODULE_PATH}
    ${AGENT_SDK_PATH}/include
    ${PLATFORM_PATH}/SecureCLib_rel/include
)