aux_source_directory(. SRC_LIST)
add_library(archivestreamobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(archivestreamobj
    PRIVATE
    safe_cmplib
    agent_platform
)

target_compile_definitions(archivestreamobj
    PRIVATE
    SUPPORT_SSL
)

if(PLUGIN_SDK AND NOT CMAKE_SYSTEM_NAME MATCHES "AIX" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()