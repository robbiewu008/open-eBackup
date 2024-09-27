MESSAGE("Compiling service factory, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../inc)

add_library(servicefactory_obj
    OBJECT
    IServiceCenter.cpp
    ServiceCenter.cpp
    ServiceFactory.cpp
)
MESSAGE("Compiling service factory, LIUWEITEST:INC_PATH is ${INC_PATH}")

target_include_directories(servicefactory_obj
    PRIVATE
    ${INC_PATH}/rpc/servicefactory
    ${INC_PATH}/rpc/certificateservice
    ${INC_PATH}/common
)

target_link_libraries(servicefactory_obj
    PRIVATE
    pthread
    thrift_interface
)
