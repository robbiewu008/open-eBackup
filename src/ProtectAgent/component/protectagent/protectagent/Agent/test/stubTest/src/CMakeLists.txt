add_library(testMainobj
    OBJECT
    AgentTestMain.cpp
)

target_link_libraries(testMainobj
    PRIVATE
    safe_cmplib
    agent_platform
)

add_subdirectory(src)
add_subdirectory(stub)

add_library(testBaseLib
    STATIC
)

target_link_libraries(testBaseLib
    PUBLIC
    testStubobj
    testMainobj

    # base object
    commonobj
    securecomobj
    arrayobj
    alarmobj
    restobj
    curlclientobj

    # LLT gtest
    ${SLIB_GTEST}
)