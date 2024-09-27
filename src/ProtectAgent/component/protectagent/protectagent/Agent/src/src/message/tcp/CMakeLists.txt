add_library(tcpobj
    OBJECT
    BusinessClient.cpp
    CConnection.cpp
    CDppMessage.cpp
    CSocket.cpp
    DppSocket.cpp
    MessageHandler.cpp
    TCPClientHandler.cpp
)

target_link_libraries(tcpobj
    PRIVATE
    safe_cmplib
    agent_platform
)

add_library(tcpsslobj
    OBJECT
    BusinessClient.cpp
    CConnection.cpp
    CDppMessage.cpp
    CSocket.cpp
    DppSocket.cpp
    MessageHandler.cpp
    TCPClientHandler.cpp
)

target_link_libraries(tcpsslobj
    PRIVATE
    safe_cmplib
    agent_platform
)

target_compile_definitions(tcpsslobj
    PRIVATE
    SUPPORT_SSL
)

if(PLUGIN_SDK AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()
