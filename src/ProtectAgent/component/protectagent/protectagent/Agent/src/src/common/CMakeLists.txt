aux_source_directory(. SRC_LIST)
if(PLUGIN_SDK)
    list(REMOVE_ITEM SRC_LIST "./StackTracer.cpp")
endif()

add_library(commonobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(commonobj
    PRIVATE
    safe_cmplib
    agent_platform
)

if(PLUGIN_SDK AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()