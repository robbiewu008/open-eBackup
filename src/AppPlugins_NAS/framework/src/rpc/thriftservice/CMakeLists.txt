MESSAGE("Compiling thrift service, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../inc)

# 递归添加certificate service目录下所有的cpp文件
file(GLOB_RECURSE thrift_service_srcs *.cpp)
add_library(thriftservice_obj
    OBJECT
    ${thrift_service_srcs}
)

target_include_directories(thriftservice_obj
    PRIVATE
    ${INC_PATH}/rpc/thriftservice
    ${INC_PATH}/rpc/servicefactory
    ${INC_PATH}/rpc/certificateservice
    ${INC_PATH}/rpc
    ${MODULE_PATH}
    ${OPEN_SRC_PATH}/openssl_rel/include
)

target_link_libraries(thriftservice_obj
    PRIVATE
    servicefactory_obj
    certificate_service_obj
    thrift_interface
)