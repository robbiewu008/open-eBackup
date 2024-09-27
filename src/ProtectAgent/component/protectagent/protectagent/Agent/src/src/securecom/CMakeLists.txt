aux_source_directory(. SRC_LIST)
add_library(securecomobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(securecomobj
    PRIVATE
    safe_cmplib
    agent_platform
)

if(PLUGIN_SDK AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()
