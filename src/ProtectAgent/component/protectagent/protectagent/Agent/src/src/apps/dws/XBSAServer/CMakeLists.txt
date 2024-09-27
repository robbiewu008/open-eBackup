file(GLOB SRC_LIST "*.cpp")
add_library(xbsaserverobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(xbsaserverobj
    PRIVATE
    safe_cmplib
    agent_platform
)