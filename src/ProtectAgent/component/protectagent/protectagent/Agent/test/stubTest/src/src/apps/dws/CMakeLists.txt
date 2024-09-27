add_library(testDwsobj
    OBJECT
    BSAClient2DMETest.cpp
    BSAClientHandlerTest.cpp
    BSAClientTest.cpp
    BSAServiceHandlerTest.cpp
    DWSPluginTest.cpp
    ThriftServerTest.cpp
)

target_link_libraries(testDwsobj
    PRIVATE
    safe_cmplib
    agent_platform
)

add_subdirectory(XBSAServer)